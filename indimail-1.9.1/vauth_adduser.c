/*
 * $Log: vauth_adduser.c,v $
 * Revision 2.16  2016-01-28 16:12:01+05:30  Cprogrammer
 * null terminate quota
 *
 * Revision 2.15  2016-01-28 00:04:39+05:30  Cprogrammer
 * maildirquota specification for -q option to vadduser
 *
 * Revision 2.14  2012-04-22 13:59:23+05:30  Cprogrammer
 * use 64bit intiger for quota calculation
 *
 * Revision 2.13  2008-08-02 09:09:44+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.12  2008-06-13 10:36:23+05:30  Cprogrammer
 * fixed compiler warning if HARD_QUOTA not defined
 *
 * Revision 2.11  2008-05-28 16:38:49+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.10  2008-05-28 15:22:57+05:30  Cprogrammer
 * removed ldap, cdb modules
 *
 * Revision 2.9  2008-05-27 19:51:19+05:30  Cprogrammer
 * create indimail, indibak tables if not exists
 *
 * Revision 2.8  2006-01-26 00:55:08+05:30  Cprogrammer
 * fix for NOQUOTA
 *
 * Revision 2.7  2005-12-29 22:51:26+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.6  2004-09-21 23:44:13+05:30  Cprogrammer
 * add user as active if actFlag is set
 *
 * Revision 2.5  2003-10-26 11:31:31+05:30  Cprogrammer
 * initialization of quota variable corrected
 *
 * Revision 2.4  2002-10-18 01:18:30+05:30  Cprogrammer
 * use the array rfc_ids[] to check for mandatory RFC821 ids
 *
 * Revision 2.3  2002-08-13 10:18:02+05:30  Cprogrammer
 * made quota configurable through environmnent variable HARD_QUOTA
 *
 * Revision 2.2  2002-06-26 03:19:24+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 2.1  2002-06-21 20:44:59+05:30  Cprogrammer
 * add postmaster and abuse account to active table by default
 *
 * Revision 1.14  2002-04-04 16:41:35+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.13  2002-04-01 02:11:16+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.12  2002-03-31 21:51:39+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.11  2002-03-27 01:53:27+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.10  2002-03-25 00:37:02+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.9  2002-03-24 19:14:08+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.8  2002-03-03 15:42:00+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.7  2001-12-08 12:38:03+05:30  Cprogrammer
 * restructuring of code
 * removed call to vauth_adddomain()
 *
 * Revision 1.6  2001-12-05 23:14:18+05:30  Cprogrammer
 * removed redundant snprintf statements
 *
 * Revision 1.5  2001-11-28 23:02:31+05:30  Cprogrammer
 * code change for updating pw_uid field by using the apop argument passed to vauth_adduser()
 *
 * Revision 1.4  2001-11-24 12:20:52+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:49+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:26:40+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: vauth_adduser.c,v 2.16 2016-01-28 16:12:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <mysqld_error.h>

char           *
vauth_adduser(char *user, char *domain, char *pass, char *gecos, char *dir, char *Quota, int apop, int actFlag)
{
	static char     dirbuf[MAX_BUFF];
	char            quota[QUOTA_BUFLEN], dom_dir[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	char           *domstr, *ptr;
#ifdef HARD_QUOTA
	char           *hard_quota;
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;

	if (vauth_open((char *) 0))
		return ((char *) 0);
	if (Quota && *Quota) {
		strncpy(quota, Quota, QUOTA_BUFLEN - 1);
		quota[QUOTA_BUFLEN - 1] = 0;
	} else {
#ifdef HARD_QUOTA
		getEnvConfigStr(&hard_quota, "HARD_QUOTA", HARD_QUOTA);
		snprintf(quota, QUOTA_BUFLEN, "%s", hard_quota);
		quota[QUOTA_BUFLEN - 1] = 0;
#else
		scopy(quota, "NOQUOTA", QUOTA_BUFLEN);
#endif
	}
	domstr = (char *) 0;
	if (!domain || !*domain)
		snprintf(dom_dir, MAX_BUFF, "%s/users", INDIMAILDIR);
	else
	{
		if(!vget_assign(domain, dom_dir, MAX_BUFF, &uid, &gid))
		{
			error_stack(stderr, "Domain %s does not exist\n", domain);
			return((char *) 0);
		}
		ptr = get_Mplexdir(user, domain, 0, uid, gid);
		scopy(dom_dir, ptr, MAX_BUFF);
		if(ptr)
			free(ptr);
	}
	if (dir && *dir)
		snprintf(dirbuf, MAX_BUFF, "%s/%s/%s", dom_dir, dir, user);
	else
		snprintf(dirbuf, MAX_BUFF, "%s/%s", dom_dir, user);
	if (site_size == LARGE_SITE)
	{
		if(domain && *domain)
			domstr = vauth_munch_domain(domain);
		else
			domstr = MYSQL_LARGE_USERS_TABLE;
		snprintf(SqlBuf, SQL_BUF_SIZE, LARGE_INSERT, domstr, user, pass, apop, gecos, dirbuf, quota);
	} else
	{
		for(i = 0;rfc_ids[i];i++)
		{
			if(!strncmp(user, rfc_ids[i], slen(rfc_ids[i]) + 1))
				break;
		}
		if(rfc_ids[i] || actFlag)
			snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_INSERT, default_table, user, domain, pass, apop, gecos, dirbuf, quota);
		else
			snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_INSERT, inactive_table, user, domain, pass, apop, gecos, dirbuf, quota);
	}
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			for(i = 0;rfc_ids[i];i++)
			{
				if(!strncmp(user, rfc_ids[i], slen(rfc_ids[i]) + 1))
					break;
			}
			if (create_table(ON_LOCAL,
				(rfc_ids[i] || actFlag) ? default_table : inactive_table, site_size == LARGE_SITE ? LARGE_TABLE_LAYOUT : SMALL_TABLE_LAYOUT))
				return ((char *) 0);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("vauth_adduser: mysql_query: %s", SqlBuf);
				return ((char *) 0);
			}
		} else
		{
			mysql_perror("vauth_adduser: mysql_query: %s", SqlBuf);
			return ((char *) 0);
		}
	}
#ifdef ENABLE_AUTH_LOGGING
	ptr = GetIpaddr();
	vset_lastauth(user, domain, "add", ptr, gecos, 0);
#endif
	return (dirbuf);
}

void
getversion_vauth_adduser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
