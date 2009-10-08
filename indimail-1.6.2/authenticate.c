/*
 * $Log: authenticate.c,v $
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

static int      defaultTask(char *, char *, struct passwd *, char *);

#ifndef lint
static char     sccsid[] = "$Id: authenticate.c,v 2.2 2009-10-08 14:39:41+05:30 Cprogrammer Exp mbhangui $";
#endif
/*
#define vauthenticate ltdl_module_LTX_vauthenticate
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

char *
vauthenticate(char *email, char *service)
{
	char           *ptr, *cptr, *real_domain, *crypt_pass;
	char            User[AUTH_SIZE], Domain[AUTH_SIZE];
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;

	for (ptr = email, cptr = User;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = Domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
	{
		fprintf(stderr, "vauthenticate: domain %s does not exist\n", Domain);
		return ((char *) 0);
	}
	if (!(real_domain = vget_real_domain(Domain)))
		real_domain = Domain;
	/*- crypt("pass", "kk"); -*/
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
		pw = inquery(PWD_QUERY, email, 0);
	else
		pw = vauth_getpw(User, real_domain);
#else
	pw = vauth_getpw(User, real_domain);
#endif
	if (!pw)
	{
		if(userNotFound)
			return ((char *) 0);
		else
			fprintf(stderr, "vauthenticate: inquery: %s\n", strerror(errno));
		close_connection();
		return ((char *) 0);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (service)
	{
		if (strcmp("webmail", service) == 0)
		{
			if (pw->pw_gid & NO_WEBMAIL)
			{
				fprintf(stderr, "vauthenticate: webmail disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		} else
		if (strcmp("pop3", service) == 0)
		{
			if (pw->pw_gid & NO_POP)
			{
				fprintf(stderr, "vauthenticate: pop3 disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		} else
		if (strcmp("imap", service) == 0)
		{
			if (pw->pw_gid & NO_IMAP)
			{
				fprintf(stderr, "vauthenticate: imap disabled for this account");
				close_connection();
				return ((char *) 0);
			}
		}
	}
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG_LOGIN"))
	{
		fprintf(stderr, "vauthenticate: service[%s] email [%s] pw_passwd [%s]\n", 
			service, email, crypt_pass);
	}
	if (defaultTask(email, Domain, pw, service))
	{
		close_connection();
		return ((char *) 0);
	}
	close_connection();
	return(crypt_pass);
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
	size_limit = parse_quota(pw->pw_shell, &count_limit);
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
getversion_vauthenticate_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
