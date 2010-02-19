/*
 * $Log: vauth_setpw.c,v $
 * Revision 2.9  2009-01-28 13:47:34+05:30  Cprogrammer
 * BUG indimailuid, indimailgid was not initialized
 *
 * Revision 2.8  2008-11-10 11:56:54+05:30  Cprogrammer
 * conditional compilation of QUERY_CACHE code
 *
 * Revision 2.7  2008-11-07 10:06:37+05:30  Cprogrammer
 * replaced flushpw() with vauth_getpw_cache()
 *
 * Revision 2.6  2008-08-02 09:09:53+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.5  2008-05-28 16:39:51+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2008-05-28 15:24:31+05:30  Cprogrammer
 * removed ldap, cdb and sqwebmail
 *
 * Revision 2.3  2002-07-03 01:18:51+05:30  Cprogrammer
 * use copyPwdStruct() to avoid comparing the same static location returned by vauth_getpw()
 *
 * Revision 2.2  2002-06-26 03:20:58+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 2.1  2002-05-04 22:45:44+05:30  Cprogrammer
 * create indibak if table does not exist
 *
 * Revision 1.16  2002-04-04 16:42:47+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.15  2002-04-03 01:43:35+05:30  Cprogrammer
 * use uid/gid from assignfile
 *
 * Revision 1.14  2002-04-01 02:11:26+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.13  2002-03-31 21:51:59+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.12  2002-03-27 01:53:39+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.11  2002-03-25 00:37:26+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.10  2002-03-24 19:14:45+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.9  2002-03-03 15:42:10+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.8  2001-12-19 20:39:27+05:30  Cprogrammer
 * update only if passwd structure is different from the existing one
 *
 * Revision 1.7  2001-11-24 20:25:33+05:30  Cprogrammer
 * call flushpw to force vauth_getpw to get the pw structure from mysql
 *
 * Revision 1.6  2001-11-24 12:21:03+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.5  2001-11-24 01:30:13+05:30  Cprogrammer
 * new error message for 0 rows updated
 *
 * Revision 1.4  2001-11-20 21:48:41+05:30  Cprogrammer
 * added variable is_inactive to indicate whether authentication succeeded through the inactive table
 *
 * Revision 1.3  2001-11-20 10:58:37+05:30  Cprogrammer
 * added code for updating the inactive table
 *
 * Revision 1.2  2001-11-14 19:27:11+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_setpw.c,v 2.9 2009-01-28 13:47:34+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <unistd.h>
#include <mysqld_error.h>
int
vauth_setpw(struct passwd *inpw, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *tmpstr;
	uid_t           myuid;
	uid_t           uid;
	gid_t           gid;
	int             err;

	if(indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	if(!vget_assign(domain, 0, 0, &uid, &gid))
	{
		uid = indimailuid;
		gid = indimailgid;
	} 
	myuid = geteuid();
	if (myuid != indimailuid && myuid != uid && myuid != 0)
	{
		if(uid == indimailuid)
			error_stack(stderr, "id should be %d or root\n", indimailuid);
		else
			error_stack(stderr, "id should be %d, %d or root\n", indimailuid, uid);
		return (-1);
	}
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	if(!pwcomp(vauth_getpw(inpw->pw_name, domain), copyPwdStruct(inpw)))
		return(0);
	if (site_size == LARGE_SITE)
	{
		if (!domain || *domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, LARGE_SETPW, tmpstr, inpw->pw_passwd, (int) inpw->pw_uid,
			(int) inpw->pw_gid, inpw->pw_gecos, inpw->pw_dir, inpw->pw_shell, inpw->pw_name);
	} else
		snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_SETPW, default_table, inpw->pw_passwd, (int) inpw->pw_uid,
			(int) inpw->pw_gid, inpw->pw_gecos, inpw->pw_dir, inpw->pw_shell, inpw->pw_name, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_setpw: %s", SqlBuf);
		return (-1);
	}
	err = mysql_affected_rows(&mysql[1]);
	if(!err && site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_SETPW, inactive_table, inpw->pw_passwd, (int) inpw->pw_uid, 
			(int) inpw->pw_gid, inpw->pw_gecos, inpw->pw_dir, inpw->pw_shell, inpw->pw_name, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE IF NOT EXISTS %s ( %s )", 
					inactive_table, SMALL_TABLE_LAYOUT);
				if (mysql_query(&mysql[1], SqlBuf))
				{
					mysql_perror("vauth_setpw: %s", SqlBuf);
					return (-1);
				}
				err = 0;
			} else
			{
				mysql_perror("vauth_setpw: %s", SqlBuf);
				return (-1);
			}
		} else
			err = mysql_affected_rows(&mysql[1]);
	}
	if(!err)
		error_stack(stderr, "0 rows updated\n");
	if(!err || err == -1)
		err = 1;
	else
	{
#ifdef QUERY_CACHE
		vauth_getpw_cache(0);
#endif
		err = 0;
	}
	return (err);
}

void
getversion_vauth_setpw_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
