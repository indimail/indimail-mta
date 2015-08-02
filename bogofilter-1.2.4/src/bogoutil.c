/* $Id: bogoutil.c 6993 2013-06-28 21:54:23Z m-a $ */

/*****************************************************************************

NAME:
  bogoutil.c -- dumps & loads bogofilter text files from/to Berkeley DB format.

AUTHORS:
  Gyepi Sam    <gyepi@praxis-sw.com>
  David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
 
#include "getopt.h"

#include "bogoconfig.h"
#include "bogofilter.h"
#include "bogohist.h"
#include "bool.h"
#include "buff.h"
#include "configfile.h"
#include "datastore.h"
#include "datastore_db.h"
#include "error.h"
#include "longoptions.h"
#include "maint.h"
#include "msgcounts.h"
#include "paths.h"
#include "prob.h"
#include "rand_sleep.h"
#include "robx.h"
#include "sighandler.h"
#include "swap.h"
#include "wordlists.h"
#include "xmalloc.h"
#include "xstrdup.h"

/* prototypes for dummies below: */
#include "score.h"

const char *progname = "bogoutil";

static int token_count = 0;

static bool maintain = false;
static bool onlyprint = false;

/* Function Definitions */

/* dummies to avoid score.o */
double msg_spamicity(void) { return .0; }
rc_t msg_status(void) { return RC_OK; }

static void ds_open_failure(bfpath *bfp, void *dbe)
{
    fprintf(stderr, "Error accessing file or directory '%s'.\n", bfp->filepath);
    if (errno != 0)
	fprintf(stderr, "error #%d - %s.\n", errno, strerror(errno));
    if (dbe != NULL)
	ds_cleanup(dbe);
    exit(EX_ERROR);
}

static int ds_dump_hook(word_t *key, dsv_t *data,
			/*@unused@*/ void *userdata)
/* returns 0 if ok, 1 if not ok */
{
    (void)userdata;

    if (fDie)
	exit(EX_ERROR);

    token_count += 1;

    if (maintain && discard_token(key, data))
	return 0;

    if (replace_nonascii_characters)
	do_replace_nonascii_characters(key->u.text, key->leng);

    fprintf(fpo, "%.*s %lu %lu",
	    CLAMP_INT_MAX(key->leng), key->u.text,
	    (unsigned long)data->spamcount,
	    (unsigned long)data->goodcount);
    if (data->date)
	fprintf(fpo, " %lu", (unsigned long)data->date);
    fprintf(fpo, "\n");

    fflush(stdout); /* solicit ferror flag if output is shorter than buffer */
    return ferror(stdout) ? 1 : 0;
}

static ex_t dump_wordlist(bfpath *bfp)
{
    ex_t rc;
    void *dbe;

    token_count = 0;

    dbe = ds_init(bfp);
    rc = ds_oper(dbe, bfp, DS_READ, ds_dump_hook, NULL);
    ds_cleanup(dbe);

    if (rc != EX_OK)
	fprintf(stderr, "error dumping tokens!\n");
    else
	if (verbose)
	    fprintf(dbgout, "%d tokens dumped\n", token_count);

    return rc;
}

#define BUFSIZE 512
const char POSIX_space[] = " \f\n\r\t\v";

static byte *spanword(byte *t)
{
    /* skip leading whitespace */
    t += strspn((const char *)t, POSIX_space);
    /* span current word */
    t += strcspn((const char *)t, POSIX_space);
    if (*t)
	*t++ = '\0';
    return t;
}

/** determines if the token is a regular token or a special non-count
 * token (.ROBX, .WORDLIST_VERSION), returns true if the token is a
 * count token */
static bool is_count(const char *in)
{
    static const char *const msgc = MSG_COUNT;
    static const char *const enco = WORDLIST_ENCODING;

    /* anything that doesn't start with a . is a count */
    if (in[0] != '.')
	return true;
    /* .MSG_COUNT is also a count */
    if (strcmp(in, msgc) == 0)
	return true;
    /* .ENCODING is also a count */
    if (strcmp(in, enco) == 0)
	return true;

    return false;
}

