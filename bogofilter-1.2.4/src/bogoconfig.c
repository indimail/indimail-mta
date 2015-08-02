/* $Id: bogoconfig.c 6993 2013-06-28 21:54:23Z m-a $ */

/*****************************************************************************

NAME:
   bogoconfig.c -- process config file parameters

   2003-02-12 - split out from config.c

AUTHOR:
   David Relson <relson@osagesoftware.com>

CONTRIBUTORS:
   David Saez	-O option, helps embedding into Exim.

******************************************************************************

The call tree is (roughly):

bogoconfig.c	  process_parameters
bogoconfig.c	    process_arglist(PASS_1_CLI)
bogoconfig.c	      process_arg(PASS_1_CLI)
configfile.c	    process_config_files
configfile.c	      read_config_file
configfile.c	        process_config_option
configfile.c	          process_config_option_as_arg
bogoconfig.c	            process_arg(PASS_2_CFG)
bogoconfig.c	    process_arglist(PASS_2_CLI)
bogoconfig.c	      process_arg(PASS_3_CLI)

Note: bogolexer also uses configfile.c.
      bogolexer.c calls process_config_files(), which calls back to it.

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bogoconfig.h"
#include "bogofilter.h"
#include "bogoreader.h"
#include "bool.h"
#include "charset.h"
#include "datastore.h"
#include "datastore_db.h"
#include "error.h"
#include "find_home.h"
#include "format.h"
#include "lexer.h"
#include "longoptions.h"
#include "maint.h"
#include "paths.h"
#include "score.h"
#include "wordlists.h"
#include "wordlists_base.h"
#include "xatox.h"
#include "xmalloc.h"
#include "xstrdup.h"
#include "xstrlcpy.h"

#ifndef	DEBUG_CONFIG
#define DEBUG_CONFIG(level)	(verbose > level)
#endif

/*---------------------------------------------------------------------------*/

/* Global variables */

char outfname[PATH_LEN] = "";

bool  run_classify = false;
bool  run_register = false;

const char *logtag = NULL;

/* Local variables and declarations */

static int inv_terse_mode = 0;

static void display_tag_array(const char *label, FIELD *array);

static void process_arglist(int argc, char **argv, priority_t precedence, int pass);
static bool get_parsed_value(char **arg, double *parm);
static void comma_parse(char opt, const char *arg, double *parm1, double *parm2, double *parm3);
static bool token_count_conflict(void);

/*---------------------------------------------------------------------------*/

