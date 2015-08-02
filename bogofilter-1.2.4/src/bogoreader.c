/* $Id: bogoreader.c 6924 2010-07-06 21:44:36Z m-a $ */

/*****************************************************************************

NAME:
   bogoreader.c -- process input files

AUTHORS: (C) Copyright 2003-2005 by
   David Relson <relson@osagesoftware.com>
   Matthias Andree <matthias.andree@gmx.de>

******************************************************************************/

/*
** Formats supported:
**
**	mbox
**	Maildir
**	MH folder
**	rmail
**	ANT		RISC-OS only
**
**	msg-count	special for bogofilter
*/

#include "common.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "bogoreader.h"
#include "error.h"
#include "fgetsl.h"
#include "lexer.h"
#include "paths.h"
#include "token.h"
#include "xmalloc.h"

static void (*fini)(void);
static int  argc;
static const char * const *argv;
static const char *filename;
static char namebuff[PATH_LEN+1];
static char dir_name[PATH_LEN+1];

static FILE *yy_file;

typedef enum ms_e {MS_FILE, MS_MAILDIR, MS_MH } ms_t;

static ms_t mailstore_type;
static bool mail_first = true;		/* for the _next_mail functions */
static bool mailstore_first = true;	/* for the _next_mailstore functions */
static bool firstline = true;		/* for mailbox /^From / match */

static bool    have_message = false;

/* Lexer-Reader Interface */

reader_more_t *reader_more;
reader_line_t *reader_getline;
reader_file_t *reader_filename;

/* Function Prototypes */

/* these functions check if there are more file names in bulk modes,
 * read-mail/mbox-from-stdin for uniformity */
static reader_more_t stdin_next_mailstore;
static reader_more_t b_stdin_next_mailstore;
static reader_more_t b_args_next_mailstore;

/* these functions check if there is more mail in a mailbox/maildir/...
 * to process, trivial mail_next_mail for uniformity */
static reader_more_t dir_next_mail;
static reader_more_t mail_next_mail;
static reader_more_t mailbox_next_mail;

/* maildir is the mailbox format specified in
 * http://cr.yp.to/proto/maildir.html */

static reader_line_t simple_getline;	/* ignores /^From / */
static reader_line_t mailbox_getline;	/* minds   /^From / */
static reader_line_t rmail_getline;	/* minds   /^#! rmail/ */
static reader_line_t ant_getline;	/* minds   /^MAIL TO:/ */

static reader_file_t get_filename;

static void bogoreader_close(void);

typedef enum { MBOX, MC, RMAIL, ANT } mbox_t;

typedef struct {
    const char	*sep;
    uint	len;
    mbox_t	type;
    reader_line_t *fcn;
} sep_2_box_t;

static sep_2_box_t sep_2_box[] = {
    { "From ",      	 5, MBOX,  mailbox_getline },
    { "\".MSG_COUNT\"", 12, MC,    mailbox_getline },	/* msg-count */
    { "#! rmail",   	 8, RMAIL, rmail_getline   },
    { "MAIL FROM:", 	10, ANT,   ant_getline     }	/* RISC-OS only */
};

static uint        seplen = 0;
static const char *separator = NULL;

static void dir_init(const char *name);
static void dir_fini(void);

typedef enum st_e { IS_DIR, IS_FILE, IS_ERR } st_t;

/* Function Definitions */

bool is_eol(const char *buf, size_t len)
{
    bool ans = ((len == 1 && memcmp(buf, NL, 1) == 0) ||
		(len == 2 && memcmp(buf, CRLF, 2) == 0));
    return ans;
}

static reader_line_t *get_reader_line(FILE *fp) {
    uint i;
    int c;
    reader_line_t *fcn = mailbox_getline;

    if (fp == NULL)
	return NULL;

    c = fgetc(fp);
    ungetc(c, fp);

    for (i = 0; i < COUNTOF(sep_2_box); i += 1) {
	sep_2_box_t *s = sep_2_box + i;
        if (s->sep[0] == c) {
            fcn = s->fcn;
	    seplen = s->len;
	    separator = s->sep;
	    break;
	}
    }
    
    if (fcn == mailbox_getline && !mbox_mode)
        fcn = simple_getline;
    
    return fcn;
}

