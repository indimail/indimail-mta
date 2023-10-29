/*
 * $Log: envrules.c,v $
 * Revision 1.22  2023-10-30 01:21:28+05:30  Cprogrammer
 * use QREGEX to use regular expressions, else use wildmat
 *
 * Revision 1.21  2023-01-13 12:09:25+05:30  Cprogrammer
 * moved parse_env function to parse_env.c
 *
 * Revision 1.20  2021-05-23 07:09:26+05:30  Cprogrammer
 * include wildmat.h for wildmat_internal
 *
 * Revision 1.19  2019-03-07 00:49:37+05:30  Cprogrammer
 * do not treat regcomp error as matches
 *
 * Revision 1.18  2018-11-29 15:21:26+05:30  Cprogrammer
 * skip white spaces in envrules
 *
 * Revision 1.17  2015-04-14 20:01:15+05:30  Cprogrammer
 * fixed matching of null address
 *
 * Revision 1.16  2014-03-07 02:07:16+05:30  Cprogrammer
 * use env variable USE_FNMATCH to use fnmatch() for pattern matching
 *
 * Revision 1.15  2014-01-29 14:06:00+05:30  Cprogrammer
 * made domainqueue file configurable through env variable DOMAINQUEUE
 *
 * Revision 1.14  2013-11-21 15:40:12+05:30  Cprogrammer
 * added domainqueue functionality
 *
 * Revision 1.13  2010-06-18 19:37:58+05:30  Cprogrammer
 * initialize errStr
 *
 * Revision 1.12  2009-05-01 12:52:03+05:30  Cprogrammer
 * return error if control_readfile() fails
 *
 * Revision 1.11  2009-05-01 10:39:44+05:30  Cprogrammer
 * flipped memory error and file open error
 * added errstr argument to envrules() to relay back errors
 *
 * Revision 1.10  2008-06-11 18:18:32+05:30  Cprogrammer
 * added configurable environment variable name for default rules file
 *
 * Revision 1.9  2008-05-21 16:06:05+05:30  Cprogrammer
 * trivial change
 *
 * Revision 1.8  2007-12-21 14:35:21+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.7  2006-01-22 11:41:37+05:30  Cprogrammer
 * added ability to have ',' character in environment
 *
 * Revision 1.6  2004-10-22 20:24:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-09 00:57:03+05:30  Cprogrammer
 * removed param.h
 *
 * Revision 1.4  2004-10-08 19:19:46+05:30  Cprogrammer
 * prevent segmentation fault for luser mistakes
 *
 * Revision 1.3  2004-10-02 18:49:58+05:30  Cprogrammer
 * added regular expression for envrules
 *
 * Revision 1.2  2004-05-23 22:16:30+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.1  2004-02-05 18:47:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <error.h>
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include "parse_env.h"
#include "control.h"
#include "envrules.h"
#include "do_match.h"

int
envrules(char *email, char *envrules_f, char *rulesfile, char **errStr)
{
	int             len, count, lcount, nullflag, use_regex = 0;
	char           *ptr, *cptr;
	static stralloc rules = { 0 };

	if (errStr)
		*errStr = 0;
	if (!(ptr = env_get(rulesfile)))
		ptr = envrules_f;
	if ((count = control_readfile(&rules, ptr, 0)) == -1) {
		if (errStr)
			*errStr = error_str(errno);
		return (AM_FILE_ERR);
	}
	if (!count)
		return (0);
	if (env_get("QREGEX"))
		use_regex = 1;
	for (count = lcount = len = 0, ptr = rules.s; len < rules.len;) {
		len += (str_len(ptr) + 1);
		for (cptr = ptr;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		if (!*email && (!*ptr || !str_diffn(ptr, "<>", 3)))
			nullflag = 1;
		else
			nullflag = 0;
		if (nullflag || do_match(use_regex, email, ptr, errStr) > 0) {
			if (parse_env(cptr + 1))
				return (AM_MEMORY_ERR);
			count = lcount + 1; /*- set line no where match occured*/
		}
		ptr = rules.s + len;
		lcount++;
	}
	return (count);
}

int
domainqueue(char *email, char *domainqueue_f, char *domainqueue, char **errStr)
{
	int             len, lcount, count;
	char           *ptr, *cptr, *domain;
	static stralloc rules = { 0 };

	if (errStr)
		*errStr = 0;
	if (!(ptr = env_get(domainqueue)))
		ptr = domainqueue_f;
	if ((count = control_readfile(&rules, ptr, 0)) == -1) {
		if (errStr)
			*errStr = error_str(errno);
		return (AM_FILE_ERR);
	}
	if (!count)
		return (0);
	for (domain = email;*domain && *domain != '@';domain++);
	if (!*domain)
		return (0);
	else
		domain++;
	for (count = lcount = len = 0, ptr = rules.s; len < rules.len;) {
		len += (str_len(ptr) + 1);
		for (cptr = ptr;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		if (do_match(0, domain, ptr, errStr) > 0) {
			if (parse_env(cptr + 1))
				return (AM_MEMORY_ERR);
			count = lcount + 1; /*- set line no where match occured*/
		}
		ptr = rules.s + len;
		lcount++;
	}
	return (count);
}

void
getversion_envrules_c()
{
	static char    *x = "$Id: envrules.c,v 1.22 2023-10-30 01:21:28+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
