/*
 * $Log: vauth_setquota.c,v $
 * Revision 2.13  2010-03-02 08:18:20+05:30  Cprogrammer
 * changed Username xxx@yyy does not exist to xxx@yyy: No such user
 *
 * Revision 2.12  2009-10-14 20:46:26+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.11  2008-11-10 11:56:23+05:30  Cprogrammer
 * conditional compilation of QUERY_CACHE code
 *
 * Revision 2.10  2008-11-07 17:02:23+05:30  Cprogrammer
 * replaced flushpw() with vauth_getpw_cache()
 *
 * Revision 2.9  2008-09-08 09:56:17+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.8  2008-08-02 09:09:56+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.7  2008-05-28 16:39:53+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.6  2008-05-28 15:24:44+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.5  2003-10-26 00:11:20+05:30  Cprogrammer
 * Preserve mail count limit in quota field when changing quota
 *
 * Revision 2.4  2002-12-02 02:28:41+05:30  Cprogrammer
 * allow quota to be specified in terms of increment/decrement
 *
 * Revision 2.3  2002-08-05 01:09:29+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.2  2002-08-03 04:36:10+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-07-03 01:19:41+05:30  Cprogrammer
 * copy passwd structure to prevent overwriting static location returned by vauth_getpw()
 *
 * Revision 1.13  2002-04-04 16:43:09+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.12  2002-04-01 02:11:31+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.11  2002-03-31 21:52:04+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.10  2002-03-27 01:53:43+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.9  2002-03-25 00:37:37+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.8  2002-03-24 19:14:55+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.7  2002-03-03 15:42:13+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.6  2001-12-19 20:39:45+05:30  Cprogrammer
 * update only if quota is different from the existing quota
 *
 * Revision 1.5  2001-11-24 20:26:38+05:30  Cprogrammer
 * call flushpw to force vauth_getpw to get the pw structure from mysql
 *
 * Revision 1.4  2001-11-24 12:21:04+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:59:15+05:30  Cprogrammer
 * added code for updating the inactive table
 *
 * Revision 1.2  2001-11-14 19:27:17+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_setquota.c,v 2.13 2010-03-02 08:18:20+05:30 Cprogrammer Stab mbhangui $";
#endif

#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
vauth_setquota(char *user, char *domain, char *quota)
{
	char           *tmpstr;
	int             no_of_rows, err;
	char            SqlBuf[SQL_BUF_SIZE], tmpQuota[MAX_BUFF];
	struct passwd  *pw;

	if (vauth_open((char *) 0))
		return(-1);
	if((pw = vauth_getpw(user, domain)))
	{
		if(!strcmp(pw->pw_shell, quota))
			return(1);
	}  else
	{
		error_stack(stderr, "%s@%s: No such user\n", user, domain);
		return(0);
	}
	if((*quota == '+') || (*quota == '-'))
		snprintf(tmpQuota, sizeof(tmpQuota), "%lld", strtoll(pw->pw_shell, 0, 0) + atoi(quota));
	else
		scopy(tmpQuota, quota, MAX_BUFF);
	if (site_size == LARGE_SITE)
	{
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority %s set pw_shell = \"%s\" where pw_name = \"%s\"", 
			tmpstr, tmpQuota, user);
	} else
		snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority %s set pw_shell = \"%s\" where pw_name = \"%s\" and \
			pw_domain = \"%s\"", default_table, tmpQuota, user, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_setquota: %s", SqlBuf);
		return(-1);
	}
	no_of_rows = mysql_affected_rows(&mysql[1]);
	if(!no_of_rows && site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority %s set pw_shell = \"%s\" where pw_name = \"%s\" and \
			pw_domain = \"%s\"", inactive_table, tmpQuota, user, domain);
		err = 0;
		if (mysql_query(&mysql[1], SqlBuf) && (err = mysql_errno(&mysql[1])) != ER_NO_SUCH_TABLE)
		{
			mysql_perror("vauth_setquota: %s", SqlBuf);
			return(-1);
		}
		if(err == ER_NO_SUCH_TABLE)
			no_of_rows = 0;
		else
			no_of_rows = mysql_affected_rows(&mysql[1]);
	}
	if(no_of_rows == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
#ifdef QUERY_CACHE
	else
	if(no_of_rows == 1)
		vauth_getpw_cache(0);
#endif
	return (no_of_rows);
}

void
getversion_vauth_setquota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