static struct option longopts_bogofilter[] = {
    /* longoptions.h - common options */
    LONGOPTIONS_COMMON
    /* longoptions.h - bogofilter */
    LONGOPTIONS_MAIN
    LONGOPTIONS_MAIN_TUNE
    /* longoptions.h - bogofilter/bogolexer options */
    LONGOPTIONS_LEX
    /* longoptions.h - bogofilter/bogoutil options */
    LONGOPTIONS_DB
    /* bogofilter specific options */
    { "classify-files",			N, 0, 'B' },
    { "syslog-tag",			R, 0, 'L' },
    { "classify-mbox",			N, 0, 'M' },
    { "unregister-nonspam",		N, 0, 'N' },
    { "dataframe",			N, 0, 'R' },
    { "unregister-spam",		N, 0, 'S' },
    { "fixed-terse-format",		N, 0, 'T' },
    { "report-unsure",			N, 0, 'U' },
    { "classify-stdin",			N, 0, 'b' },
    { "bogofilter-dir",			R, 0, 'd' },
    { "nonspam-exits-zero",		N, 0, 'e' },
    { "use-syslog",			N, 0, 'l' },
    { "register-ham",			N, 0, 'n' },
    { "passthrough",			N, 0, 'p' },
    { "register-spam",			N, 0, 's' },
    { "update-as-scored",		N, 0, 'u' },
    { "debug-flags",			R, 0, 'x' },
    { "debug-to-stdout",		N, 0, 'D' },
    { "no-header-tags",			N, 0, 'H' },
    { "query",				N, 0, 'Q' },
    { "db-cachesize",			R, 0, 'k' },
    { "ns-esf",				R, 0, O_NS_ESF },
    { "sp-esf",				R, 0, O_SP_ESF },
    { "ham-cutoff",			R, 0, O_HAM_CUTOFF },
    { "header-format",			R, 0, O_HEADER_FORMAT },
    { "log-header-format",		R, 0, O_LOG_HEADER_FORMAT },
    { "log-update-format",		R, 0, O_LOG_UPDATE_FORMAT },
    { "min-dev",			R, 0, O_MIN_DEV },
    { "robs",				R, 0, O_ROBS },
    { "robx",				R, 0, O_ROBX },
    { "spam-cutoff",			R, 0, O_SPAM_CUTOFF },
    { "spam-header-name",		R, 0, O_SPAM_HEADER_NAME },
    { "spam-header-place",		R, 0, O_SPAM_HEADER_PLACE },
    { "spam-subject-tag",		R, 0, O_SPAM_SUBJECT_TAG },
    { "spamicity-formats",		R, 0, O_SPAMICITY_FORMATS },
    { "spamicity-tags",			R, 0, O_SPAMICITY_TAGS },
    { "stats-in-header",		R, 0, O_STATS_IN_HEADER },
    { "terse",				R, 0, O_TERSE },
    { "terse-format",			R, 0, O_TERSE_FORMAT },
    { "thresh-update",			R, 0, O_THRESH_UPDATE },
    { "timestamp",			R, 0, O_TIMESTAMP },
    { "unsure-subject-tag",		R, 0, O_UNSURE_SUBJECT_TAG },
    { "wordlist",			R, 0, O_WORDLIST },
    /* end of list */
    { NULL,				0, 0, 0 }
};

/*---------------------------------------------------------------------------*/

static bool get_bool(const char *name, const char *arg)
{
    bool b = str_to_bool(arg);
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %s\n", name,
		b ? "Yes" : "No");
    return b;
}

static bool get_double(const char *name, const char *arg, double *d)
{
    remove_comment(arg);
    if (!xatof(d, arg))
	return false;
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %f\n", name, *d);
    return true;
}

static char *get_string(const char *name, const char *arg)
{
    char *s = xstrdup(arg);
    remove_comment(s);

    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> '%s'\n", name, s);
    return s;
}

static e_txn get_txn(const char *name, const char *arg)
{
    e_txn t = get_bool(name, arg) ? T_ENABLED : T_DISABLED;
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %s\n", name,
		t ? "enabled" : "disabled");
    return t;
}

void process_parameters(int argc, char **argv, bool warn_on_error)
{
    bogotest = 0;
    verbose = 0;
    run_type = RUN_UNKNOWN;
    fpin = stdin;
    set_today();		/* compute current date for token age */

#ifdef __EMX__
    _response (&argc, &argv);	/* expand response files (@filename) */
    _wildcard (&argc, &argv);	/* expand wildcards (*.*) */
#endif

    process_arglist(argc, argv, PR_COMMAND, PASS_1_CLI);
    process_config_files(warn_on_error, longopts_bogofilter);
    process_arglist(argc, argv, PR_COMMAND, PASS_3_CLI);

    /* directories from command line and config file are already handled */

    wordlists_set_bogohome();

    stats_prefix= stats_in_header ? "  " : "# ";

    return;
}

static bool get_parsed_value(char **arg, double *parm)
{
    char *str = *arg;
    bool ok = true;
    if (parm && str && *str) {
	if (*str == ',')
	    str += 1;
	else {
	    ok = xatof(parm, str);
	    str = strchr(str+1, ',');
	    if (str)
		str += 1;
	}
	*arg = str;
    }
    return ok;
}

static void comma_parse(char opt, const char *arg, double *parm1, double *parm2, double *parm3)
{
    char *parse = xstrdup(arg);
    char *copy = parse;
    bool ok = ( get_parsed_value(&copy, parm1) &&
		get_parsed_value(&copy, parm2) &&
		get_parsed_value(&copy, parm3) );
    if (!ok)
	fprintf(stderr, "Cannot parse -%c option argument '%s'.\n", opt, arg);
    xfree(parse);
}

