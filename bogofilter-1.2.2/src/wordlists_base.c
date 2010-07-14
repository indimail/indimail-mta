/* $Id: wordlists_base.c 5873 2005-05-06 01:34:01Z relson $ */

#include "common.h"

#include "find_home.h"
#include "mxcat.h"
#include "paths.h"
#include "wordlists.h"
#include "wordlists_base.h"
#include "xmalloc.h"
#include "xstrdup.h"

/** priority queue of wordlists, ordered by their "override" parameter */
/*@null@*/ wordlist_t* word_lists=NULL;

/** Check if wordlist nodes \a a and \a b have the same type, override,
 * listname and filepath, \returns true if these parameters match in
 * either list node. */
static bool is_dup_wordlist(wordlist_t *a, wordlist_t *b)
{
    if (a->type != b->type)
	return false;

    if (a->override!= b->override)
	return false;

    if (strcmp(a->listname, b->listname) != 0)
	return false;

    if (a->bfp->filepath != NULL &&
	b->bfp->filepath != NULL &&
	strcmp(a->bfp->filepath, b->bfp->filepath) != 0)
	return false;

    return true;
}

/** Free a wordlist node and return the pointer to the next node */
static wordlist_t *free_wordlistnode(wordlist_t *node)
{
    wordlist_t *next = node->next;

    xfree(node->listname);
    bfpath_free(node->bfp);
    xfree(node);

    return next;
}

void wordlists_set_bogohome(void)
{
    wordlist_t *list;

    /* set up default wordlist, if not yet done */
    if (word_lists == NULL)
	init_wordlist("word", WORDLIST, 0, WL_REGULAR);

    for (list = word_lists; list != NULL; list = list->next)
	bfpath_set_bogohome(list->bfp);
}

void init_wordlist(const char* name,
		   const char *path,
		   int override, WL_TYPE type)
{
    wordlist_t *n = (wordlist_t *)xcalloc(1, sizeof(*n));
    wordlist_t *list_ptr;

    /* initialize list node */
    n->listname=xstrdup(name);
    n->bfp     =bfpath_create(path);
    n->type    =type;
    n->override=override;

    /* now enqueue according to "override" (priority) */
    list_ptr=word_lists;

    if (list_ptr == NULL ||
	list_ptr->override > override) {
	/* prepend to list */
	n->next=word_lists;
	word_lists=n;
	return;
    }

    for ( ; ; list_ptr=list_ptr->next) {
	/* drop duplicates */
	if (is_dup_wordlist(n, list_ptr)) {
	    free_wordlistnode(n);
	    return;
	}

	/* append to list */
	if (list_ptr->next == NULL ||
	    list_ptr->next->override > override) {
	    n->next=list_ptr->next;
	    list_ptr->next=n;
	    return;
	}
    }
}

wordlist_t *get_default_wordlist(wordlist_t *list)
{
    for (; list != NULL ; list = list->next)
    {
	if (list->type != WL_IGNORE)
	    return list;
    }

    /* not found -> abort */
    fprintf(stderr, "Can't find default wordlist.\n");
    exit(EX_ERROR);
}

/* setup_wordlists()
   returns: -1 for error, 0 for success
   **
   ** precedence: (high to low)
   **
   **	command line
   **	$BOGOFILTER_DIR
   **	user config file
   **	site config file
   **	$HOME
   */

void free_wordlists()
{
    wordlist_t *list = word_lists;

    while (list)
    {
	list = free_wordlistnode(list);
    }

    bogohome_cleanup();
}

void display_wordlists(wordlist_t *list, const char *fmt)
{
    for (; list; list = list->next)
    {
	printf(fmt, "wordlist");
	printf("%s,%s,%s,%d\n",
		(list->type == WL_REGULAR) ? "r" : "i",
		list->listname,
		list->bfp->filepath,
		list->override);
    }
}
