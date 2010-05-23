/*
 * $Log: vauth_deluser.c,v $
 * Revision 2.8  2008-09-08 09:55:44+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.7  2008-05-28 16:38:57+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.6  2008-05-28 15:23:28+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.5  2004-05-17 00:52:11+05:30  Cprogrammer
 * force flag argument to delusercntrl()
 *
 * Revision 2.4  2003-06-18 23:08:18+05:30  Cprogrammer
 * moved deletion of lastauth to vdeluser()
 *
 * Revision 2.3  2002-08-05 01:07:47+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.2  2002-08-03 04:34:55+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-06-26 03:19:44+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 1.14  2002-04-04 16:41:56+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.13  2002-04-01 02:11:19+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.12  2002-03-31 21:51:47+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.11  2002-03-27 01:53:31+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.10  2002-03-25 00:37:10+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.9  2002-03-24 19:14:18+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.8  2002-03-03 15:42:04+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.7  2001-12-27 20:46:05+05:30  Cprogrammer
 * registration information not to be deleted
 *
 * Revision 1.6  2001-12-08 00:39:59+05:30  Cprogrammer
 * vauth_deluser to remove all entries for the user being deleted
 *
 * Revision 1.5  2001-11-28 23:07:43+05:30  Cprogrammer
 * code change for distributed architecture
 *
 * Revision 1.4  2001-11-24 12:20:55+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:57:32+05:30  Cprogrammer
 * added code for deleting user from inactive table
 *
 * Revision 1.2  2001-11-14 19:26:50+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_deluser.c,v 2.8 2008-09-08 09:55:44+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>

int
vauth_deluser(char *user, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *tmpstr;
	int             err;

	if ((err = vauth_open((char *) 0)) != 0)
		return (1);
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n", domain);
		return (1);
	} else
	if (err == 1)
	{
		if (vauth_updateflag(user, domain, DEL_FLAG))
			return (1);
		if (delusercntrl(user, domain, 0))
			return (1);
	}
#endif
	if (site_size == LARGE_SITE)
	{
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_name = \"%s\"", tmpstr, user);
	} else
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_name = \"%s\" and pw_domain = \"%s\"", 
				default_table, user, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_deluser: %s", SqlBuf);
		return (1);
	} 
	err = mysql_affected_rows(&mysql[1]);
	if (!err && site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_name = \"%s\" and pw_domain = \"%s\"", 
				inactive_table, user, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_deluser: %s", SqlBuf);
			return (1);
		} 
		err = mysql_affected_rows(&mysql[1]);
	}
	if (!err || err == -1)
		err = 1;
	else
		err = 0;
	return (err);
}

void
getversion_vauth_deluser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