static run_t check_run_type(run_t add_type, run_t conflict)
{
    if (run_type & conflict) {
	(void)fprintf(stderr, "Error:  Invalid combination of options.\n");
	exit(EX_ERROR);
    }
    return (run_type | add_type);
}

static int validate_args(void)
{
/*  flags '-s', '-n', '-S', and '-N' are mutually exclusive with
    flags '-p', '-u', '-e', and '-R'. */
    run_classify = (run_type & (RUN_NORMAL | RUN_UPDATE)) != 0;
    run_register = (run_type & (REG_SPAM | REG_GOOD | UNREG_SPAM | UNREG_GOOD)) != 0;

    if (*outfname && !passthrough)
    {
	(void)fprintf(stderr,
		      "Warning: Option -O %s has no effect without -p\n",
		      outfname);
    }
    
    if (run_register && (run_classify || Rtable))
    {
	(void)fprintf(stderr,
		      "Error:  Option '-u' may not be used with options '-s', '-n', '-S', or '-N'.\n"
	    );
	return EX_ERROR;
    }

    return EX_OK;
}

static const char *help_text[] = {
    "help options:\n",
    "  -h, --help                - print this help message.\n",
    "  -V, --version             - print version information and exit.\n",
    "  -Q, --query               - query (display) base bogofilter configuration.\n",
    "  -QQ                       - display extended configuration info.\n",
    "classification options:\n",
    "  -p, --passthrough         - passthrough.\n",
    "  -e, --ham-true            - in -p mode, exit with code 0 when the mail is not spam.\n",
    "  -u, --update-as-scored    - score message as spam or non-spam and register accordingly.\n",
    "  -M, --classify-mbox       - set mailbox mode.  Classify multiple messages in an mbox formatted file.\n",
    "  -b, --classify-stdin      - set streaming bulk mode. Process multiple messages (files or directories) read from STDIN.\n",
    "  -B, --classify-files=list - set bulk mode. Process multiple messages (files or directories) named on the command line.\n",
    "  -R, --dataframe           - print an R data frame.\n",
    "registration options:\n",
    "  -s, --register-spam       - register message(s) as spam.\n",
    "  -n, --register-ham        - register message(s) as non-spam.\n",
    "  -S, --unregister-spam     - unregister message(s) from spam list.\n",
    "  -N, --unregister-nonspam  - unregister message(s) from non-spam list.\n",
    "general options:\n",
    "  -c, --config-file=file    - read specified config file.\n",
    "  -C, --no-config-file      - don't read standard config files.\n",
    "  -d, --bogofilter-dir=path - specify directory for wordlists.\n",
    "  -H, --no-header-tags      - disables header line tagging.\n",
    "  -k, --db-cachesize=size   - set Berkeley DB cache size (MB).\n",
    "  -l, --use-syslog          - write messages to syslog.\n",
    "  -L, --syslog-tag=tag      - specify the tag value for log messages.\n",
    "  -I, --input-file=file     - read message from 'file' instead of stdin.\n",
    "  -O, --output-file=file    - save message to 'file' in passthrough mode.\n",
    "parameter options:\n",
    "  -mv1[,v2[,v3]]            - set user defined min_dev, robs, and robx values.\n",
    "      --min-dev=v1, --robs=v2, --robx=v3\n",
    "  -ov1[,v2]                 - set user defined spam and non-spam cutoff values.\n",
    "      --spam-cutoff=v1, --ham-cutoff=v2\n",
    "info options:\n",
    "  -t, --terse               - set terse output mode.\n",
    "  -T, --fixed-terse-format  - set invariant terse output mode.\n",
    "  -q, --quiet               - suppress token statistics.\n",
    "  -U, --report-unsure       - print statistics if spamicity is 'unsure'.\n",
    "  -v, --verbosity           - set debug verbosity level.\n",
    "  -y, --timestamp-date      - set date for token timestamps.\n",
    "  -D, --debug-to-stdout     - direct debug output to stdout.\n",
    "  -x, --debug-flags=list    - set flags to display debug information.\n",
    "config file options:\n",
    "  --option=value - can be used to set the value of a config file option.\n",
    "                   see bogofilter.cf.example for more info.\n",
    "  --block-on-subnets                return class addr tokens\n",
    "  --bogofilter-dir                  directory for wordlists\n",
    "  --charset-default                 default character set\n",
    "  --db-cachesize                    Berkeley db cache in Mb\n",
#ifdef	HAVE_DECL_DB_CREATE
    "  --db-log-autoremove               enable/disable autoremoval of log files\n",
    "  --db-transaction                  enable/disable transactions\n",
 #ifdef	FUTURE_DB_OPTIONS
    "  --db-txn-durable                                 \n",
 #endif
#endif
    "  --ham-cutoff                      nonspam if score below this\n",
    "  --header-format                   spam header format\n",
    "  --log-header-format               header written to log\n",
    "  --log-update-format               logged on update\n",
    "  --min-dev                         ignore if score near\n",
    "  --min-token-len                   min len for single tokens\n",
    "  --max-token-len                   max len for single tokens\n",
    "  --max-multi-token-len             max len for multi-word tokens\n",
    "  --multi-token-count               number of tokens per multi-word token\n",
    "  --ns-esf                          effective size factor for ham\n",
    "  --replace-nonascii-characters     substitute '?' if bit 8 is 1\n",
    "  --robs                            Robinson's s parameter\n",
    "  --robx                            Robinson's x parameter\n",
    "  --sp-esf                          effective size factor for spam\n",
    "  --spam-cutoff                     spam if score above this\n",
    "  --spam-header-name                passthrough adds/replaces\n",
    "  --spam-subject-tag                passthrough prepends Subject\n",
    "  --spamicity-formats               spamicity output format\n",
    "  --spamicity-tags                  spamicity tag format\n",
    "  --stats-in-header                 use header not body\n",
    "  --terse                           report in short form\n",
    "  --terse-format                    short form\n",
    "  --thresh-update                   no update if near 0 or 1\n",
    "  --timestamp                       enable/disable token timestamps\n",
    "  --token-count                     fixed token count for scoring\n",
    "  --token-count-min                 min token count for scoring\n",
    "  --token-count-max                 max token count for scoring\n",
#ifndef	DISABLE_UNICODE
    "  --unicode                         enable/disable unicode based wordlist\n",
#endif
    "  --unsure-subject-tag              like spam-subject-tag\n",
    "  --user-config-file                configuration file\n",
    "  --wordlist                        specify wordlist parameters\n",
    "\n",
    "bogofilter is a tool for classifying email as spam or non-spam.\n",
    "\n",
    "For updates and additional information, see\n",
    "URL: http://bogofilter.sourceforge.net\n",
    NULL
};