static ex_t load_wordlist(bfpath *bfp)
{
    void *dsh;
    byte buf[BUFSIZE];
    byte *p;
    int rv = 0;
    size_t len;
    int load_count = 0;
    unsigned long line = 0;
    unsigned long count[IX_SIZE], date;
    YYYYMMDD today_save = today;

    void *dbe = ds_init(bfp);

    dsh = ds_open(dbe, bfp, DS_WRITE | DS_LOAD);
    if (dsh == NULL)
	/* print error, cleanup, and exit */
	ds_open_failure(bfp, dbe);

    memset(buf, '\0', BUFSIZE);

    if (DST_OK != ds_txn_begin(dsh))
	exit(EX_ERROR);

    for (;;) {
	dsv_t data;
	word_t *token;
	if (fgets((char *)buf, BUFSIZE, fpin) == NULL) {
	    if (ferror(fpin)) {
		perror(progname);
		rv = 2;
	    }
	    break;
	}

	line++;

	len = strlen((char *)buf);

	/* too short. */
	if (len < 4)
	    continue;

	p = spanword(buf);
	len = strlen((const char *)buf);

	if (max_token_len != 0 &&
	    len > max_token_len)
	    continue;		/* too long - discard */

	spamcount = (uint) atoi((const char *)p);
	if ((int) spamcount < 0)
	    spamcount = 0;
	p = spanword(p);

	goodcount = (uint) atoi((const char *)p);
	if ((int) goodcount < 0)
	    goodcount = 0;
	p = spanword(p);

	date = (uint) atoi((const char *)p);
	p = spanword(p);

	if (*p != '\0') {
	    fprintf(stderr,
		    "%s: Unexpected input [%s] on line %lu. "
		    "Expecting whitespace before count.\n",
		    progname, buf, line);
	    rv = 1;
	    break;
	}

	if (date == 0)				/* date as YYYYMMDD */
	    date = today_save;

	if (replace_nonascii_characters)
	    do_replace_nonascii_characters(buf, len);
 
 	token = word_new(buf, len);
	data.goodcount = goodcount;
	data.spamcount = spamcount;
	data.date = date;

	if (is_count((const char *)buf)
		&& !(maintain && discard_token(token, &data))) {
	    load_count += 1;
	    /* Slower, but allows multiple lists to be concatenated */
	    set_date(date);
	    switch (ds_read(dsh, token, &data)) {
		case 0:
		case 1:
		    break;
		default:
		    rv = 1;
	    }
	    data.spamcount += spamcount;
	    data.goodcount += goodcount;
	    if (ds_write(dsh, token, &data)) rv = 1;
	}
	word_free(token);
    }

    if (rv) {
	fprintf(stderr, "read or write error, aborting.\n");
	ds_txn_abort(dsh);
    } else {
	switch (ds_txn_commit(dsh)) {
	    case DST_FAILURE:
	    case DST_TEMPFAIL:
		fprintf(stderr, "commit failed\n");
		exit(EX_ERROR);
	    case DST_OK:
		break;
	}
    }

    ds_close(dsh);

    ds_cleanup(dbe);

    if (verbose)
	fprintf(dbgout, "%d tokens loaded\n", load_count);

    return rv;
}

static int get_token(buff_t *buff, FILE *fp)
{
    int rv = 0;

    if (fgets((char *)buff->t.u.text, buff->size, fp) == NULL) {
	if (ferror(fp)) {
	    perror(progname);
	    rv = 2;
	} else {
	    rv = 1;
	}
    } else {
	buff->t.leng = (uint) strlen((const char *)buff->t.u.text);
	if (buff->t.u.text[buff->t.leng - 1] == '\n' ) {
	    buff->t.leng -= 1;
	    buff->t.u.text[buff->t.leng] = (byte) '\0';
	}
	else
	{
	    fprintf(stderr,
		    "%s: Unexpected input [%s]. Does not end with newline "
		    "or line too long.\n",
		    progname, buff->t.u.text);
	    rv = 1;
	}
    }
    return rv;
}