/* Checks if name is a directory.
 * Returns IS_DIR for directory, IS_FILE for other type, IS_ERR for error
 */
static st_t isdir(const char *name)
{
    struct stat stat_buf;
    if (stat(name, &stat_buf)) return IS_ERR;
    return (S_ISDIR(stat_buf.st_mode) != 0) ? IS_DIR : IS_FILE;
}

static void save_dirname(const char *name)
{
    size_t l = strlen(name);
    l = min(l, sizeof(dir_name)-2);
    memcpy(dir_name, name, l);
    if (dir_name[l-1] == DIRSEP_C)
	l -= 1;
    dir_name[l] = '\0';
}

static const char* const maildir_subs[]={ DIRSEP_S "new", DIRSEP_S "cur", NULL };
static const char *const *maildir_sub;
static DIR *reader_dir;

/* MA: Check if the given name points to a Maildir. We don't require the
 * /tmp directory for simplicity.
 * This function checks if dir, dir/new and dir/cur are all directories.
 * Returns IS_DIR for directory, IS_FILE for other type, IS_ERR for error
 */
static st_t ismaildir(const char *dir) {
    st_t r;
    size_t l;
    char *x;
    const char *const *y;
    const size_t maxlen = 4;

    r = isdir(dir);
    if (r != IS_DIR) return r;
    x = xmalloc((l = strlen(dir)) + maxlen /* append */ + 1 /* NUL */);
    memcpy(x, dir, l);
    for (y = maildir_subs; *y; y++) {
	strlcpy(x + l, *y, maxlen + 1);
	r = isdir(x);
	if (r != IS_DIR) {
	    xfree(x);
	    return r;
	}
    }
    xfree(x);
    return IS_DIR;
}

static void dummy_fini(void) { }

static reader_more_t *mailstore_next_store;
static reader_more_t *mailstore_next_mail = NULL;

/* this is the 'nesting driver' for our input.
 * mailstore := one of { mail, mbox, maildir }
 * if we have a current mailstore-specific handle, check that if we have
 * further input in the mailstore first. if we don't, see if we have
 * further mailstores to process
 */
static bool reader__next_mail(void)
{
    for (;;) {
	/* check mailstore-specific method */
	if (mailstore_next_mail) {
	    if ((*mailstore_next_mail)()) /* more mails in the mailstore */
		return true;
	    mailstore_next_mail = NULL;
	}

	/* ok, that one has been exhausted, try the next mailstore */

	/* mailstore_next_store opens the mailstore */
	if (!(*mailstore_next_store)())
	    return false;

	/* ok, we have more mailstores, so check if the current mailstore has
	 * input - loop.
	 */
    }
}

/* open mailstore (Maildir, mbox file or file with a single mail) and set
 * _getline and _next_mail pointers dependent on the mailstore's type.
 *
 * - automatically detects maildir
 * - does not automatically distinguish between mbox and mail
 *   and takes mbox_mode instead
 */
