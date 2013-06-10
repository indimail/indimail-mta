/*
 * $Log: parse_quota.c,v $
 * Revision 2.7  2013-06-10 15:45:07+05:30  Cprogrammer
 * return 0 for quota=NOQUOTA
 *
 * Revision 2.6  2012-04-22 13:58:14+05:30  Cprogrammer
 * added case for specifying quota in gigabytes
 *
 * Revision 2.5  2011-02-11 23:00:08+05:30  Cprogrammer
 * corrected mathematical calculation for 'k', 'K', 'm', 'M' modifier for quota
 *
 * Revision 2.4  2009-10-15 10:47:32+05:30  Cprogrammer
 * skip LLONG_MAX, LLONG_MIN if not defined and use ERANGE
 *
 * Revision 2.3  2009-10-14 20:44:57+05:30  Cprogrammer
 * use strtoll() instead of atol() for better error checking
 *
 * Revision 2.2  2008-06-24 21:48:53+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.1  2002-08-11 00:26:55+05:30  Cprogrammer
 * added option to skip setting count
 *
 * Revision 1.1  2002-02-24 02:50:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <string.h>
#if !defined(LLONG_MIN) && !defined(LLONG_MAX)
#include <errno.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: parse_quota.c,v 2.7 2013-06-10 15:45:07+05:30 Cprogrammer Exp mbhangui $";
#endif

mdir_t
parse_quota(char *quota, mdir_t *count)
{
	char           *ptr;
	char            tmpbuf[MAX_BUFF];
	mdir_t          per_user_limit;
	int             i;

	scopy(tmpbuf, quota, MAX_BUFF);
	if ((ptr = strchr(tmpbuf, ',')))
	{
		*ptr = 0;
		if(count)
		{
			*count = strtoll(ptr + 1, 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
			if (*count == LLONG_MIN || *count == LLONG_MAX)
#else
			if (errno == ERANGE)
#endif
				return (-1);
		}
	} else
	if(count)	
		*count = 0;
	if (!strncmp(tmpbuf, "NOQUOTA", 8))
		return (0); /*- NOQUOTA */
	per_user_limit = strtoll(tmpbuf, 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (per_user_limit == LLONG_MIN || per_user_limit == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
		return (-1);
	for (i = 0; quota[i] != 0; ++i)
	{
		if (quota[i] == 'k' || quota[i] == 'K')
		{
			per_user_limit = per_user_limit * 1024;
			break;
		}
		if (quota[i] == 'm' || quota[i] == 'M')
		{
			per_user_limit = per_user_limit * 1048576;
			break;
		}
		if (quota[i] == 'g' || quota[i] == 'G')
		{
			per_user_limit = per_user_limit * 1073741824;
			break;
		}
	}
	return (per_user_limit);
}

void
getversion_parse_quota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