static ex_t display_words(bfpath *bfp, int argc, char **argv, bool show_probability)
{
    byte buf[BUFSIZE];
    buff_t *buff = buff_new(buf, 0, BUFSIZE);
    const byte *word;

    const char *path = bfp->filepath;

    const char *head_format = !show_probability ? "%-30s %6s %6s\n"   : "%-30s %6s  %6s  %6s\n";
    const char *data_format = !show_probability ? "%-30s %6lu %6lu\n" : "%-30s %6lu  %6lu  %f\n";

    void *dsh = NULL; /* initialize to silence bogus gcc warning */
    void *dbe;

    int rv = 0;

    dsv_t msgcnts;

    /* protect against broken stat(2) that succeeds for empty names */
    if (path == NULL || *path == '\0') {
        fprintf(stderr, "Expecting non-empty directory or file name.\n");
        return EX_ERROR;
    }

    dbe = ds_init(bfp);
    dsh = ds_open(dbe, bfp, DS_READ);;
    if (dsh == NULL)
	/* print error, cleanup, and exit */
	ds_open_failure(bfp, dbe);

    if (DST_OK != ds_txn_begin(dsh)) {
	ds_close(dsh);
	ds_cleanup(dbe);
	fprintf(stderr, "Cannot begin transaction.\n");
	return EX_ERROR;
    }

    if (show_probability)
    {
	ds_get_msgcounts(dsh, &msgcnts);
	robs = ROBS;
	robx = ROBX;
    }

    fprintf(fpo, head_format, "", "spam", "good", "  Fisher");
    while (argc >= 0)
    {
	dsv_t val;
	word_t *token;
	int rc;

	unsigned long spam_count;
	unsigned long good_count;
	double rob_prob = 0.0;
	
	if (argc == 0)
	{
	    if (get_token(buff, stdin) != 0)
		break;
	    token = &buff->t;
	} else {
	    word = (const byte *) *argv++;
	    if (--argc == 0)
		argc = -1;
	    token = word_news((const char *)word);
	}

	rc = ds_read(dsh, token, &val);
	switch (rc) {
	    case 0:
		spam_count = val.spamcount;
		good_count = val.goodcount;

		if (!show_probability)
		    fprintf(fpo, data_format, token->u.text, spam_count, good_count);
		else
		{
		    rob_prob = calc_prob(good_count, spam_count, msgcnts.goodcount, msgcnts.spamcount);
		    fprintf(fpo, data_format, token->u.text, spam_count, good_count, rob_prob);
		}
		break;
	    case 1:
		break;
	    default:
		fprintf(stderr, "Cannot read from database.\n");
		rv = EX_ERROR;
		goto finish;
	}

	if (token != &buff->t)
	    word_free(token);
    }

finish:
    if (DST_OK != rv ? ds_txn_abort(dsh) : ds_txn_commit(dsh)) {
	fprintf(stderr, "Cannot %s transaction.\n", rv ? "abort" : "commit");
	rv = EX_ERROR;
    }
    ds_close(dsh);
    ds_cleanup(dbe);

    buff_free(buff);

    return rv;
}

static ex_t get_robx(bfpath *bfp)
{
    double rx;
    int ret = 0;

    init_wordlist("word", bfp->filepath, 0, WL_REGULAR);
    rx = compute_robinson_x();
    if (rx < 0)
	return EX_ERROR;

    if (onlyprint)
	printf("%f\n", rx);
    else {
	dsv_t val;
	word_t *word_robx = word_news(ROBX_W);

	/* since compute_robinson_x() closes the wordlists, 
	   init_wordlist() must be called again */
	init_wordlist("word", bfp->filepath, 0, WL_REGULAR);

	open_wordlists(DS_WRITE);

	val.goodcount = 0;
	val.spamcount = (uint32_t) (rx * 1000000);
	do {
	    ret = ds_write(word_lists->dsh, word_robx, &val);
	    if (ret == DS_ABORT_RETRY) {
		rand_sleep(1000, 1000000);
		begin_wordlist(word_lists);
	    }
	} while (ret == DS_ABORT_RETRY);

	close_wordlists(true);
	free_wordlists();

	word_free(word_robx);
    }

    return ret ? EX_ERROR : EX_OK;
}

