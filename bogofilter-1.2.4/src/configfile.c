/* $Id: configfile.c 6876 2010-02-15 19:16:16Z m-a $ */

/*****************************************************************************

NAME:
   configfile.c -- process config file parameters

   2003-02-12 - split out from config.c so bogolexer use the code.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "getopt.h"

#include "bogoconfig.h"
#include "bogofilter.h"
#include "bool.h"
#include "maint.h"
#include "error.h"
#include "find_home.h"
#include "format.h"
#include "lexer.h"
#include "longoptions.h"
#include "maint.h"
#include "wordlists.h"
#include "xatox.h"
#include "xmalloc.h"
#include "xstrdup.h"

#ifndef	DEBUG_CONFIG
#define DEBUG_CONFIG(level)	(verbose > level)
#endif

/*---------------------------------------------------------------------------*/

/* NOTE: MAXBUFFLEN _MUST_ _NOT_ BE LARGER THAN INT_MAX! */
#define	MAXBUFFLEN	((int)200)

/*---------------------------------------------------------------------------*/

/* Global variables */

#ifndef __riscos__
const char *user_config_file   = "~/.bogofilter.cf";
#else
const char *user_config_file   = "Choices:bogofilter.bogofilter/cf";
#endif

bool	stats_in_header = true;
char 	*config_file_name;

/*---------------------------------------------------------------------------*/

/*  remove trailing comment from the line.
 */
void remove_comment(const char *line)
{
    char *tmp = strchr(line, '#');
    if (tmp != NULL) {
	tmp -= 1;
	while (tmp > line && isspace((unsigned char)*tmp))
	    tmp -= 1;
	*(tmp+1) = '\0';
    }
    return;
}

bool process_config_option(const char *arg, bool warn_on_error, priority_t precedence, struct option *longopts)
{
    uint pos;
    bool ok = true;

    char *val = NULL;
    const char *opt = arg;
    char *dupl;
    const char delim[] = " \t=";

    while (isspace((unsigned char)*opt))		/* ignore leading whitespace */
	opt += 1;

    dupl = xstrdup(opt);
    pos = strcspn(dupl, delim);
    if (pos < strlen(dupl)) { 		/* if delimiter present */
	val = dupl + pos;
	*val++ = '\0';
	val += strspn(val, delim);
    }

    if (val == NULL ||
	!process_config_option_as_arg(dupl, val, precedence, longopts)) {
	ok = false;
	if (warn_on_error)
	    fprintf(stderr, "Error - bad parameter '%s'\n", arg);
    }

    xfree(dupl);
    return ok;
}

/* option_compare()
**
** Returns true if options are equal, without regard to case and
** allows underscore from config file to match hyphen
*/

static bool option_compare(const char *opt, const char *name)
{
    char co, cn;
    if (strlen(opt) != strlen(name))
	return false;
    while (((co = *opt++) != '\0') && ((cn = *name++) != '\0')) {
	if ((co == cn) || (tolower((unsigned char)co) == tolower((unsigned char)cn)))
	    continue;
	if (co != '_' || cn != '-')
	    return false;
    }
    return true;
}

bool process_config_option_as_arg(const char *opt, const char *val, priority_t precedence, struct option *long_options)
{
    struct option *option;

    for (option = long_options; option->name; option += 1) {
	if (!option_compare(opt, option->name))
	    continue;
	if (strcmp(val, "''") == 0)
	    val = "";
	process_arg(option->val, option->name, val, precedence, PASS_2_CFG);
	return true;
    }

    return false;
}

bool read_config_file(const char *fname, bool tilde_expand, bool warn_on_error, priority_t precedence, struct option *longopts)
{
    bool ok = true;
    int lineno = 0;
    FILE *fp;

    if (config_file_name != NULL)
	xfree(config_file_name);

    if (!tilde_expand)
	config_file_name = xstrdup(fname);
    else
	config_file_name = tildeexpand(fname);

    fp = fopen(config_file_name, "r");

    if (fp == NULL) {
	xfree(config_file_name);
	config_file_name = NULL;
	return false;
    }

    if (DEBUG_CONFIG(0))
	fprintf(dbgout, "Reading %s\n", config_file_name);

    while (!feof(fp))
    {
	size_t len;
	char buff[MAXBUFFLEN];

	lineno += 1;
	if (fgets(buff, sizeof(buff), fp) == NULL)
	    break;
	len = strlen(buff);
	if ( buff[0] == '#' || buff[0] == ';' || buff[0] == '\n' )
	    continue;
	while (len >= 1
		&& (iscntrl((unsigned char)buff[len-1])
		    || isspace((unsigned char)buff[len-1])))
	    buff[--len] = '\0';

	if (DEBUG_CONFIG(1))
	    fprintf(dbgout, "Testing:  %s\n", buff);

	if (!process_config_option(buff, warn_on_error, precedence, longopts))
	    ok = false;
    }

    if (ferror(fp)) {
	fprintf(stderr, "Error reading file \"%s\"\n.", config_file_name);
	ok = false;
    }

    (void)fclose(fp); /* we're just reading, so fclose should succeed */

    return ok;
}

/* exported */
bool process_config_files(bool warn_on_error, struct option *longopts)
{
    bool ok = true;
    const char *env = getenv("BOGOTEST");

    if (!suppress_config_file) {
	if (!read_config_file(system_config_file, false, warn_on_error, PR_CFG_SITE, longopts))
	    ok = false;
	if (!read_config_file(user_config_file, true, warn_on_error, PR_CFG_USER, longopts))
	    ok = false;
    }

    if (env)
	set_bogotest(env);

    return ok;
}