static bool open_mailstore(const char *name)
{
    filename = name;
    bogoreader_close();
    firstline = true;
    switch (isdir(filename)) {
    case IS_FILE:
	if (DEBUG_READER(0))
	    fprintf(dbgout, "%s:%d - assuming %s is a %s\n", __FILE__, __LINE__, filename, mbox_mode ? "mbox" : "message");
	fpin = fopen( filename, "r" );
	if (fpin == NULL) {
	    fprintf(stderr, "Can't open file '%s': %s\n", filename,
		    strerror(errno));
	    return false;
	} else {
	    mail_first = true;
	    msg_count_file = false;
	    reader_getline = get_reader_line(fpin);
	    mailstore_next_mail = mbox_mode ? mailbox_next_mail : mail_next_mail;
	    return true;
	}
    case IS_DIR:
	if (ismaildir(filename) == IS_DIR) {
	    /* MAILDIR */
	    mailstore_type = MS_MAILDIR;
	    dir_init(filename);
	    reader_getline      = simple_getline;
	    mailstore_next_mail = dir_next_mail;
	    return true;
	} else {
	    /* MH */
	    mailstore_type = MS_MH;
	    dir_init(filename);
	    reader_getline      = simple_getline;
	    mailstore_next_mail = dir_next_mail;
	    return true;
	}
    case IS_ERR:
	fprintf(stderr, "Can't stat mailstore '%s': %s\n",
		filename, strerror(errno));
	break;
    default:
	fprintf(stderr, "Can't identify type of mailstore '%s'\n", filename);
	break;
    }
    return false;
}

/*** _next_mailstore functions ***********************************************/

/* this initializes for reading a single mail or a mbox from stdin */
static bool stdin_next_mailstore(void)
{
    bool val = mailstore_first;

    reader_getline = get_reader_line(fpin);

    if (reader_getline == NULL)
	return false;

    mailstore_next_mail = mbox_mode ? mailbox_next_mail : mail_next_mail;
    mailstore_first = false;
    return val;
}

/* this reads file names from stdin and processes them according to
 * their type */
static bool b_stdin_next_mailstore(void)
{
    int len;
    filename = namebuff;

    if ((len = fgetsl(namebuff, sizeof(namebuff), stdin)) <= 0)
	return false;

    if (len > 0 && namebuff[len-1] == '\n')
	namebuff[len-1] = '\0';

    return open_mailstore(filename);
}

/* this reads file names from the command line and processes them
 * according to their type */
static bool b_args_next_mailstore(void)
{
    if (argc <= 0)
	return false;
    filename = *argv;
    argc -= 1;
    argv += 1;
    return open_mailstore(filename);
}

/*** _next_mail functions ***********************************************/

/* trivial function, returns true on first run,
 * returns false on all subsequent runs */
static bool mail_next_mail(void)
{
    bool val = mail_first;
    mail_first = false;
    return val;
}

/* always returns true on the first run
 * subsequent runs return true when a From line was encountered */
static bool mailbox_next_mail(void)
{
    bool val = mail_first || have_message;
    mail_first = false;
    return val;
}

/* iterates over files in a directory */
static bool dir_next_mail(void)
{
    struct dirent *dirent;
    struct stat st;

    for (;;) {
	if (reader_dir == NULL) {
	    char *x = dir_name;
	    /* open next directory */
	    if (mailstore_type == MS_MAILDIR) {
		size_t siz;

		if (*maildir_sub == NULL)
		    return false; /* IMPORTANT for termination */
		siz = strlen(dir_name) + 4 + 1;
		x = xmalloc(siz);
		strlcpy(x, dir_name, siz);
		strlcat(x, *(maildir_sub++), siz);
	    }
	    reader_dir = opendir(x);
	    if (!reader_dir) {
		fprintf(stderr, "cannot open directory '%s': %s", x,
			strerror(errno));
	    }
	    if (x != dir_name)
		xfree(x);
	}

	while ((errno = 0, dirent = readdir(reader_dir)) != NULL) {
	    /* skip private files */
	    if ((mailstore_type == MS_MAILDIR && dirent->d_name[0] != '.') ||
		(mailstore_type == MS_MH && isdigit((unsigned char)dirent->d_name[0])))
		break;
	}

	if (errno) {
	    fprintf(stderr, "Cannot read directory %s: %s",
		    dir_name, strerror(errno));
	    exit(EX_ERROR);
	}

	if (dirent == NULL) {
	    if (reader_dir)
		closedir(reader_dir);
	    reader_dir = NULL;
	    if (mailstore_type == MS_MAILDIR)
		continue;
	    if (mailstore_type == MS_MH)
		return false;
	}

	filename = namebuff;
	snprintf(namebuff, sizeof(namebuff), "%s%s%c%s", dir_name, 
		 (mailstore_type == MS_MH) ? "" : *(maildir_sub-1),
		 DIRSEP_C, dirent->d_name);

	bogoreader_close();
	fpin = fopen( filename, "r" );
	if (fpin == NULL) {
	    fprintf(stderr, "Warning: can't open file '%s': %s\n", filename,
		    strerror(errno));
	    /* don't barf, the file may have been changed by another MUA,
	     * or a directory that just doesn't belong there, just skip it */
	    continue;
	}

	/* skip non-regular files */
	if (0 == fstat(fileno(fpin), &st) && !S_ISREG(st.st_mode))
	    continue;

	if (DEBUG_READER(0))
	    fprintf(dbgout, "%s:%d - reading %s (%p)\n", __FILE__, __LINE__,
		    filename, (void *)fpin);

	return true;
    }
}