static void print_version(void)
{
    (void)fprintf(stdout,
		  "%s version %s\n"
		  "    Database: %s\n"
		  "Copyright (C) 2002-2010 David Relson, Matthias Andree\n"
		  "Copyright (C) 2002-2003 Gyepi Sam.\n\n"
		  "%s comes with ABSOLUTELY NO WARRANTY.  "
		  "This is free software, and\nyou are welcome to "
		  "redistribute it under the General Public License.  "
		  "See\nthe COPYING file with the source distribution for "
		  "details.\n"
		  "\n", 
		  progname, version, ds_version_str(), PACKAGE);
}

static void usage(FILE *fp)
{
    fprintf(fp, "Usage: %s {-h|-V}\n", progname);
    fprintf(fp, "   or: %s [OPTIONS] {-d|-l|-u|-m|-w|-p|--db-verify} file%s\n",
	    progname, DB_EXT);
    fprintf(fp, "   or: %s [OPTIONS] {-H|-r|-R} file\n", progname);
#if defined (ENABLE_DB_DATASTORE) || defined (ENABLE_SQLITE_DATASTORE)
    fprintf(fp, "   or: %s [OPTIONS] {--db-print-leafpage-count} file%s\n",
	    progname, DB_EXT);
    fprintf(fp, "   or: %s [OPTIONS] {--db-print-pagesize} file%s\n",
	    progname, DB_EXT);
#endif
#if	defined(ENABLE_DB_DATASTORE) && !defined(DISABLE_TRANSACTIONS)
    fprintf(fp, "   or: %s [OPTIONS] {--db-checkpoint} directory\n",
	    progname);
    fprintf(fp, "   or: %s [OPTIONS] {--db-list-logfiles} directory [list options]\n",
	    progname);
    fprintf(fp, "   or: %s [OPTIONS] {--db-prune|--db-remove-environment} directory\n",
	    progname);
    fprintf(fp, "   or: %s [OPTIONS] {--db-recover|--db-recover-harder} directory\n",
	    progname);
#endif
}

static const char *help_text[] = {
    "\n",
    "OPTIONS are:\n",
    "  -C, --no-config-file        - don't read standard config files.\n",
    "  -D, --debug-to-stdout       - direct debug output to stdout.\n",
#ifdef	ENABLE_DB_DATASTORE
    "  -k, --db-cachesize=size     - set Berkeley DB cache size (MB).\n",
#endif
    "  -v, --verbosity             - set debug verbosity level.\n",
    "  -x, --debug-flags=list      - set flags to display debug information.\n",
    "  -y, --timestamp-date=date   - set default date (format YYYYMMDD).\n",
    "\n",
    "Modes of operation are:\n",

    "  -h, --help                  - print this help message and exit.\n",
    "  -V, --version               - print version information and exit.\n",
    "\n",
    "  -d, --dump=file             - dump data from file to stdout.\n",
    "  -l, --load=file             - load data from stdin into file.\n",
    "  -u, --upgrade=file          - upgrade wordlist version.\n",
    "\n",

    "info options:\n",
    "  -w file                     - display counts for words from stdin.\n",
    "  -p file                     - display word counts and probabilities.\n",
    "  -I, --input-file=file       - read 'file' instead of standard input.\n",
    "  -H file                     - display histogram and statistics for wordlist.\n",
    "                                - use with -v  to exclude hapaxes.\n",
    "                                - use with -vv to exclude pure spam/ham.\n",
    "  -r file                     - compute Robinson's X for the specified file.\n",
    "  -R file                     - compute Robinson's X and save it in wordlist.\n",
    "\n",

    "database maintenance, the \"-m file\" option is required in this group:\n",
    "  -m file                     - enable maintenance works (expiring tokens).\n",
    "  -n                          - replace non-ascii characters with '?'.\n",
    "  -a age                      - exclude tokens with older ages.\n",
    "  -c cnt                      - exclude tokens with lower counts.\n",
    "  -s l,h                      - exclude tokens with lengths between 'l' and 'h'\n"
    "                                (low and high).\n",
#ifndef	DISABLE_UNICODE
    "  --unicode=yes/no            - convert wordlist to/from unicode\n",
#endif
    "\n",

    "token parsing options:\n",
    "  --min-token-len             - min len for single tokens\n",
    "  --max-token-len             - max len for single tokens\n",
    "  --max-multi-token-len       - max len for multi-word tokens\n",
    "  --multi-token-count         - number of tokens per multi-word token\n",
    "\n",

    NULL
    };