static void help(FILE *fp)
{
    uint i;
    (void)fprintf(fp,
                  "%s version %s\n"
                  "\n"
                  "Usage:  %s [options] < message\n"
                  "\n",
                  progtype, version, PACKAGE
	);
    for (i=0; help_text[i] != NULL; i++)
	(void)fprintf(fp, "%s", help_text[i]);
}

static void print_version(void)
{
    (void)fprintf(stdout,
		  "%s version %s\n"
		  "    Database: %s\n"
		  "Copyright (C) 2002-2010 David Relson, Matthias Andree\n"
		  "Copyright (C) 2002-2004 Greg Louis\n"
		  "Copyright (C) 2002-2003 Eric S. Raymond, Adrian Otto, Gyepi Sam\n\n"
		  "%s comes with ABSOLUTELY NO WARRANTY.  "
		  "This is free software, and\nyou are welcome to "
		  "redistribute it under the General Public License.  "
		  "See\nthe COPYING file with the source distribution for "
		  "details.\n"
		  "\n",
		  progtype, version, ds_version_str(), PACKAGE);
}

#define	OPTIONS	":-:bBc:Cd:DehHI:k:lL:m:MnNo:O:pPqQRsStTuUvVx:X:y:"

/** These functions process command line arguments.
 **
 ** They are called to perform passes 1 & 2 of command line switch processing.
 ** The config file is read in between the two function calls.
 **
 ** The functions will exit if there's an error, for example if
 ** there are leftover command line arguments.
 */

