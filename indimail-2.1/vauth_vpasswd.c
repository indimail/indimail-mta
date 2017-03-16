/*
 * $Log: vauth_vpasswd.c,v $
 * Revision 2.12  2008-11-10 11:56:34+05:30  Cprogrammer
 * conditional compilation of QUERY_CACHE code
 *
 * Revision 2.11  2008-11-07 17:02:37+05:30  Cprogrammer
 * replaced flushpw() with vauth_getpw_cache()
 *
 * Revision 2.10  2008-09-08 09:56:34+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.9  2008-08-02 09:10:01+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.8  2008-05-28 16:40:00+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2008-05-28 15:24:57+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.6  2003-02-19 22:25:06+05:30  Cprogrammer
 * indimailuid,indimailgid was not set properly
 *
 * Revision 2.5  2003-01-05 00:17:18+05:30  Cprogrammer
 * error message correction
 *
 * Revision 2.4  2002-08-11 18:22:35+05:30  Cprogrammer
 * removed updation of passwd in hostcntrl
 *
 * Revision 2.3  2002-08-05 01:12:00+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.2  2002-08-03 04:37:07+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-07-03 01:20:24+05:30  Cprogrammer
 * copy passwd structure to prevent overwriting static location returned by vauth_getpw().
 *
 * Revision 1.16  2002-04-04 16:43:29+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.15  2002-04-03 01:43:51+05:30  Cprogrammer
 * use uid/gid from assign file
 *
 * Revision 1.14  2002-04-01 02:11:33+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.13  2002-03-31 21:52:07+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.12  2002-03-27 01:53:46+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.11  2002-03-25 00:37:44+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.10  2002-03-24 19:15:03+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.9  2002-03-03 15:42:20+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.8  2001-12-22 18:14:32+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.7  2001-12-11 12:13:13+05:30  Cprogrammer
 * added open master for updates
 *
 * Revision 1.6  2001-11-28 23:08:08+05:30  Cprogrammer
 * code change for distributed architecture
 *
 * Revision 1.5  2001-11-24 20:26:56+05:30  Cprogrammer
 * call flushpw to force vauth_getpw to get the pw structure from mysql
 *
 * Revision 1.4  2001-11-24 12:21:05+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:10:43+05:30  Cprogrammer
 * added code for updating inactive table
 *
 * Revision 1.2  2001-11-14 19:27:23+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_vpasswd.c,v 2.12 2008-11-10 11:56:34+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
vauth_vpasswd(char *user, char *domain, char *pass, int apop)
{
	char           *tmpstr;
	int             err;
	uid_t           myuid, uid;
	char            SqlBuf[SQL_BUF_SIZE];

	if(indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	if(!vget_assign(domain, 0, 0, &uid, 0))
		uid = indimailuid;
	myuid = geteuid();
	if (myuid != indimailuid && myuid != 0)
	{
		if(uid == indimailuid)
			error_stack(stderr, "id should be %d or root\n", indimailuid);
		else
			error_stack(stderr, "id should be %d, %d or root\n", indimailuid, uid);
		return (-1);
	}
	if (vauth_open((char *) 0))
		return (-1);
	if (site_size == LARGE_SITE)
	{
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority %s set pw_passwd = \"%s\" where pw_name = \"%s\"", 
				tmpstr, pass, user);
	} else
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority %s set pw_passwd = \"%s\" where pw_name = \"%s\" and pw_domain = \"%s\"", 
			default_table, pass, user, domain);
	}
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_vpasswd: %s", SqlBuf);
		return (-1);
	}
	err = mysql_affected_rows(&mysql[1]);
	if(!err && site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority %s set pw_passwd = \"%s\" where pw_name = \"%s\" and pw_domain = \"%s\"", 
			inactive_table, pass, user, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_vpasswd: %s", SqlBuf);
			return (-1);
		}
		err = mysql_affected_rows(&mysql[1]);
	}
#ifdef QUERY_CACHE
	if(err == 1)
		vauth_getpw_cache(0);
#endif
	return (1);
}

void
getversion_vauth_vpasswd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
