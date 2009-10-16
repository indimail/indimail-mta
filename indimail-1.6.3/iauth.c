/*
 * $Log: iauth.c,v $
 * Revision 2.5  2009-10-14 20:43:07+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.4  2009-10-11 09:36:16+05:30  Cprogrammer
 * renamed authenticate to iauth
 * completed acct_mgmt function
 *
 * Revision 2.3  2009-10-11 09:11:14+05:30  Cprogrammer
 * completed acct_mgmt code
 *
 * Revision 2.2  2009-10-08 14:39:41+05:30  Cprogrammer
 * check pw_gid bit field only when service is not null
 *
 * Revision 2.1  2009-10-08 11:17:36+05:30  Cprogrammer
 * moved authenticate from pam-multi to indimail
 *
 * Revision 1.1  2008-09-01 15:16:05+05:30  Cprogrammer
 * Initial revision
 *
 *
 * authenticate.c - Generic PAM Authentication module for pam_multi
 * Copyright (C) <2008>  Manvendra Bhangui <mbhangui@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The GNU General Public License does not permit incorporating your program
 * into proprietary programs.  If your program is a subroutine library, you
 * may consider it more useful to permit linking proprietary applications with
 * the library.  If this is what you want to do, use the GNU Lesser General
 * Public License instead of this License.  But first, please read
 * <http://www.gnu.org/philosophy/why-not-lgpl.html>.
 *
 * Testing
 * pamtester imap postmaster@indimail.org authenticate
 *
 */
#include "indimail.h"
#undef PACKAGE
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#ifdef ENABLE_AUTH_LOGGING
#include <mysqld_error.h>
#endif

static int      defaultTask(char *, char *, struct passwd *, char *);

#ifndef lint
static char     sccsid[] = "$Id: iauth.c,v 2.5 2009-10-14 20:43:07+05:30 Cprogrammer Exp mbhangui $";
#endif
/*
#define iauth ltdl_module_LTX_iauth
-*/

#ifndef INDIMAIL
void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = getenv(envname)))
		*source = defaultValue;
	return;
}
#endif

void
close_connection()
{
#ifdef PASSWD_CACHE
	if (!getenv("PASSWD_CACHE"))
		vclose();
#else /*- Not PASSWD_CACHE */
	vclose();
#endif
}

struct passwd  *_global_pw;

static char *
i_auth(char *email, char *service, int *size, int debug)
{
	char           *ptr, *cptr, *real_domain, *crypt_pass;
	char            User[AUTH_SIZE], Domain[AUTH_SIZE];
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;

	_global_pw = (struct passwd *) 0;
	*size = 0;
	for (ptr = email, cptr = User;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = Domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	/*- crypt("pass", "kk"); -*/
	if (debug)
		fprintf(stderr, "authenticate.so: opening MySQL connection\n");
#ifdef PASSWD_CACHE
	if (!getenv("PASSWD_CACHE"))
	{
#ifdef CLUSTERED_SITE
		if (vauthOpen_user(email))
#else
		if (vauth_open((char *) 0))
#endif
			return ((char *) 0);
	}
#else
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email))
#else
	if (vauth_open((char *) 0))
#endif
		return ((char *) 0);
#endif
#ifdef PASSWD_CACHE
	if (getenv("PASSWD_CACHE"))
	{
		if (debug)
			fprintf(stderr, "authenticate.so: doing inquery\n");
		pw = inquery(PWD_QUERY, email, 0);
	} else
	{
		if (debug)
			fprintf(stderr, "authenticate.so: doing vauth_getpw\n");
		if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
		{
			fprintf(stderr, "iauth: domain %s does not exist\n", Domain);
			return ((char *) 0);
		}
		if (!(real_domain = vget_real_domain(Domain)))
			real_domain = Domain;
		pw = vauth_getpw(User, real_domain);
	}
#else
	if (debug)
		fprintf(stderr, "authenticate.so: doing vauth_getpw\n");
	if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
	{
		fprintf(stderr, "i_auth: domain %s does not exist\n", Domain);
		return ((char *) 0);
	}
	if (!(real_domain = vget_real_domain(Domain)))
		real_domain = Domain;
	pw = vauth_getpw(User, real_domain);
#endif
	if (!pw)
	{
		if(userNotFound)
			return ((char *) 0);
		else
			fprintf(stderr, "i_auth: inquery: %s\n", strerror(errno));
		close_connection();
		return ((char *) 0);
	}
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG_LOGIN"))
	{
		fprintf(stderr, "i_auth: service[%s] email [%s] pw_passwd [%s]\n", 
			service, email, crypt_pass);
	}
	close_connection();
	_global_pw = pw;
	*size = strlen(crypt_pass) + 1;
	if (debug)
		fprintf(stderr, "authenticate.so: returning data of size %d\n", *size);
	return(crypt_pass);
}

