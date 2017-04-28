/*
 * $Log: envrules.c,v $
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
#include "qregex.h"
#include "error.h"
#include "matchregex.h"
#include "stralloc.h"
#include "env.h"
#include "str.h"
#include "control.h"
#include "envrules.h"
#include <fnmatch.h>

int             wildmat_internal(char *, char *);
static int      parse_env(char *);

int
do_match(int use_regex, char *text, char *regex, char **errStr)
{
	int             i;

	if (errStr)
		*errStr = 0;
	if (use_regex)
		return (matchregex(text, regex, errStr));
	else
	if (env_get("USE_FNMATCH"))
	{
#ifdef FNM_CASEFOLD
		i = FNM_PATHNAME|FNM_CASEFOLD;
#else
		i = FNM_PATHNAME;
#endif
		switch(fnmatch(regex, text, i))
		{
		case 0:
			return (1);
		case FNM_NOMATCH:
			return (0);
		default:
			return (AM_REGEX_ERR);
		}
	} else
		return (wildmat_internal(text, regex));
}

int
envrules(char *email, char *envrules_f, char *rulesfile, char **errStr)
{
	int             len, count, nullflag, use_regex = 0;
	char           *ptr, *cptr;
	static stralloc rules = { 0 };

	if (errStr)
		*errStr = 0;
	if (!(ptr = env_get(rulesfile)))
		ptr = envrules_f;
	if ((count = control_readfile(&rules, ptr, 0)) == -1)
	{
		if (errStr)
			*errStr = error_str(errno);
		return (AM_FILE_ERR);
	}
	if (!count)
		return(0);
	if (env_get("QREGEX"))
		use_regex = 1;
	for (count = len = 0, ptr = rules.s;len < rules.len;)
	{
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
		if (nullflag || do_match(use_regex, email, ptr, errStr))
		{
			if (parse_env(cptr + 1))
				return(AM_MEMORY_ERR);
			count++;
		}
		ptr = rules.s + len;
	}
	return(count);
}

int
domainqueue(char *email, char *domainqueue_f, char *domainqueue, char **errStr)
{
	int             len, count;
	char           *ptr, *cptr, *domain;
	static stralloc rules = { 0 };

	if (errStr)
		*errStr = 0;
	if (!(ptr = env_get(domainqueue)))
		ptr = domainqueue_f;
	if ((count = control_readfile(&rules, ptr, 0)) == -1)
	{
		if (errStr)
			*errStr = error_str(errno);
		return (AM_FILE_ERR);
	}
	if (!count)
		return(0);
	for (domain = email;*domain && *domain != '@';domain++);
	if (!*domain)
		return (0);
	else
		domain++;
	for (count = len = 0, ptr = rules.s;len < rules.len;)
	{
		len += (str_len(ptr) + 1);
		for (cptr = ptr;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		if (do_match(0, domain, ptr, errStr))
		{
			if (parse_env(cptr + 1))
				return(AM_MEMORY_ERR);
			count++;
		}
		ptr = rules.s + len;
	}
	return(count);
}

static int
parse_env(char *envStrings)
{
	char           *ptr1, *ptr2, *ptr3, *ptr4;

	for (ptr2 = ptr1 = envStrings;*ptr1;ptr1++)
	{
		if (*ptr1 == ',')
		{
			/*
			 * Allow ',' in environment variable if escaped
			 * by '\' character
			 */
			if (ptr1 != envStrings && *(ptr1 - 1) == '\\')
			{
				for (ptr3 = ptr1 - 1, ptr4 = ptr1; *ptr3; *ptr3++ = *ptr4++);
				continue;
			}
			*ptr1 = 0;
			/*- envar=, - Unset the environment variable */
			if (ptr1 != envStrings && *(ptr1 - 1) == '=')
			{
				*(ptr1 - 1) = 0;
				if (*ptr2 && !env_unset(ptr2))
					return (1);
			} else /*- envar=val, - Set the environment variable */
			if (*ptr2 && !env_put(ptr2))
				return (1);
			ptr2 = ptr1 + 1;
		}
	}
	/*- envar=, */
	if (ptr1 != envStrings && *(ptr1 - 1) == '=')
	{
		*(ptr1 - 1) = 0;
		if (*ptr2 && !env_unset(ptr2))
			return (1);
	} else /*- envar=val, */
	if (*ptr2 && !env_put(ptr2))
		return (1);
	return(0);
}

void
getversion_envrules_c()
{
	static char    *x = "$Id: envrules.c,v 1.17 2015-04-14 20:01:15+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