/*** _getline functions ***********************************************/

/* reads from a mailbox, paying attention to ^From lines */
static int mailbox_getline(buff_t *buff)
{
    uint used = buff->t.leng;
    byte *buf = buff->t.u.text + used;
    int count;
    static word_t *saved = NULL;
    static bool emptyline = false;		/* for mailbox /^From / match */

    if (saved != NULL) {
	count = saved->leng;
	buff_add(buff, saved);
	word_free(saved);
	saved = NULL;
	return count;
    }

    count = buff_fgetsl(buff, fpin);
    have_message = false;

    /* XXX FIXME: do we need to unescape the >From, >>From, >>>From, ... lines
     * by discarding the first ">"? */

    /* DR 08/25/03 - NO!!! */

    if ((firstline || emptyline) &&
	seplen != 0 && count >= (int) seplen && memcmp(separator, buf, seplen) == 0)
    {
	if (firstline) {
	    firstline = false;
	}
	else {
	    have_message = true;
	    saved = word_new(buf, count);
	    count = EOF;
	}
    }
    else {
	if (buff->t.leng < buff->size)		/* for easier debugging - removable */
	    Z(buff->t.u.text[buff->t.leng]);	/* for easier debugging - removable */
    }

    emptyline = is_eol((char *)buf, count);

    return count;
}

/* reads from an rmail batch, paying attention to ^#! rmail lines */
static int rmail_getline(buff_t *buff)
{
    int count;
    uint used = buff->t.leng;
    byte *buf = buff->t.u.text + used;
    static word_t *saved = NULL;
    static uint bytesleft = 0;

    if (saved != NULL) {
	count = saved->leng;
	buff_add(buff, saved);
	word_free(saved);
	saved = NULL;
	return count;
    }

    if (bytesleft) {
	count = buff_fgetsln(buff, fpin, bytesleft);
	if (count > 0)
	    bytesleft -= count;
	return count;
    }

    count = buff_fgetsl(buff, fpin);
    have_message = false;

    if (count >= (int) seplen && memcmp(separator, buf, seplen) == 0)
    {
	uint i;
	bytesleft = 0;
	for (i = seplen; i < (uint) count; i++) {
	    if (isspace(buf[i])) continue;
	    if (!isdigit(buf[i])) break;
	    bytesleft = bytesleft * 10 + (buf[i] - '0');
	}
	if (firstline) {
	    firstline = false;
	}
	else {
	    have_message = true;
	    saved = word_new(buf, count);
	    count = EOF;
	}
    } else {
	if (buff->t.leng < buff->size)		/* for easier debugging - removable */
	    Z(buff->t.u.text[buff->t.leng]);	/* for easier debugging - removable */
    }

    return count;
}

