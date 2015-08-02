/* $Id: bogolexer.c 6993 2013-06-28 21:54:23Z m-a $ */

/*****************************************************************************

NAME:
   bogolexer.c -- runs bogofilter's lexer.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include "getopt.h"
#include <stdlib.h>
#include <string.h>

#include "bogoconfig.h"
#include "bogoreader.h"
#include "bool.h"
#include "charset.h"
#include "configfile.h"
#include "lexer.h"
#include "longoptions.h"
#include "mime.h"
#include "textblock.h"
#include "token.h"
#include "format.h"
#include "xstrdup.h"

const char *progname = "bogolexer";

/* Function Definitions */

static void usage(void)
{
    fprintf(stdout, "Usage: %s [ -p | -q | -n | -h ]\n", progname);
}

static void help(void)
{
    usage();
    fprintf(stdout,
	    "\n"
	    "\t-p\t- print the tokens from stdin.\n"
	    "\t-q\t- quiet mode, no tokens are printed.\n"
	    "\t-h\t- help, this output.\n"
	    "\t-n\t- map non-ascii characters to '?'.\n"
	    "\t-v\t- set verbosity level.\n"
	    "\t-c file\t- read specified config file.\n"
	    "\t-C\t- don't read standard config files.\n"
	    "\t-H\t- disables header line tagging.\n"
	    "\t-I file\t- read message from file instead of stdin.\n"
	    "\t-O file\t- write to file instead of stdout.\n"
	    "\t-x list\t- set debug flags.\n"
	    "\t-D\t- direct debug output to stdout.\n");
    fprintf(stdout,
	    "\n"
	    "%s (version %s) is part of the bogofilter package.\n", 
	    progname, version);
}

static void print_version(void)
{
    (void)fprintf(stdout,
		  "%s version %s\n"
		  "Copyright (C) 2002-2010 David Relson\n\n"
		  "%s comes with ABSOLUTELY NO WARRANTY.  "
		  "This is free software, and\nyou are welcome to "
		  "redistribute it under the General Public License.  "
		  "See\nthe COPYING file with the source distribution for "
		  "details.\n"
		  "\n", 
		  progname, version, PACKAGE);
}

static bool get_bool(const char *name, const char *arg)
{
    bool b = str_to_bool(arg);
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %s\n", name,
		b ? "Yes" : "No");
    return b;
}

static char *get_string(const char *name, const char *arg)
{
    char *s = xstrdup(arg);
    remove_comment(s);

    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> '%s'\n", name, s);
    return s;
}

static struct option longopts_bogolexer[] = {
    /* longoptions.h - common options */
    LONGOPTIONS_COMMON
    /* longoptions.h - options for bogofilter and bogolexer */
    LONGOPTIONS_LEX
    /* longoptions.h - options for bogolexer and bogoutil */
    LONGOPTIONS_LEX_UTIL
    /* end of list */
    { NULL,				0, 0, 0 }
};

#define	OPTIONS	":c:CDhHI:nO:pqvVx:X:m"

/** These functions process command line arguments.
 **
 ** They are called to perform passes 1 & 2 of command line switch processing.
 ** The config file is read in between the two function calls.
 **
 ** The functions will exit if there's an error, for example if
 ** there are leftover command line arguments.
 */

static void process_arglist(int argc, char **argv)
{
    int option;

    fpin = stdin;
    dbgout = stderr;

#ifdef __EMX__
    _response (&argc, &argv);	/* expand response files (@filename) */
    _wildcard (&argc, &argv);	/* expand wildcards (*.*) */
#endif

    while (1)
    {
	int option_index = 0;
	int this_option_optind = optind ? optind : 1;
	const char *name;

#ifdef __EMX__
	if (optind == 1) optind = 0;
#endif

	option = getopt_long_chk(argc, argv, OPTIONS,
			     longopts_bogolexer, &option_index);

	if (option == -1)
	    break;

	name = (option_index == 0) ? argv[this_option_optind] : longopts_bogolexer[option_index].name;
	process_arg(option, name, optarg, PR_COMMAND, PASS_1_CLI);
    }

    if (optind < argc) {
	fprintf(stderr, "Extra arguments given, first: %s. Aborting.\n",
		argv[optind]);
	exit(EX_ERROR);
    }
}

int process_arg(int option, const char *name, const char *val, priority_t precedence, arg_pass_t pass)
{
    pass = 0;		/* suppress compiler warning */

    switch (option)
    {
    case ':':
	fprintf(stderr, "Option %s requires an argument.\n", name);
	exit(EX_ERROR);

    case '?':
	fprintf(stderr, "Unknown option '%s'.\n", name);
	break;

    case 'c':
    case O_CONFIG_FILE:
	read_config_file(val, false, false, precedence, longopts_bogolexer);
	/*@fallthrough@*/
	/* fall through to suppress reading config files */

    case 'C':
	suppress_config_file = true;
	break;

    case O_USER_CONFIG_FILE:
	user_config_file = get_string(name, val);
	break;

    case 'D':
	dbgout = stdout;
	break;

    case 'h':
	help();
	exit(EX_OK);

    case 'H':
	header_line_markup = false;
	break;

    case 'I':
	bogoreader_name(val);
	break;

    case 'O':
	fpo = fopen(val, "wt");
	if (fpo == NULL) {
	    fprintf(stderr, "Can't write file '%s'\n", val);
	    exit(EX_ERROR);
	}
	break;

    case 'n':
	replace_nonascii_characters = true;
	break;

    case O_CHARSET_DEFAULT:
	charset_default = get_string(name, val);
	break;

    case O_UNICODE:
	encoding = get_bool(name, val) ? E_UNICODE : E_RAW;
	break;

    case O_REPLACE_NONASCII_CHARACTERS:
	replace_nonascii_characters = get_bool(name, val);
	break;

    case 'p':
	passthrough = true;
	break;

    case 'q':
	quiet = true;
	break;

    case 'v':
	verbose += 1;
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

    case O_BLOCK_ON_SUBNETS:
	block_on_subnets = get_bool(name, val);
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
	/* config file options:
	**  ok    - if from config file
	**  error - if on command line
	*/
	if (pass == PASS_2_CFG) {
	    fprintf(stderr, "Invalid option '%s'.\n", name);
	    exit(EX_ERROR);
	}
    }

    return 0;
}

static int count=0;

int main(int argc, char **argv)
{
    token_t t;

    fpo = stdout;

    mbox_mode = true;		/* to allow multiple messages */

    process_arglist(argc, argv);
    process_config_files(false, longopts_bogolexer);

    if (encoding == E_UNKNOWN)
	encoding = E_DEFAULT;

    textblock_init();

    if (!passthrough)
    {
	if (quiet)
	    fprintf(fpo, "quiet mode.\n");
	else
	    fprintf(fpo, "normal mode.\n");
    }

    bogoreader_init(argc, (const char * const *) argv);

    while ((*reader_more)()) {
	word_t token;
	lexer_init();

	while ((t = get_token( &token )) != NONE)
	{
	    count += 1;
	    if (passthrough) {
		fprintf(fpo, "%s\n", token.u.text);
	    }
	    else if (!quiet)
		fprintf(fpo, "get_token: %d \"%s\"\n", (int)t, token.u.text);
	}
    }

    if ( !passthrough )
	fprintf(fpo, "%d tokens read.\n", count);

    /* cleanup storage */
    token_cleanup();
    mime_cleanup();
    textblock_free();

    MEMDISPLAY;

    return 0;
}