#ifdef ENABLE_AUTH_LOGGING
#define NO_OF_ITEMS 2
char           *
i_acctmgmt(char *email, char *service, int *size, int *nitems, int debug)
{
	char           *ptr, *cptr, *real_domain, *crypt_pass;
	char            User[AUTH_SIZE], Domain[AUTH_SIZE];
	char            SqlBuf[SQL_BUF_SIZE];
	int             i;
	static long     exp_times[NO_OF_ITEMS];
	time_t          tmval;
	uid_t           uid;
	gid_t           gid;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	*nitems = NO_OF_ITEMS;
	*size = 0;
	for (ptr = email, cptr = User;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = Domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (debug)
		fprintf(stderr, "authenticate.so: opening MySQL connection\n");
	if (vauth_open((char *) 0))
		return ((char *) 0);
	if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
	{
		fprintf(stderr, "i_acctmgmt: domain %s does not exist\n", Domain);
		return ((char *) 0);
	}
	if (!(real_domain = vget_real_domain(Domain)))
		real_domain = Domain;
	for (i = 0;i < NO_OF_ITEMS;i++)
	{
		switch (i)
		{
		case 0:
			snprintf(SqlBuf, SQL_BUF_SIZE, 
				"select high_priority UNIX_TIMESTAMP(timestamp) from lastauth where user=\"%s\" and domain=\"%s\" \
				and (service = \"pop3\" or service=\"imap\" or service=\"webm\")", User, Domain);
			break;
		case 1:
			snprintf(SqlBuf, SQL_BUF_SIZE, 
				"select high_priority UNIX_TIMESTAMP(timestamp) from lastauth where user=\"%s\" and domain=\"%s\" \
				and service=\"pass\"", User, Domain);
			break;
		}
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
				exp_times[i] = 0;
				if (debug)
					fprintf(stderr, "authenticate.so: expiry[%d] = %ld\n", i, exp_times[i]);
				continue;
			}
			mysql_perror("i_acctmgmt: mysql_query: %s", SqlBuf);
			return ((char *) 0);
		}
		res = mysql_store_result(&mysql[1]);
		exp_times[i] = 0;
		while ((row = mysql_fetch_row(res)))
		{
			tmval = atol(row[0]);
			if (tmval > exp_times[i])
				exp_times[i] = tmval;
		}
		if (debug)
			fprintf(stderr, "authenticate.so: expiry[%d] = %ld\n", i, exp_times[i]);
		mysql_free_result(res);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (service && _global_pw)
	{
		if (strcmp("webmail", service) == 0)
		{
			if (_global_pw->pw_gid & NO_WEBMAIL)
			{
				fprintf(stderr, "i_acctmgmt: webmail disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		} else
		if (strcmp("pop3", service) == 0)
		{
			if (_global_pw->pw_gid & NO_POP)
			{
				fprintf(stderr, "i_acctmgmt: pop3 disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		} else
		if (strcmp("imap", service) == 0)
		{
			if (_global_pw->pw_gid & NO_IMAP)
			{
				fprintf(stderr, "i_acctmgmt: imap disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		}
		if (defaultTask(email, Domain, _global_pw, service))
		{
			close_connection();
			return ((char *) 0);
		}
	}
	*size = sizeof(long) * NO_OF_ITEMS;
	return ((char *) &exp_times[0]);
}
#else
char           *
i_acctmgmt(char *email, char *service, int *size, int *nitems, int debug)
{
	*nitems = 0;
	return ((char *) 0);
}
#endif

char *
iauth(char *email, char *service, int auth_or_accmgmt, int *size, int *nitems, int debug)
{
	if (!auth_or_accmgmt && nitems)
		*nitems = 1;
	return (auth_or_accmgmt ?  i_acctmgmt(email, service, size, nitems, debug) : i_auth(email, service, size, debug));
}

static int
defaultTask(char *userid, char *TheDomain, struct passwd *pw, char *service)
{
	char            Maildir[MAX_BUFF], authenv1[MAX_BUFF], authenv2[MAX_BUFF], authenv3[MAX_BUFF],
	                authenv4[MAX_BUFF], authenv5[MAX_BUFF], TheUser[MAX_BUFF], TmpBuf[MAX_BUFF];
	char           *ptr, *cptr;
	int             status;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif

	for (cptr = TheUser, ptr = userid;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	scopy(TmpBuf, service, MAX_BUFF);
	if ((ptr = strrchr(TmpBuf, ':')))
		*ptr = 0;
	if (Check_Login(TmpBuf, TheDomain, pw->pw_gecos))
	{
		fprintf(stderr, "Login not permitted for %s\n", TmpBuf);
		return (1);
	}
	status = Login_Tasks(pw, userid, TmpBuf);
	if (status == 2 && !strncasecmp(service, "imap", 4))
		return(1);
	snprintf(Maildir, MAX_BUFF, "%s/Maildir", status == 2 ? "/mail/tmp" : pw->pw_dir);
	if (access(pw->pw_dir, F_OK) || access(Maildir, F_OK) || chdir(pw->pw_dir))
	{
		fprintf(stderr, "chdir: %s: %s\n", pw->pw_dir, strerror(errno));
		return(1);
	}
	snprintf(authenv1, MAX_BUFF, "AUTHENTICATED=%s", userid);
	snprintf(authenv2, MAX_BUFF, "AUTHADDR=%s@%s", TheUser, TheDomain);
	snprintf(authenv3, MAX_BUFF, "AUTHFULLNAME=%s", pw->pw_gecos);
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
		return (1);
	}
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%"PRIu64"S,%"PRIu64"C", size_limit, count_limit);
#else
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%sS", pw->pw_shell);
#endif
	snprintf(authenv5, MAX_BUFF, "MAILDIR=%s", Maildir);
	putenv(authenv1);
	putenv(authenv2);
	putenv(authenv3);
	putenv(authenv4);
	putenv(authenv5);
	return(0);
}

void
getversion_iauth_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
