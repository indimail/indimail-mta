/*
 * $Log: vchkpass.c,v $
 * Revision 2.39  2011-12-22 11:58:30+05:30  Cprogrammer
 * use domain from realm (fix for outlook)
 *
 * Revision 2.38  2011-12-18 20:41:19+05:30  Cprogrammer
 * use username%domain to fix authentication for brain-dead MS outlook
 *
 * Revision 2.37  2011-10-28 14:16:41+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.36  2011-10-25 20:49:56+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.35  2010-05-01 14:14:52+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.34  2010-04-12 08:48:22+05:30  Cprogrammer
 * use domain limits for account/password expiry
 * use inquery() for domain limits if QUERY_CACHE is defined
 *
 * Revision 2.33  2009-11-15 14:40:32+05:30  Cprogrammer
 * use vauthOpen_user() if QUERY_CACHE is not set for an extended domain
 *
 * Revision 2.32  2009-11-15 12:26:53+05:30  Cprogrammer
 * reorganized code for better readability
 *
 * Revision 2.31  2009-09-23 15:00:24+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.30  2009-04-17 21:00:09+05:30  Cprogrammer
 * log database error
 *
 * Revision 2.29  2009-04-17 14:31:28+05:30  Cprogrammer
 * BUG - Security bug with authentication of non-existent users
 *
 * Revision 2.28  2008-12-19 11:53:39+05:30  Cprogrammer
 * exit with return status of POSTAUTH command
 *
 * Revision 2.27  2008-12-18 11:55:26+05:30  Cprogrammer
 * changed PASSWD_CACHE to query cache
 *
 * Revision 2.26  2008-11-06 15:06:06+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 *
 * Revision 2.25  2008-08-28 21:40:59+05:30  Cprogrammer
 * removed is_open = 0
 *
 * Revision 2.24  2008-08-24 17:44:20+05:30  Cprogrammer
 * removed redundant code
 *
 * Revision 2.23  2008-07-13 19:49:35+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.22  2008-05-28 15:26:11+05:30  Cprogrammer
 * removed ldap module
 *
 * Revision 2.21  2005-12-29 22:51:56+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.20  2005-12-21 09:52:15+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.19  2005-07-07 09:00:14+05:30  Cprogrammer
 * allow inactive users to send mails if ALLOW_INACTIVE env variable is defined
 * allow users created only in ldap to send mails if ALLOW_INACTIVE is defined
 * execute script defined by POSTAUTH env variable after successful auth
 *
 * Revision 2.18  2005-01-22 00:42:30+05:30  Cprogrammer
 * case for NO_RELAY bit in pw_gid fixed
 *
 * Revision 2.17  2004-10-27 14:49:36+05:30  Cprogrammer
 * close mysql before exit
 *
 * Revision 2.16  2004-10-22 20:51:36+05:30  Cprogrammer
 * BUG - typo, changed USE_LDAP to USE_LDAP_PASSWD
 *
 * Revision 2.15  2004-07-12 22:48:40+05:30  Cprogrammer
 * made PASSWD_CACHE configurable through env variable
 *
 * Revision 2.14  2004-05-17 00:52:48+05:30  Cprogrammer
 * missing conditional compilation added
 *
 * Revision 2.13  2004-01-05 14:12:01+05:30  Cprogrammer
 * added ldap authentication code
 *
 * Revision 2.12  2002-12-11 10:29:09+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.11  2002-10-20 22:18:10+05:30  Cprogrammer
 * correction for compilation warning on solaris
 *
 * Revision 2.10  2002-09-01 20:54:01+05:30  Cprogrammer
 * added option to call next program on command line passing data on file descriptor 3
 *
 * Revision 2.9  2002-08-31 16:00:48+05:30  Cprogrammer
 * added more informative messages for system errors
 *
 * Revision 2.8  2002-08-31 14:27:43+05:30  Cprogrammer
 * corrected logic when passwd fails and next auth program needs to be exec'ed
 *
 * Revision 2.7  2002-08-30 23:29:04+05:30  Cprogrammer
 * incorrect size of buffers corrected
 *
 * Revision 2.6  2002-07-05 03:54:37+05:30  Cprogrammer
 * used sizeof operator for cross platform compatibility
 *
 * Revision 2.5  2002-05-12 01:20:35+05:30  Cprogrammer
 * added conditional compilation of debug message
 *
 * Revision 2.4  2002-05-11 00:19:18+05:30  Cprogrammer
 * Implemented CRAM-MD5 authentication
 *
 * Revision 2.3  2002-05-10 10:08:37+05:30  Cprogrammer
 * invoke alternate checkpassword program only if authentication fails
 *
 * Revision 2.2  2002-04-15 21:08:16+05:30  Cprogrammer
 * replaced vauth_getpw() with inquery()
 *
 * Revision 2.1  2002-04-15 20:14:04+05:30  Cprogrammer
 * prevent authentication if NO_RELAY is set
 *
 * Revision 1.1  2002-03-18 22:26:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef ENABLE_DOMAIN_LIMITS
#include <time.h>
#endif
#define _XOPEN_SOURCE
#include <unistd.h>
#include <errno.h>

#ifndef lint
static char     sccsid[] = "$Id: vchkpass.c,v 2.39 2011-12-22 11:58:30+05:30 Cprogrammer Stab mbhangui $";
#endif
#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;

int
main(int argc, char **argv)
{
	char           *tmpbuf, *login, *ologin, *response, *challenge, *crypt_pass, *ptr, *cptr;
	char            user[AUTH_SIZE], fquser[AUTH_SIZE], domain[AUTH_SIZE], buf[MAX_BUFF];
	int             count, offset, norelay = 0, status, auth_method;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if (argc < 2)
		_exit(2);
	if (!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char))))
	{
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		fprintf(stderr, "malloc-%d: %s\n", authlen + 1, strerror(errno));
		_exit(111);
	}
	for (offset = 0;;)
	{
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1)
		{
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			fprintf(stderr, "read: %s\n", strerror(errno));
			_exit(111);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = tmpbuf + count; /*- username */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	challenge = tmpbuf + count; /*- challenge (or plain text) */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	response = tmpbuf + count; /*- response (cram-md5, cram-sha1, etc) */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		auth_method = 0;
	else
		auth_method = tmpbuf[count + 1];

	for (cptr = user, ptr = login;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	ologin = login;
	if (*ptr)
	{
		ptr++;
		for (cptr = domain;*ptr;*cptr++ = *ptr++);
		*cptr = 0;
	} else /*- no @ in the login */
	{
		if (auth_method == AUTH_DIGEST_MD5) { /*- for handling dumb programs like
												outlook written by dumb programmers */
			for (cptr = buf, ptr = response;*ptr;*cptr++ = *ptr++) {
				if (*ptr == '\n') {
					*cptr = 0;
					ptr++;
					if (!strncmp("realm=", buf, 6)) {
						ptr = buf + 6;
						for (cptr = domain;*ptr;ptr++) {
							if (isspace(*ptr) || *ptr == '\"')
								continue;
							*cptr++ = *ptr;
						}
						*cptr = 0;
						break;
					} else
						cptr = buf;
				}
			}
			*cptr = 0;
			if (!strncmp("realm=", buf, 6)) {
				ptr = buf + 6;
				for (cptr = domain;*ptr && *ptr != ',';ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					*cptr++ = *ptr;
				}
				*cptr = 0;
			}
		}
		if (!*domain && parse_email(login, user, domain, AUTH_SIZE)) {
			getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
			for (cptr = domain;*ptr;*cptr++ = *ptr++);
			*cptr = 0;
		}
		snprintf(fquser, AUTH_SIZE, "%s@%s", user, domain);
		login = fquser;
	}
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
	{
#ifdef CLUSTERED_SITE
		if (vauthOpen_user(login, 0))
#else
		if (vauth_open((char *) 0))
#endif
		{
			if (userNotFound)
				pipe_exec(argv, tmpbuf, offset);
			else
				fprintf(stderr, "vauth_open: %s\n", strerror(errno));
			printf("454-failed to connect to database (%s) (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
		pw = vauth_getpw(user, domain);
		vclose();
	} else
		pw = inquery(PWD_QUERY, login, 0);
#else
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(login, 0))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "vauth_open: %s\n", strerror(errno));
		printf("454-failed to connect to database (%s) (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	pw = vauth_getpw(user, domain);
	vclose();
#endif
	if (!pw)
	{
		if (userNotFound)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "vchkpass: inquery: %s\n", strerror(errno));
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	} else
	if (pw->pw_gid & NO_SMTP)
	{
		printf("553-Sorry, this account cannot use SMTP (#5.7.1)\r\n");
		fflush(stdout);
		_exit (1);
	} else
	if (is_inactive && !getenv("ALLOW_INACTIVE"))
	{
		printf("553-Sorry, this account is inactive (#5.7.1)\r\n");
		fflush(stdout);
		_exit (1);
	} else
	if (pw->pw_gid & NO_RELAY)
		norelay = 1;
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG"))
	{
		fprintf(stderr, "%s: login [%s] challenge [%s] response [%s] pw_passwd [%s] method[%d]\n", 
			argv[0], login, challenge, response, crypt_pass, auth_method);
	}
	if (pw_comp((unsigned char *) ologin, (unsigned char *) crypt_pass,
		(unsigned char *) (*response ? challenge : 0),
		(unsigned char *) (*response ? response : challenge), auth_method))
	{
		pipe_exec(argv, tmpbuf, offset);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (getenv("DOMAIN_LIMITS"))
	{
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!getenv("QUERY_CACHE"))
		{
			if (vget_limits(domain, &limits))
			{
				fprintf(stderr, "vchkpass: unable to get domain limits for for %s\n", domain);
				printf("454-unable to get domain limits for %s\r\n", domain);
				fflush(stdout);
				_exit (111);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(domain, &limits))
		{
			fprintf(stderr, "vchkpass: unable to get domain limits for for %s\n", domain);
			printf("454-unable to get domain limits for %s\r\n", domain);
			fflush(stdout);
			_exit (111);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry)
		{
			printf("553-Sorry, your domain has expired (#5.7.1)\r\n");
			fflush(stdout);
			_exit (1);
		} else
		if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry)
		{
			printf("553-Sorry, your password has expired (#5.7.1)\r\n");
			fflush(stdout);
			_exit (1);
		} 
	}
#endif
	status = 0;
	if ((ptr = (char *) getenv("POSTAUTH")) && !access(ptr, X_OK))
	{
		snprintf(buf, MAX_BUFF, "%s %s", ptr, login);
		status = runcmmd(buf, 0);
	}
	_exit(norelay ? 3 : status);
	/*- Not reached */
	return(0);
}

void
getversion_vchkpass_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