static void help(FILE *fp)
{
    uint i;
    const char **messages;
    usage(fp);
    messages = help_text;
    for (i=0; messages[i] != NULL; i++)
	(void)fprintf(fp, "%s", messages[i]);
    messages = dsm_help_bogoutil();
    for (i=0; messages[i] != NULL; i++)
	(void)fprintf(fp, "%s", messages[i]);
    (void)fprintf(fp,
		  "%s (version %s) is part of the bogofilter package.\n",
                  progname, version
	);
}

static const char *ds_file = NULL;
static bool  prob = false;

static cmd_t flag = M_NONE;

static struct option longopts_bogoutil[] = {
    /* longoptions.h - common options */
    LONGOPTIONS_COMMON
    /* longoptions.h - options for bogofilter and bogoutil */
    LONGOPTIONS_DB
    /* longoptions.h - options for bogolexer and bogoutil */
    LONGOPTIONS_LEX_UTIL

    /* bogoutil specific options */
    { "db-prune",                       R, 0, O_DB_PRUNE },
    { "db-checkpoint",                  R, 0, O_DB_CHECKPOINT },
    { "db-list-logfiles",               R, 0, O_DB_LIST_LOGFILES },
    { "db-print-leafpage-count",	R, 0, O_DB_PRINT_LEAFPAGE_COUNT },
    { "db-print-pagesize",		R, 0, O_DB_PRINT_PAGESIZE },
    { "db-recover",                     R, 0, O_DB_RECOVER },
    { "db-recover-harder",              R, 0, O_DB_RECOVER_HARDER },
    { "db-remove-environment",		R, 0, O_DB_REMOVE_ENVIRONMENT },
    { "db-verify",                      R, 0, O_DB_VERIFY },

    /* end of list */
    { NULL,				0, 0, 0 }
};

#define	OPTIONS	":a:c:Cd:DhH:I:k:l:m:nO:p:r:R:s:u:vVw:x:X:y:"

static int process_arglist(int argc, char **argv)
{
    int option;
    int count = 0;

    fpin = stdin;
    fpo  = stdout;
    dbgout = stderr;

#ifdef __EMX__
    _response (&argc, &argv);	/* expand response files (@filename) */
    _wildcard (&argc, &argv);	/* expand wildcards (*.*) */
#endif

    /* default: no token length checking */
    max_token_len = 0;

    while (1)
    {
	int option_index = 0;
	int this_option_optind = optind ? optind : 1;
	const char *name;

	option = getopt_long_chk(argc, argv, OPTIONS,
			     longopts_bogoutil, &option_index);

	if (option == -1)
 	    break;

	name = (option_index == 0) ? argv[this_option_optind] : longopts_bogoutil[option_index].name;
	count += process_arg(option, name, optarg, PR_NONE, PASS_1_CLI);
    }

    if (max_token_len != 0 && max_multi_token_len == 0) {
	/* token length checking ... */
	if (multi_token_count == 1)
	    max_multi_token_len = max_token_len + MAX_PREFIX_LEN;
	else
	    max_multi_token_len = max_token_len = (max_token_len+1) * multi_token_count + MAX_PREFIX_LEN;
    }

    if (count != 1)
    {
	usage(stderr);
	fprintf(stderr, "%s: Exactly one of the file or directory commands must be present.\n", progname);
	exit(EX_ERROR);
    }

    return count;
}

