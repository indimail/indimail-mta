/* $Id: wordlists.c 6917 2010-07-05 15:59:15Z m-a $ */

#include "common.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "bogofilter.h"
#include "datastore.h"
#include "msgcounts.h"
#include "mxcat.h"
#include "paths.h"
#include "rand_sleep.h"
#include "wordlists.h"
#include "xmalloc.h"
#include "xstrdup.h"

#include "bsdqueue.h"

#define	MIN_SLEEP	0.5e+3		/* 0.5 milliseconds */
#define	MAX_SLEEP	2.0e+6		/* 2.0 seconds */

/* Function Definitions */

#define MAX_ENVS    10

/* Idea: - build dirname
 *	 - search list of known environments if we have that dirname,
 *	   if so, return it
 *	 - create and insert new environment
 *
 * for destruction: - when closed an environment, NULL it in word_lists
 * past the current one that match our address
 */

static LIST_HEAD(envlist, envnode) envs;
struct envlist envlisthead;
struct envnode {
    LIST_ENTRY(envnode) entries;
    void *dbe;
    char directory[1];
};

static void *list_searchinsert(bfpath *bfp)
{
    uint l;
    struct envnode *i, *n;

    for (i = envlisthead.lh_first ; i ; i = i->entries.le_next) {
	if (strcmp(bfp->dirname, &i->directory[0]) == 0)
	    return i->dbe;
    }

    l = strlen(bfp->dirname) + 1;
    n = xmalloc(sizeof(struct envnode) + l);

    n->dbe = ds_init(bfp);

    memcpy(&n->directory[0], bfp->dirname, l);
    LIST_INSERT_HEAD(&envlisthead, n, entries);
    return n->dbe;
}

void begin_wordlist(wordlist_t *list)
{
    dsv_t val;

    while (1) {
	if (ds_txn_begin(list->dsh)) {
	    rand_sleep(1000,1000000);
	    continue;
	}
	switch (ds_get_msgcounts(list->dsh, &val)) {
	    case 0:
	    case 1:
		list->msgcount[IX_GOOD] = val.goodcount;
		list->msgcount[IX_SPAM] = val.spamcount;
		if (wordlist_version == 0 &&
			ds_get_wordlist_version(list->dsh, &val) == 0)
		    wordlist_version = val.count[0];
		break;
	    case DS_ABORT_RETRY:
		continue;
	}
	switch (ds_get_wordlist_encoding(list->dsh, &val)) {
	    case 0:		/* found */
		list->encoding = val.spamcount;
		break;
	    case 1:		/* not found */
		break;
	    case DS_ABORT_RETRY:
		continue;
	}
	break;
    }
}

static bool open_wordlist(wordlist_t *list, dbmode_t mode)
{
    bool retry = false;
    void *dbe;
    bfpath *bfp = list->bfp;

    if (list->dsh != NULL)		/* Only need to open the list once. */
	return retry;

    if (list->type == WL_IGNORE) 	/* open ignore list in read-only mode */
	mode = DS_READ;

    /* FIXME: create or reuse environment from filepath */

    dbe = list_searchinsert(bfp);

    if (dbe == NULL)
	exit(EX_ERROR);

    list->dsh = ds_open(dbe, bfp, mode); /* FIXME -- euh, what is here to fix? */

    if (list->dsh == NULL) {
	int err = errno;
	close_wordlists(false);	/* unlock and close */
	switch (err) {
	    /* F_SETLK can't obtain lock */
	case EAGAIN:
#ifdef __EMX__
	case EACCES:
#endif
	    rand_sleep(MIN_SLEEP, MAX_SLEEP);
	    retry = true;
	    break;
	default:
	    if (query)	/* If "-Q", failure is OK */
		return false;
	    fprintf(stderr,
		    "Can't open file '%s' in directory '%s'.\n",
		    bfp->filename, bfp->dirname);
	    if (err != 0)
		fprintf(stderr,
			"error #%d - %s.\n", err, strerror(err));

	    /* print error and exit */
	    wordlist_error(err);
	} /* switch */
    } else { /* ds_open */
	begin_wordlist(list);
    }

    if (list->encoding != E_UNKNOWN) {
	if (encoding == E_UNKNOWN)
	    encoding = list->encoding;
	if (encoding != list->encoding) {
	    fprintf(stderr, "Can't mix database encodings, i.e. utf-8 and any other.\n");
	    exit(EX_ERROR);
	}
    }

    return retry;
}

/* Note: seems like some cleanup around here should be possible....
** It seems wrong to call set_bogohome() from outside of paths.c.
** (DR 04/03/05)
*/
static void check_wordlist_path(wordlist_t *list)
{
    bfpath *bfp = list->bfp;
    bfpath_mode mode;

    if (((run_type & UNREG_SPAM) != 0) ||
	((run_type & UNREG_GOOD) != 0))
	mode = BFP_MUST_EXIST;
    else
	mode = BFP_MAY_CREATE;

    bfpath_check_mode(bfp, mode);
    bfpath_set_bogohome(bfp);
}

