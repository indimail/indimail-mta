/*
 * $Log: vauth_getpw.c,v $
 * Revision 2.30  2011-06-20 22:05:12+05:30  Cprogrammer
 * fix using password cache with PWSTRUCT when a different user is specified
 *
 * Revision 2.29  2008-11-07 10:06:30+05:30  Cprogrammer
 * removed flushpw
 *
 * Revision 2.28  2008-11-06 15:38:22+05:30  Cprogrammer
 * added cache reset option
 *
 * Revision 2.27  2008-09-08 09:55:50+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.26  2008-05-28 16:39:17+05:30  Cprogrammer
 * removed USE_MYSQL, ldap code
 *
 * Revision 2.25  2008-05-28 15:24:00+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.24  2008-05-21 15:51:37+05:30  Cprogrammer
 * fixed non-ldap use.
 *
 * Revision 2.23  2005-12-29 22:51:30+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.22  2004-01-01 14:51:21+05:30  Cprogrammer
 * added PASSWD_CACHE environment variable to turn on passwd caching
 *
 * Revision 2.21  2003-12-20 15:32:05+05:30  Cprogrammer
 * merged statements for 2 switch cases
 *
 * Revision 2.20  2003-10-25 00:46:22+05:30  Cprogrammer
 * added domain limits
 *
 * Revision 2.19  2003-10-23 13:22:50+05:30  Cprogrammer
 * changed fread() to read()
 *
 * Revision 2.18  2003-08-24 16:06:32+05:30  Cprogrammer
 * additional domain argument passed to LdapGetpw()
 * gecos modification from ldap on if LDAP_ACCESS_CHECK is defined and not = 2
 *
 * Revision 2.17  2003-05-30 00:00:26+05:30  Cprogrammer
 * added environment FORCE_MYSQL to bypass ldap passwd
 * to avoid vmoduser, vreorg, vset_lastdeliver wrongly
 * setting pw_gid field
 *
 * Revision 2.16  2003-02-01 14:12:47+05:30  Cprogrammer
 * set pw_gecos from ldap
 *
 * Revision 2.15  2003-01-28 23:28:18+05:30  Cprogrammer
 * set gecos based on mails attribute
 *
 * Revision 2.14  2003-01-03 02:14:53+05:30  Cprogrammer
 * include syntax error as a userNotFound
 *
 * Revision 2.13  2002-11-27 01:41:05+05:30  Cprogrammer
 * allow pop3 for attribute mails=3 as well
 *
 * Revision 2.12  2002-11-22 01:16:18+05:30  Cprogrammer
 * moved attr_flags within #ifdef USE_LDAP_PASSWD
 *
 * Revision 2.11  2002-08-25 22:35:17+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.10  2002-08-05 01:08:26+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.9  2002-07-23 22:44:04+05:30  Cprogrammer
 * added code to fallback to mysql in case of ldap problems
 *
 * Revision 2.8  2002-07-22 19:39:27+05:30  Cprogrammer
 * change due to LdapGetpw() change
 *
 * Revision 2.7  2002-07-15 02:06:48+05:30  Cprogrammer
 * restructured ldap code
 *
 * Revision 2.6  2002-07-05 11:34:48+05:30  Cprogrammer
 * use ldap also in case $QMAILDIR/control/host.ldap is present
 *
 * Revision 2.5  2002-07-02 12:20:54+05:30  Cprogrammer
 * added code to fetch encrypted passwd from LDAP server
 *
 * Revision 2.4  2002-06-26 03:20:06+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 * added function ldapflushpw()
 *
 * Revision 2.3  2002-06-21 20:39:16+05:30  Cprogrammer
 * use real domain when quering auth table
 *
 * Revision 2.2  2002-05-12 01:23:39+05:30  Cprogrammer
 * removed md5.h
 *
 * Revision 2.1  2002-04-10 23:56:21+05:30  Cprogrammer
 * call to mysql_free_result() to fix memory leak
 *
 * Revision 1.21  2002-04-08 15:00:29+05:30  Cprogrammer
 * replaced parsing of PWSTRUCT variable with function strToPw()
 *
 * Revision 1.20  2002-04-08 03:47:44+05:30  Cprogrammer
 * set is_overquota if user is overquota
 *
 * Revision 1.19  2002-04-07 13:43:06+05:30  Cprogrammer
 * return NULL intially if pwstruct is "No such user"
 *
 * Revision 1.18  2002-04-04 16:42:22+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.17  2002-04-01 02:11:22+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.16  2002-03-31 21:51:50+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.15  2002-03-27 01:53:35+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.14  2002-03-25 00:37:19+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.13  2002-03-24 19:20:35+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.12  2002-03-21 10:05:20+05:30  Cprogrammer
 * fixed problem with is_inactive getting permanently set for inactive users
 *
 * Revision 1.11  2002-03-18 19:04:38+05:30  Cprogrammer
 * set userNotFound when pwstruct is set as "No such user" by qmail-lspawn
 *
 * Revision 1.10  2002-03-17 20:45:03+05:30  Cprogrammer
 * added inactive_flag to PWSTRUCT
 *
 * Revision 1.9  2002-03-13 09:55:32+05:30  Cprogrammer
 * added code for mysql_query to be done in qmail-lspawn and result passed through
 * env variable PWSTRUCT
 *
 * Revision 1.8  2002-03-03 15:42:07+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.7  2001-12-05 23:14:44+05:30  Cprogrammer
 * if indibak does not exist set userNotFound = 1
 *
 * Revision 1.6  2001-11-24 20:28:40+05:30  Cprogrammer
 * make vauth_getpw faster by returning pw from static location if called for the same user and domain
 *
 * Revision 1.5  2001-11-24 12:20:57+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.4  2001-11-22 22:51:02+05:30  Cprogrammer
 * set is_inactive when authentication happens through inactive table
 *
 * Revision 1.3  2001-11-20 10:57:49+05:30  Cprogrammer
 * added code for checking the inactive table
 *
 * Revision 1.2  2001-11-14 19:27:00+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: vauth_getpw.c,v 2.30 2011-06-20 22:05:12+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char     __IUser[MAX_BUFF], __IDomain[MAX_BUFF];
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

struct passwd  *
vauth_getpw(char *user, char *domain)
{
	char           *in_domain, *domstr, *pwstruct, *real_domain, *ptr;
	int             mem_size, row_count, pass, err, dom_len;
	static struct passwd pwent;
	static char     IUser[MAX_BUFF], IPass[MAX_BUFF], IGecos[MAX_BUFF];
	static char     IDir[MAX_BUFF], IShell[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE], _user[MAX_BUFF], _domain[MAX_BUFF];
	MYSQL_RES      *res;
	MYSQL_ROW       row;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

	if (!domain || !*domain || !user || !*user)
		return ((struct passwd *) 0);
	scopy(_user, user, MAX_BUFF);
	for (ptr = domain, dom_len = 0;*ptr;ptr++, dom_len++);
	scopy(_domain, domain, dom_len + 1);
	lowerit(_user);
	lowerit(_domain);
	if (!pwent.pw_name) /*- first time */
	{
		pwent.pw_name = IUser;
		pwent.pw_passwd = IPass;
		pwent.pw_gecos = IGecos;
		pwent.pw_dir = IDir;
		pwent.pw_shell = IShell;
	}
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("PASSWD_CACHE"))
	{
		if (*__IUser && *__IDomain && !strncmp(__IUser, _user, MAX_BUFF) && !strncmp(__IDomain, _domain, MAX_BUFF))
			return (&pwent);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	is_inactive = userNotFound = is_overquota = 0;
	if ((pwstruct = (char *) getenv("PWSTRUCT")))
	{
		for (ptr = pwstruct;*ptr && *ptr != '@';ptr++);
		if (*ptr == '@')
		{
			*ptr = 0;
			if (!strncmp(_user, pwstruct, MAX_BUFF) && !strncmp(_domain, ptr + 1, dom_len))
			{
				*ptr = '@';
				return (strToPw(pwstruct, slen(pwstruct) + 1));
			}
			*ptr = '@';
		} else
			return (strToPw(pwstruct, slen(pwstruct) + 1));
	}
	if (vauth_open((char *) 0))
		return ((struct passwd *) 0);
	if (site_size == SMALL_SITE)
	{
		if (!(real_domain = vget_real_domain(_domain)))
			real_domain = _domain;
	} else
		real_domain = _domain;
	mem_size = slen(real_domain) + 1;
	in_domain = calloc(mem_size, sizeof(char));
	scopy(in_domain, real_domain, mem_size);
	domstr = (char *) 0;
	if (site_size == LARGE_SITE && (!in_domain || !*in_domain))
		domstr = MYSQL_LARGE_USERS_TABLE;
	else
	if (domain && *domain)
		domstr = vauth_munch_domain(in_domain);
	if (site_size == LARGE_SITE)
		snprintf(SqlBuf, SQL_BUF_SIZE, LARGE_SELECT, domstr, _user);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_SELECT, default_table, _user, in_domain);
	for (pass = 1;pass <= 2;pass++)
	{
		if (pass == 2)
			snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_SELECT, inactive_table, _user, in_domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			err = mysql_errno(&mysql[1]);
			if (err == ER_NO_SUCH_TABLE || err == ER_SYNTAX_ERROR)
				userNotFound = 1;
			else
				mysql_perror("vauth_getpw: %s", SqlBuf);
			free(in_domain);
			return ((struct passwd *) 0);
		}
		if (!(res = mysql_store_result(&mysql[1])))
		{
			mysql_perror("vauth_getpw: mysql_store_result:");
			free(in_domain);
			return ((struct passwd *) 0);
		}
		if ((row_count = mysql_num_rows(res)))
			break;
		else
			mysql_free_result(res);
	}
	free(in_domain);
	if (!row_count)
	{
		userNotFound = 1;
		return ((struct passwd *) 0);
	}
	if (pass == 2)
		is_inactive = 1;
	if ((row = mysql_fetch_row(res)))
	{
		scopy(pwent.pw_name, row[0], MAX_BUFF);
		scopy(pwent.pw_passwd, row[1], MAX_BUFF);
		pwent.pw_uid = atoi(row[2]);
		pwent.pw_gid = atoi(row[3]);
		if (pwent.pw_gid & BOUNCE_MAIL)
			is_overquota = 1;
		scopy(pwent.pw_gecos, row[4], MAX_BUFF);
		scopy(pwent.pw_dir, row[5], MAX_BUFF);
		scopy(pwent.pw_shell, row[6], MAX_BUFF);
		mysql_free_result(res);
		scopy(__IUser, _user, MAX_BUFF);
		scopy(__IDomain, _domain, MAX_BUFF);
#ifdef ENABLE_DOMAIN_LIMITS
		if (getenv("DOMAIN_LIMITS") && !(pwent.pw_gid & V_OVERRIDE))
		{
			if (!vget_limits(_domain, &limits))
				pwent.pw_gid |= vlimits_get_flag_mask(&limits);
			else
				return ((struct passwd *) 0);
		}
#endif
		return (&pwent);
	} 
	mysql_free_result(res);
	return ((struct passwd *) 0);
}

#ifdef QUERY_CACHE
void
vauth_getpw_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif

void
getversion_vauth_getpw_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