int process_arg(int option, const char *name, const char *val, priority_t precedence, arg_pass_t pass)
{
    int count = 0;

    (void) precedence;		/* suppress compiler warning */
    (void) pass;		/* suppress compiler warning */

    switch (option) {
    case '?':
	fprintf(stderr, "Unknown option '%s'.\n", name);
	break;

    case 'd':
	flag = M_DUMP;
	count += 1;
	ds_file = val;
	break;

    case O_CONFIG_FILE:
	read_config_file(val, false, false, PR_COMMAND, longopts_bogoutil);
	/*@fallthrough@*/
	/* fall through to suppress reading config files */

    case 'C':
	suppress_config_file = true;
	break;

    case 'k':
	db_cachesize=(uint) atoi(val);
	break;

    case 'l':
	flag = M_LOAD;
	count += 1;
	ds_file = val;
	break;

    case 'm':
	flag = M_MAINTAIN;
	count += 1;
	ds_file = val;
	break;

    case 'p':
	prob = true;
	/*@fallthrough@*/

    case 'w':
	flag = M_WORD;
	count += 1;
	ds_file = val;
	break;

    case O_DB_PRINT_LEAFPAGE_COUNT:
	flag = M_LEAFPAGES;
	count += 1;
	ds_file = val;
	break;

    case O_DB_PRINT_PAGESIZE:
	flag = M_PAGESIZE;
	count += 1;
	ds_file = val;
	break;

    case 'r':
	onlyprint = true;
    case 'R':
	flag = M_ROBX;
	count += 1;
	ds_file = val;
	break;

    case 'u':
	upgrade_wordlist_version = true;
	flag = M_MAINTAIN;
	count += 1;
	ds_file = val;
	break;

    case 'v':
	verbose++;
	break;

    case ':':
	fprintf(stderr, "Option %s requires an argument.\n", name);
	exit(EX_ERROR);

    case 'h':
	help(stdout);
	exit(EX_OK);

    case 'H':
	flag = M_HIST;
	count += 1;
	ds_file = val;
	break;

    case 'V':
	print_version();
	exit(EX_OK);

    case 'x':
	set_debug_mask(val);
	break;

    case 'X':
	set_bogotest(val);
	break;

    case 'a':
	maintain = true;
	thresh_date = string_to_date(val);
	break;

    case 'c':
	maintain = true;
	thresh_count = (uint) atoi(val);
	break;

    case 's':
    {
	unsigned long mi, ma;

	maintain = true;
	    
	if (2 == sscanf(val, "%lu,%lu", &mi, &ma)) {
	    size_min = mi;
	    size_max = ma;
	} else {
	    fprintf(stderr, "syntax error in argument \"%s\" of -s\n.",
		    val);
	    exit(EX_ERROR);
	}
    }
    break;

    case 'n':
	maintain = true;
	replace_nonascii_characters ^= true;
	break;

    case 'y':		/* date as YYYYMMDD */
    {
	YYYYMMDD date = string_to_date(val);
	maintain = true;
	if (date != 0 && date < 19990000) {
	    fprintf(stderr, "Date format for '-y' option is YYYYMMDD\n");
	    exit(EX_ERROR);
	}
	set_date( date );
	break;
    }

    case 'I':
	fpin = fopen(val, "r");
	if (fpin == NULL) {
	    fprintf(stderr, "Can't read file '%s'\n", val);
	    exit(EX_ERROR);
	}
	break;

    case 'O':
	fpo = fopen(val, "wt");
	if (fpo == NULL) {
	    fprintf(stderr, "Can't write file '%s'\n", val);
	    exit(EX_ERROR);
	}
	break;

    case 'D':
	dbgout = stdout;
	break;

    case O_DB_VERIFY:
	flag = M_VERIFY;
	count += 1;
	ds_file = val;
	break;

    case O_UNICODE:
	encoding = str_to_bool(val) ? E_UNICODE : E_RAW;
	break;

    case O_MAX_TOKEN_LEN:
	max_token_len = atoi(val);
	break;

    case O_MIN_TOKEN_LEN:
	min_token_len = atoi(val);
	break;

    case O_MAX_MULTI_TOKEN_LEN:
	max_multi_token_len=atoi(val);
	break;

    case O_MULTI_TOKEN_COUNT:
	multi_token_count=atoi(val);
	break;

    default:
	if (!dsm_options_bogoutil(option, &flag, &count, &ds_file, name, val)) {
	    fprintf(stderr, "Invalid option '%s'\n", name);
	    exit(EX_ERROR);
	}
    }

    return count;
}

static bfpath_mode get_mode(cmd_t cmd)
{
    bfpath_mode mode = BFP_ERROR;

    switch (cmd) {
    case M_LOAD:
	mode = BFP_MAY_CREATE;
	break;
    case M_DUMP:
    case M_HIST:
    case M_MAINTAIN:
    case M_ROBX:
    case M_VERIFY:
    case M_WORD:
    case M_CHECKPOINT:	/* database transaction/integrity operations */
    case M_CRECOVER:
    case M_LEAFPAGES:
    case M_PAGESIZE:
    case M_PURGELOGS:
    case M_RECOVER:
    case M_REMOVEENV:
    case M_LIST_LOGFILES:
	mode = BFP_MUST_EXIST;
	break;
    case M_NONE:
	usage(stderr);
	exit(EX_ERROR);
    }

    return mode;
}