static void process_arglist(int argc, char **argv, priority_t precedence, int pass)
{
    ex_t exitcode;

    if (pass != PASS_1_CLI) {
	optind = opterr = 1;
#ifdef __EMX__
	optind = 0;
#endif
	/* don't use #ifdef here: */
#if HAVE_DECL_OPTRESET
	optreset = 1;
#endif
    }

    while (1) 
    {
	int option;
	int option_index = 0;
	int this_option_optind = optind ? optind : 1;
	const char *name;

	option = getopt_long_chk(argc, argv, OPTIONS,
			     longopts_bogofilter, &option_index);

	if (option == -1)
	    break;

	name = (option_index == 0) ? argv[this_option_optind] : longopts_bogofilter[option_index].name;

#ifdef EXCESSIVE_DEBUG
	if (getenv("BOGOFILTER_DEBUG_OPTIONS")) {
	    fprintf(stderr, "config: option=%c (%d), optind=%d, opterr=%d, optarg=%s\n", 
		    isprint((unsigned char)option) ? option : '_', option, 
		    optind, opterr, optarg ? optarg : "(null)");
	}
#endif

	process_arg(option, name, optarg, precedence, pass);
    }

    if (pass == PASS_1_CLI) {
	if (run_type == RUN_UNKNOWN)
	    run_type = RUN_NORMAL;
    }

    if (pass == PASS_3_CLI) {
	exitcode = validate_args();
	if (exitcode) 
	    exit(exitcode);

	if (bulk_mode == B_NORMAL && optind < argc) {
	    fprintf(stderr, "Extra arguments given, first: %s. Aborting.\n", argv[optind]);
	    exit(EX_ERROR);
	}

	if (inv_terse_mode) {
	    verbose = max(1, verbose); 	/* force printing */
	    set_terse_mode_format(inv_terse_mode);
	}
	
	if (token_count_conflict()) {
	    fprintf(stderr, "Conflicting token count arguments given.\n");
	    exit(EX_ERROR);
	}
    }

    return;
}