/* reads from an ANT batch, paying attention to ^#! rmail lines */
static int ant_getline(buff_t *buff)
{
    int count;
    uint used = buff->t.leng;
    byte *buf = buff->t.u.text + used;
    static word_t *saved = NULL;
    static bool dot_found = true;

    if (saved != NULL) {
	count = saved->leng;
	buff_add(buff, saved);
	word_free(saved);
	saved = NULL;
	return count;
    }

    count = buff_fgetsl(buff, fpin);
    have_message = false;

    if (dot_found && count >= (int) seplen && memcmp(separator, buf, seplen) == 0)
    {
	dot_found = false;		/* ignore until dot */
	if (firstline) {
	    firstline = false;
	}
	else {
	    have_message = true;
	    saved = word_new(buf, count);
	    count = EOF;
	}
    } else {
        if ((count == 2 || count == 3) && 
	    (buf[0] == '.') && 
	    (buf[1] == '\r' || buf[1] == '\n'))
            dot_found = true;			/* dot found.  look for separator */
	if (buff->t.leng < buff->size)		/* for easier debugging - removable */
	    Z(buff->t.u.text[buff->t.leng]);	/* for easier debugging - removable */
    }

    return count;
}

/* reads a file as a single mail ( no ^From detection ). */
static int simple_getline(buff_t *buff)
{
    int count = buff_fgetsl(buff, fpin);

    if (buff->t.leng < buff->size)	/* for easier debugging - removable */
	Z(buff->t.u.text[buff->t.leng]);/* for easier debugging - removable */

    return count;
}

/* initialize for MH directory and
 * Maildir subdirectories (cur and new). */
static void dir_init(const char *name)
{
    reader_dir = NULL;
    fini = dir_fini;

    if (mailstore_type == MS_MAILDIR)
	maildir_sub = maildir_subs;

    save_dirname(name);

    return;
}

/* finish up MH/Maildir store */

static void dir_fini(void)
{
    if (reader_dir)
	closedir(reader_dir);
    reader_dir = NULL;
    return;
}

/* returns current file name */
static const char *get_filename(void)
{
    return filename;
}

/* global reader initialization, exported */
void bogoreader_init(int _argc, const char * const *_argv)
{
    mailstore_first = mail_first = true;
    reader_more = reader__next_mail;
    fini = dummy_fini;
    switch (bulk_mode) {
    case B_NORMAL:		/* read mail (mbox) from stdin */
	yy_file = fpin;
	mailstore_next_store = stdin_next_mailstore;
	if (run_type & (REG_SPAM|REG_GOOD|UNREG_SPAM|UNREG_GOOD))
	    mbox_mode = true;
	break;
    case B_STDIN:		/* '-b' - streaming (stdin) mode */
	mailstore_next_store = b_stdin_next_mailstore;
	break;
    case B_CMDLINE:		 /* '-B' - command line mode */
	argc = _argc;
	argv = (const char * const *) _argv;
	mailstore_next_store = b_args_next_mailstore;
	mailstore_next_mail  = NULL;
	break;
    default:
	fprintf(stderr, "Unknown bulk_mode = %d\n", (int) bulk_mode);
	abort();
	break;
    }
    reader_filename = get_filename;
}

/* For bogoconfig to distinguish '-I file' from '-I dir' */
/* global reader initialization, exported */
void bogoreader_name(const char *name)
{
    bool ok;
    switch (isdir(name)) {
    case IS_FILE:
	fpin = fopen( name, "r" );
	ok = fpin != NULL;
	break;
    case IS_DIR:
	ok = open_mailstore(name);
	break;
    default:
	ok = false;
	break;
    }
    if (!ok) {
	fprintf(stderr, "Can't read '%s'\n", name);
	exit(EX_ERROR);
    }
}

/* cleanup after reading a message, exported. */
/* Only called if the passthrough code says it is ok to close the file. */

void bogoreader_close_ifeof(void)
{
    if (fpin && feof(fpin))
       bogoreader_close();
}

/* global cleanup, exported */
void bogoreader_fini(void)
{
    bogoreader_close();
    fini();
}

static void bogoreader_close(void)
{
    if (fpin && fpin != stdin)
	fclose(fpin);
    fpin = NULL;
}