int main(int argc, char *argv[])
{
    ex_t rc = EX_OK;

    bfpath *bfp;
    bfpath_mode mode;

    fBogoutil = true;

    signal_setup();			/* setup to catch signals */
    atexit(bf_exit);

    progtype = build_progtype(progname, DB_TYPE);

    set_today();			/* compute current date for token age */

    process_arglist(argc, argv);
    process_config_files(false, longopts_bogoutil);	/* need to read lock sizes */

    /* Extra or missing parameters */
    if (flag != M_WORD && flag != M_LIST_LOGFILES && argc != optind) {
	fprintf(stderr, "Missing or extraneous argument.\n");
	usage(stderr);
	exit(EX_ERROR);
    }

    bfp = bfpath_create(ds_file);
    if (bogohome == NULL)
	set_bogohome( "." );		/* set default */

    bfpath_set_bogohome(bfp);

    mode = get_mode(flag);
    if (bfpath_check_mode(bfp, mode)) {
	if (bfp->isdir)
	    bfpath_set_filename(bfp, WORDLIST);
    }

    if (!bfpath_check_mode(bfp, mode)) {
	fprintf(stderr, "Can't open wordlist '%s'\n", bfp->filepath);
	exit(EX_ERROR);
    }

    errno = 0;		/* clear error status */

    switch (flag) {
	case M_RECOVER:
	    ds_init(bfp);
	    rc = ds_recover(bfp, false);
	    break;
	case M_CRECOVER:
	    ds_init(bfp);
	    rc = ds_recover(bfp, true);
	    break;
	case M_CHECKPOINT:
	    ds_init(bfp);
	    rc = ds_checkpoint(bfp);
	    break;
	case M_LIST_LOGFILES:
	    dsm_init(bfp);
	    rc = ds_list_logfiles(bfp, argc - optind, argv + optind);
	    break;
	case M_PURGELOGS:
	    ds_init(bfp);
	    rc = ds_purgelogs(bfp);
	    break;
	case M_REMOVEENV:
	    dsm_init(bfp);
	    rc = ds_remove(bfp);
	    break;
	case M_VERIFY:
	    dsm_init(bfp);
	    rc = ds_verify(bfp);
	    break;
	case M_LEAFPAGES:
	    {
		u_int32_t c;

		dsm_init(bfp);
		c = ds_leafpages(bfp);
		if (c == 0xffffffff) {
		    fprintf(stderr, "%s: error getting leaf page count.\n", ds_file);
		    rc = EX_ERROR;
		} else if (c == 0) {
		    puts("UNKNOWN");
		} else {
		    printf("%lu\n", (unsigned long)c);
		}
	    }
	    break;
	case M_PAGESIZE:
	    {
		u_int32_t s;

		dsm_init(bfp);
		s = ds_pagesize(bfp);
		if (s == 0xffffffff) {
		    fprintf(stderr, "%s: error getting page size.\n", ds_file);
		} else if (s == 0) {
		    puts("UNKNOWN");
		} else {
		    printf("%lu\n", (unsigned long)s);
		}
	    }
	    break;
	case M_DUMP:
	    rc = dump_wordlist(bfp);
	    break;
	case M_LOAD:
	    rc = load_wordlist(bfp);
	    break;
	case M_MAINTAIN:
	    maintain = true;
	    rc = maintain_wordlist_file(bfp);
	    break;
	case M_WORD:
	    argc -= optind;
	    argv += optind;
	    rc = display_words(bfp, argc, argv, prob);
	    break;
	case M_HIST:
	    rc = histogram(bfp);
	    break;
	case M_ROBX:
	    rc = get_robx(bfp);
	    break;
	case M_NONE:
	default:
	    /* should have been handled above */
	    abort();
	    break;
    }

    bfpath_free(bfp);

    return rc;
}