int process_arg(int option, const char *name, const char *val, priority_t precedence, arg_pass_t pass)
{
    switch (option)
    {
    case '?':
	if (pass == PASS_1_CLI)
	    fprintf(stderr, "Unknown option '%s'.\n", name);
	break;

    case 'b':
	bulk_mode = B_STDIN;
	fpin = NULL;	/* Ensure that input file isn't stdin */
	break;

    case 'B':
	bulk_mode = B_CMDLINE;
	break;

    case 'c':
    case O_CONFIG_FILE:
	if (pass == PASS_1_CLI) {
	    if (!read_config_file(val, false, !quiet, PR_CFG_USER, longopts_bogofilter)) {
		fprintf(stderr, "Cannot open %s: %s\n", val, strerror(errno));
		exit(EX_ERROR);
	    }
	}

	/*@fallthrough@*/
	/* fall through to suppress reading config files */

    case 'C':
	if (pass == PASS_1_CLI)
	    suppress_config_file = true;
	break;

    case O_USER_CONFIG_FILE:
	user_config_file = get_string(name, val);
	break;

    case 'D':
	dbgout = stdout;
	break;

    case 'e':
    case O_HAM_TRUE:
	nonspam_exits_zero = true;
	break;

    case 'h':
	help(stdout);
	exit(EX_OK);

    case 'I':
	if (pass == PASS_1_CLI)
	    bogoreader_name(val);
	break;

    case 'L':
	logtag = xstrdup(val);
	/*@fallthrough@*/

    case 'l':
	logflag = true;
	break;

    case 'M':
	mbox_mode = true;
	break;

    case 'n':
	run_type = check_run_type(REG_GOOD, REG_SPAM | UNREG_GOOD);
	break;

    case 'N':
	run_type = check_run_type(UNREG_GOOD, REG_GOOD | UNREG_SPAM);
	break;

    case 'O':
	if (pass == PASS_1_CLI)
	    xstrlcpy(outfname, val, sizeof(outfname));
	break;

    case 'p':
	passthrough = true;
	break;

    case 'q':
	quiet = true;
	break;

    case 'Q':
	if (pass == PASS_1_CLI)
	    query += 1;
	break;

    case 'R':
	Rtable = true;
	break;

    case 's':
	run_type = check_run_type(REG_SPAM, REG_GOOD | UNREG_SPAM);
	break;

    case 'S':
	run_type = check_run_type(UNREG_SPAM, REG_SPAM | UNREG_GOOD);
	break;

    case 'u':
	run_type |= RUN_UPDATE;
	break;
	
    case 'U':
	unsure_stats = (val == NULL) ? true : get_bool(name, val);
	break;

    case 'v':
	if (pass == PASS_1_CLI)
	    verbose++;
	break;

    case 'x':
	set_debug_mask( val );
	break;

    case 'X':
	set_bogotest( val );
	break;

    case 'y':		/* date as YYYYMMDD */
    {
	const char *str = val ? val : optarg;
	YYYYMMDD date = string_to_date(str);
	if (date != 0 && date < 19990000) {
	    fprintf(stderr, "Date format for '-y' option is YYYYMMDD\n");
	    exit(EX_ERROR);
	}
	set_date( date );
	break;
    }
	
    case ':':
	fprintf(stderr, "Option %s requires an argument.\n", name);
	exit(EX_ERROR);

    case '-':
	if (pass == PASS_3_CLI)
	    process_config_option(val, true, precedence, longopts_bogofilter);
	break;

    case 'd':
	if (pass != PASS_1_CLI)
	    set_wordlist_dir(val, precedence);
	break;

    case O_NS_ESF:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &ns_esf);
	break;

    case O_SP_ESF:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &sp_esf);
	break;

    case 'H':
	header_line_markup = (val == NULL) ? false : get_bool(name, val);
	break;

    case 'k':
	db_cachesize=atoi(val);
	break;

    case 'm':
	if (pass != PASS_1_CLI) {
	    comma_parse(option, val, &min_dev, &robs, &robx);
	    if (DEBUG_CONFIG(1))
		fprintf(dbgout, "md %6.4f, rs %6.4f, rx %6.4f\n", min_dev, robs, robx);
	}
	break;

    case O_MIN_DEV:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &min_dev);
	break;

    case O_ROBS:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &robs);
	break;

    case O_ROBX:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &robx);
	break;

    case 'o':
	if (pass != PASS_1_CLI) {
	    comma_parse(option, val, &spam_cutoff, &ham_cutoff, NULL);
	    if (DEBUG_CONFIG(1))
		fprintf(dbgout, "sc %6.4f, hc %6.4f\n", spam_cutoff, ham_cutoff);
	}
	break;

    case O_SPAM_CUTOFF:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &spam_cutoff);
	break;

    case O_HAM_CUTOFF:
	if (pass != PASS_1_CLI)
	    get_double(name, val, &ham_cutoff);
	break;

    case 't':
	terse = true;
	break;

    case 'T':			/* invariant terse mode */
	terse = true;
	if (pass == PASS_1_CLI)
	    inv_terse_mode += 1;
	break;

    case 'V':
	print_version();
	exit(EX_OK);

    case O_BLOCK_ON_SUBNETS:		block_on_subnets = get_bool(name, val);			break;
    case O_CHARSET_DEFAULT:		charset_default = get_string(name, val);		break;
    case O_HEADER_FORMAT:		header_format = get_string(name, val);			break;
    case O_LOG_HEADER_FORMAT:		log_header_format = get_string(name, val);		break;
    case O_LOG_UPDATE_FORMAT:		log_update_format = get_string(name, val);		break;
    case O_MAX_TOKEN_LEN:		max_token_len=atoi(val);				break;
    case O_MIN_TOKEN_LEN:		min_token_len=atoi(val);				break;
    case O_MAX_MULTI_TOKEN_LEN:		max_multi_token_len=atoi(val);				break;
    case O_MULTI_TOKEN_COUNT:		multi_token_count=atoi(val);				break;
    case O_REPLACE_NONASCII_CHARACTERS:	replace_nonascii_characters = get_bool(name, val);	break;
    case O_SPAMICITY_FORMATS:		set_spamicity_formats(val);				break;
    case O_SPAMICITY_TAGS:		set_spamicity_tags(val);				break;
    case O_SPAM_HEADER_NAME:		spam_header_name = get_string(name, val);		break;
    case O_SPAM_HEADER_PLACE:		spam_header_place = get_string(name, val);		break;
    case O_SPAM_SUBJECT_TAG:		spam_subject_tag = get_string(name, val);		break;
    case O_STATS_IN_HEADER:		stats_in_header = get_bool(name, val);			break;
    case O_TERSE:			terse = get_bool(name, val);				break;
    case O_TERSE_FORMAT:		terse_format = get_string(name, val);			break;
    case O_THRESH_UPDATE:		get_double(name, val, &thresh_update);			break;
    case O_TIMESTAMP:			timestamp_tokens = get_bool(name, val);			break;
    case O_TOKEN_COUNT_FIX:             token_count_fix = atoi(val);                            break;
    case O_TOKEN_COUNT_MIN:             token_count_min = atoi(val);                            break;
    case O_TOKEN_COUNT_MAX:             token_count_max = atoi(val);                            break;
    case O_UNSURE_SUBJECT_TAG:		unsure_subject_tag = get_string(name, val);		break;
    case O_UNICODE:			encoding = get_bool(name, val) ? E_UNICODE : E_RAW;	break;
    case O_WORDLIST:			configure_wordlist(val);				break;

    case O_DB_TRANSACTION:		eTransaction = get_txn(name, val);			break;

    default:
#ifndef	DISABLE_TRANSACTIONS
	if (!dsm_options_bogofilter(option, name, val))
#endif
	{
	    fprintf(stderr, "Invalid option '%s'.\n", name);
	    exit(EX_ERROR);
	}
    }

    return 0;
}

#define	Q1	if (query >= 1)
#define	Q2	if (query >= 2)
#define	Q3	if (query >= 3)

#define YN(b) (b ? "Yes" : "No")
#define NB(b) ((b != NULL && *b != '\0') ? b : "''")

rc_t query_config(void)
{
    Q1 fprintf(stdout, "# %s version %s\n", progname, version);
    Q1 fprintf(stdout, "\n");
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "robx", robx, robx);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "robs", robs, robs);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "min_dev", min_dev, min_dev);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "ham_cutoff", ham_cutoff, ham_cutoff);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "spam_cutoff", spam_cutoff, spam_cutoff);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "ns_esf", ns_esf, ns_esf);
    Q1 fprintf(stdout, "%-11s = %0.6f  # (%8.2e)\n", "sp_esf", sp_esf, sp_esf);
    Q1 fprintf(stdout, "\n");
    Q3 fprintf(stdout, "%-17s = %d\n",    "token-count",         token_count_fix);
    Q3 fprintf(stdout, "%-17s = %d\n",    "token-count-min",     token_count_min);
    Q3 fprintf(stdout, "%-17s = %d\n",    "token-count-max",     token_count_max);
    Q3 fprintf(stdout, "\n");
    Q1 fprintf(stdout, "%-17s = %s\n",    "block-on-subnets",    YN(block_on_subnets));
    Q1 fprintf(stdout, "%-17s = %s\n",    "encoding",		 (encoding != E_UNICODE) ? "raw" : "utf-8");
    Q1 fprintf(stdout, "%-17s = %s\n",    "charset-default",     charset_default);
    Q1 fprintf(stdout, "%-17s = %s\n",    "replace-nonascii-characters", YN(replace_nonascii_characters));
    Q2 fprintf(stdout, "%-17s = %s\n",    "no-header-tags",      YN(header_line_markup));
    Q1 fprintf(stdout, "%-17s = %s\n",    "stats-in-header",     YN(stats_in_header));
    Q2 fprintf(stdout, "%-17s = %s\n",    "report-unsure",       YN(unsure_stats));
    Q1 fprintf(stdout, "%-17s = %0.6f\n", "thresh-update",       thresh_update);
    Q1 fprintf(stdout, "%-17s = %s\n",    "timestamp",           YN(timestamp_tokens));
    Q2 fprintf(stdout, "%-17s = %ld\n",   "timestamp-date",      (long int)today);
    Q1 fprintf(stdout, "\n");
    Q1 fprintf(stdout, "%-17s = %s\n", "spam-header-name",    spam_header_name);
    Q1 fprintf(stdout, "%-17s = %s\n", "spam-header-place",   NB(spam_header_place));
    Q1 fprintf(stdout, "%-17s = %s\n", "spam-subject-tag",    NB(spam_subject_tag));
    Q1 fprintf(stdout, "%-17s = %s\n", "unsure-subject-tag",  NB(unsure_subject_tag));
    Q2 fprintf(stdout, "%-18s = %s\n", "syslog-tag",          NB(logtag));
    Q1 fprintf(stdout, "%-17s = %s\n", "header-format",       header_format);
    Q1 fprintf(stdout, "%-17s = %s\n", "terse-format",        terse_format);
    Q1 fprintf(stdout, "%-17s = %s\n", "log-header-format",   log_header_format);
    Q1 fprintf(stdout, "%-17s = %s\n", "log-update-format",   log_update_format);
    Q1 display_tag_array("spamicity-tags   ", spamicity_tags);
    Q1 display_tag_array("spamicity-formats", spamicity_formats);

    Q2 fprintf(stdout, "\n");

    Q2 fprintf(stdout, "%-18s = %s\n", "no-config-file",   YN(suppress_config_file));
    Q2 fprintf(stdout, "%-18s = %s\n", "config-file",      NB(config_file_name));
    Q2 fprintf(stdout, "%-18s = %s\n", "user-config-file", NB(user_config_file));
    Q2 fprintf(stdout, "\n");

    Q2 fprintf(stdout, "%-18s = %s\n", "bogofilter-dir",        bogohome);
    Q2 display_wordlists(word_lists, "%-18s   ");
    Q2 fprintf(stdout, "\n");