/* set bogohome using first wordlist's directory */
void set_wordlist_directory(void)
{
    wordlist_t *list;

    /* return if bogohome already set */
    if (bogohome != NULL)
	return;

    list = word_lists;		/* get first wordlist */

    if (list != NULL) {
	bfpath *bfp = list->bfp;
	char *dir = get_directory_from_path(bfp->filepath);
	if (dir != NULL) {
	    check_wordlist_path(list);
	    set_bogohome(dir);
	    free(dir);
	}
    }
}

void open_wordlists(dbmode_t mode)
{
    wordlist_t *list;
    bool retry = true;

    /* set default wordlist if none specified */
    if (word_lists == NULL)
	init_wordlist("word", WORDLIST, 0, WL_REGULAR);

    /* set bogohome using first wordlist's directory */
    set_wordlist_directory();

    chk_bogohome();

    /* add bogohome value to path structs */
    for (list = word_lists; list != NULL; list = list->next)
	bfpath_set_bogohome(list->bfp);

    LIST_INIT(&envs);

    while (retry) {
	retry = false;
	for (list = word_lists; list != NULL ; list = list->next) {
	    check_wordlist_path(list);
	    retry |= open_wordlist(list, list->type == WL_IGNORE ? DS_READ : mode);
	    if (list == word_lists && encoding == E_UNKNOWN)
		encoding = list->encoding;
	    if (list->encoding != E_UNKNOWN &&
		list->encoding != encoding) {
		    fprintf(stderr, "Can't mix database encodings, i.e. utf-8 and any other.\n");
		    exit(EX_ERROR);
	    }
	}
    }
}

/** close all open word lists */
bool close_wordlists(bool commit /** if unset, abort */)
    /* FIXME: we really need to look at the list's environments */
{
    bool err = false;
    wordlist_t *list;
    struct envnode *i;

    for (list = word_lists; list != NULL ; list = list->next) {
	void *vhandle = list->dsh;
	list->dsh = NULL;
	if (vhandle) {
	    if (commit) {
		if (ds_txn_commit(vhandle))
		    err = true;
	    } else {
		(void)ds_txn_abort(vhandle);
	    }
	    ds_close(vhandle);
	}
    }

    while ((i = envlisthead.lh_first)) {
	ds_cleanup(i->dbe);
	LIST_REMOVE(i, entries);
	xfree(i);
    }

    if (commit)
	word_lists = NULL;

    return err;
}

#ifdef COMPILE_DEAD_CODE
/* some sanity checking of lists is needed because we may
   allow users to specify lists eventually and there are
   opportunities to generate divide-by-zero exceptions or
   follow bogus pointers. */
static void sanitycheck_lists(void)
{
    wordlist_t* list=word_lists;
    int listcount=0;

    while (1) {
	if (list == NULL) break;
	if (list->name == NULL) {
	    fprintf(stderr, "A list has no name.\n");
	    exit(EX_ERROR);
	}
	if (list->msgcount==0) {
	    fprintf(stderr, "list %s has zero message count.\n", list->name);
	    exit(EX_ERROR);
	}
	listcount++;
	list=list->next;
    }
    if (0==listcount) {
	fprintf(stderr, "No wordlists available!\n");
	exit(EX_ERROR);
    }
    if (DEBUG_WORDLIST(1))
	fprintf(dbgout, "%d lists look OK.\n", listcount);
}
#endif

/** skips to the end of a work (comma, blank, tab separated), replaces
 * the delimiter by 0, skips over whitespace, and returns a pointer to
 * the next character. */
static char *spanword(char *p)
{
    const char *delim = ", \t";
    p += strcspn(p, delim);		/* skip to end of word */
    *p++ = '\0';
    p += strspn(p, " \t");		/* skip trailing whitespace */
    return p;
}

/* type - 'n', or 'i' (normal or ignore)
 * name - 'user', 'system', 'ignore', etc
 * path - 'wordlist.db', 'ignorelist.db', etc
 * override - 1,2,...
 */

/* returns true for success, false for error */
bool configure_wordlist(const char *val)
{
    char  ch;
    WL_TYPE type;
    char* listname;
    char* filename;
    int  precedence;

    char *tmp = xstrdup(val);

    ch= tmp[0];		/* save wordlist type (good/spam) */
    tmp = spanword(tmp);

    switch (toupper((unsigned char)ch))
    {
    case 'R':
	type = WL_REGULAR;
	break;
    case 'I':
	type = WL_IGNORE;
	break;
    default:
	fprintf( stderr, "Unknown wordlist type - '%c'\n", ch);
	return (false);
    }

    listname=tmp;		/* name of wordlist */
    tmp = spanword(tmp);

    filename=tmp;		/* path to wordlist */
    tmp = spanword(tmp);

    precedence=atoi(tmp);
    (void)spanword(tmp);

    init_wordlist(listname, filename, precedence, type);

    return true;
}

/* print error and exit */

void wordlist_error(int err)
{
    if (err == ENOENT)
	fprintf(stderr,
		"\n"
		"Remember to register some spam and ham messages before you\n"
		"use bogofilter to evaluate mail for its probable spam status!\n");

    if (err == EINVAL)
	fprintf(stderr,
		"\n"
		"Make sure that the database version this program is linked against\n"
		"can handle the format of the data base file (after updates in particular).\n");

    exit(EX_ERROR);
}
