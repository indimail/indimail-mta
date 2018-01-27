/*
 * $Log: setuserquota.c,v $
 * Revision 2.7  2009-10-14 20:45:36+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.6  2009-09-25 23:50:32+05:30  Cprogrammer
 * return status of recalc_quota()
 *
 * Revision 2.5  2008-08-02 09:08:40+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.4  2008-06-24 21:54:59+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.3  2005-12-29 22:49:29+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2002-05-20 15:53:31+05:30  Cprogrammer
 * removed unecessary debug statement
 *
 * Revision 2.1  2002-05-15 00:05:41+05:30  Cprogrammer
 * update maildirsize for .current_size if quota is changed
 *
 * Revision 1.5  2001-12-19 20:38:58+05:30  Cprogrammer
 * removed vauth_getpw as vauth_getpw is being done in vauth_setquota
 *
 * Revision 1.4  2001-11-24 20:24:33+05:30  Cprogrammer
 * if update in vauth_getpw returns 0 rows, call vauth_getpw to determine if user exists
 *
 * Revision 1.3  2001-11-24 12:20:03+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:59+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: setuserquota.c,v 2.7 2009-10-14 20:45:36+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Update a users quota
 */
int
vsetuserquota(char *username, char *domain, char *quota)
{
	char           *domain_ptr;
	char            tmpbuf[MAX_BUFF];
	int             i;
	struct passwd  *pw;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit, mailcount;
#endif

	if (!username || !*username)
	{
		error_stack(stderr, "Illegal Username\n");
		return(-1);
	}
	lowerit(username);
	if(domain && *domain)
		domain_ptr = domain;
	else
		getEnvConfigStr(&domain_ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	lowerit(domain_ptr);
	if((i = vauth_setquota(username, domain_ptr, quota)) == -1)
	{
		error_stack(stderr, "vauth_setquota: Failed to update database\n");
		return(-1);
	} else
	if(!i)
		return(-1);
	if(!(pw = vauth_getpw(username, domain_ptr)))
	{
		fprintf(stderr, "no such user %s@%s\n", username, domain_ptr);
		return(1);
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/Maildir", pw->pw_dir);
#ifdef USE_MAILDIRQUOTA
	if ((size_limit = parse_quota(quota, &count_limit)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", quota, strerror(errno));
		return (1);
	}
	return (recalc_quota(tmpbuf, &mailcount, size_limit, count_limit, 2));
#else
	return (recalc_quota(tmpbuf, 2))
#endif
}

void
getversion_setuserquota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
