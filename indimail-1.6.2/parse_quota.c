/*
 * $Log: parse_quota.c,v $
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
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: parse_quota.c,v 2.2 2008-06-24 21:48:53+05:30 Cprogrammer Stab mbhangui $";
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
			*count = atol(ptr + 1);
	} else
	if(count)	
		*count = 0;
	per_user_limit = atol(tmpbuf);
	for (i = 0; quota[i] != 0; ++i)
	{
		if (quota[i] == 'k' || quota[i] == 'K')
		{
			per_user_limit = per_user_limit * 1000;
			break;
		}
		if (quota[i] == 'm' || quota[i] == 'M')
		{
			per_user_limit = per_user_limit * 1000000;
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
