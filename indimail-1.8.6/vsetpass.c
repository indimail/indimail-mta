/*
 * $Log: vsetpass.c,v $
 * Revision 2.4  2011-10-28 14:16:47+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.3  2011-10-25 20:50:01+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.2  2010-05-01 14:15:21+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.1  2009-08-05 14:35:38+05:30  Cprogrammer
 * setpassword interface for indimail
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <errno.h>

#ifndef lint
static char     sccsid[] = "$Id: vsetpass.c,v 2.4 2011-10-28 14:16:47+05:30 Cprogrammer Stab mbhangui $";
#endif
#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;

int
main(int argc, char **argv)
{
	char           *tmpbuf, *login, *new_pass, *old_pass, *response, *crypt_pass, *ptr, *cptr;
	char            Dir[MAX_BUFF], Crypted[MAX_BUFF], user[AUTH_SIZE], domain[AUTH_SIZE];
	int             count, offset, i;
	mdir_t          quota;
	struct passwd  *pw;

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
	old_pass = tmpbuf + count; /*- challenge (or plain text) */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	response = tmpbuf + count; /*- response */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	new_pass = tmpbuf + count; /*- new password */
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
	{
		if (vauth_open((char *) 0))
		{
			if (userNotFound)
				pipe_exec(argv, tmpbuf, offset);
			else
				fprintf(stderr, "vauth_open: %s\n", strerror(errno));
			printf("454-failed to connect to database (%s) (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
	}
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
#endif
	for (cptr = user, ptr = login;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
#ifdef QUERY_CACHE
	if (getenv("QUERY_CACHE"))
		pw = inquery(PWD_QUERY, login, 0);
	else
		pw = vauth_getpw(user, domain);
#else
	pw = vauth_getpw(user, domain);
#endif
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
		vclose();
#else /*- Not QUERY_CACHE */
	vclose();
#endif
	if (!pw)
	{
		if (userNotFound)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "vsetpass: inquery: %s\n", strerror(errno));
		printf("454 %s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	} 
	if (pw->pw_gid & NO_PASSWD_CHNG)
	{
		printf("553 Sorry, this account cannot change password (#5.7.1)\r\n");
		fflush(stdout);
		_exit (1);
	} else
	if (is_inactive && !getenv("ALLOW_INACTIVE"))
	{
		printf("553 Sorry, this account has expired (#5.7.1)\r\n");
		fflush(stdout);
		_exit (1);
	}
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG"))
	{
		fprintf(stderr, "%s: login [%s] old_pass [%s] new_pass[%s] response [%s] pw_passwd [%s]\n", 
			argv[0], login, old_pass, new_pass, response, crypt_pass);
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) (*response ? old_pass : 0),
		(unsigned char *) (*response ? response : old_pass), 0))
	{
		pipe_exec(argv, tmpbuf, offset);
		printf("454 %s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	mkpasswd3(new_pass, Crypted, MAX_BUFF);
	if (getenv("DEBUG"))
		fprintf(stderr, "%s: login [%s] old_pass [%s] new_pass [%s] response [%s] pw_passwd [%s]\n", 
			argv[0], login, old_pass, new_pass, response, Crypted);
	if ((i = vauth_vpasswd(user, domain, Crypted, 0)) == 1)
	{
		snprintf(Dir, MAX_BUFF, "%s/Maildir", pw->pw_dir);
		if(access(Dir, F_OK))
			quota = 0l;
		else
		{
#ifdef USE_MAILDIRQUOTA
			quota = check_quota(Dir, 0);
#else
			quota = check_quota(Dir);
#endif
		}
#ifdef ENABLE_AUTH_LOGGING
		vset_lastauth(user, domain, "pass", GetIpaddr(), pw->pw_gecos, quota);
#endif
	}
	return(i == 1 ? 0 : 1);
}

void
getversion_vsetpass_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