#ifndef	DISABLE_TRANSACTIONS
    Q2 fprintf(stdout, "%-18s = %lu\n", "db-cachesize",         (unsigned long)db_cachesize);

#ifdef	ENABLE_TRANSACTIONS
#ifdef	HAVE_DECL_DB_CREATE
    Q2 fprintf(stdout, "%-18s = %s\n", "db-log-autoremove",     YN(db_log_autoremove));
#ifdef	FUTURE_DB_OPTIONS
    Q2 fprintf(stdout, "%-18s = %s\n", "db-log-txn-durable",    YN(db_txn_durable));
#endif
#endif
#endif
#endif

    return RC_OK;
}

static void display_tag_array(const char *label, FIELD *array)
{
    int i;
    int count = (ham_cutoff < EPS) ? 2 : 3;

    fprintf(stdout, "%s =", label);
    for (i = 0; i < count; i += 1)
	fprintf(stdout, "%s %s", (i == 0) ? "" : ",", array[i]);
    fprintf(stdout, "\n");
}

static bool token_count_conflict(void)
{
    if (token_count_fix != 0) {
	if (token_count_fix < token_count_min)
	    return true;
    }

    if (token_count_max != 0) {
	if (token_count_max < token_count_min)
	    return true;
	if (token_count_max < token_count_fix)
	    return true;
    }

    return false;
}
