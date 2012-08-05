/*
 * $Log: systpass.c,v $
 * Revision 2.11  2011-10-28 14:16:33+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.10  2011-10-25 20:49:47+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.9  2009-09-23 15:00:17+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.8  2008-12-19 11:53:28+05:30  Cprogrammer
 * exit with return status of POSTAUTH command
 *
 * Revision 2.7  2008-07-13 19:47:57+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.6  2005-12-21 09:52:05+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.5  2005-07-06 23:51:27+05:30  Cprogrammer
 * added execution of postauth script post successful authentication
 *
 * Revision 2.4  2002-12-11 10:28:53+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.3  2002-10-20 22:17:18+05:30  Cprogrammer
 * corrected compilation warnings on solaris
 *
 * Revision 2.2  2002-09-11 20:38:01+05:30  Cprogrammer
 * strip of domain as login is a local user
 *
 * Revision 2.1  2002-09-01 20:51:54+05:30  Cprogrammer
 * smtp auth program for system passwords
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include "indimail.h"
#ifdef HAS_SHADOW
#include <shadow.h>
#endif

#ifndef lint
static char     sccsid[] = "$Id: systpass.c,v 2.11 2011-10-28 14:16:33+05:30 Cprogrammer Stab mbhangui $";
#endif

int             authlen = 512;

#ifdef ENABLE_PASSWD
int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge, *stored;
	char            buf[MAX_BUFF];
	int             count, offset, status;
	struct passwd  *pw;
#ifdef HAS_SHADOW
	struct spwd    *spw;
#endif

	if(argc < 2)
		_exit(2);
	if(!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char))))
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
		} while(count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while(count == -1 && errno == EINTR);
#endif
		if (count == -1)
		{
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			fprintf(stderr, "read: %s\n", strerror(errno));
			_exit(111);
		}
		else
		if(!count)
			break;
		offset += count;
		if(offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = tmpbuf + count; /*- username */
	for(;tmpbuf[count] && count < offset;count++);
	if(count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	challenge = tmpbuf + count; /*- challenge */
	for(;tmpbuf[count] && count < offset;count++);
	if(count == offset || (count + 1) == offset)
		_exit(2);
	response = tmpbuf + count + 1; /*- response */
	if((ptr = strchr(login, '@'))) /*- For stupid Netscape */
		*ptr = 0;
	if(!(pw = getpwnam(login)))
	{
		if(errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "syspass: getpwnam: %s\n", strerror(errno));
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	stored = pw->pw_passwd;
#ifdef HAS_SHADOW
	if(!(spw = getspnam(login)))
	{
		if(errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "syspass: getspnam: %s\n", strerror(errno));
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	stored = spw->sp_pwdp;
#endif
#ifdef DEBUG
	fprintf(stderr, "%s: login [%s] challenge [%s] response [%s] pw_passwd [%s]\n", 
		argv[0], login, challenge, response, stored);
#endif
	if (pw_comp((unsigned char *) login, (unsigned char *) stored,
		(unsigned char *) (*response ? challenge : 0),
		(unsigned char *) (*response ? response : challenge), 0))
	{
		pipe_exec(argv, tmpbuf, offset);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	status = 0;
	if ((ptr = (char *) getenv("POSTAUTH")) && !access(ptr, X_OK))
	{
		snprintf(buf, MAX_BUFF, "%s %s", ptr, login);
		status = runcmmd(buf, 0);
	}
	_exit(status);
	/*- Not reached */
	return(0);
}
#else
int
main(int argc, char **argv)
{
	if(argc < 2)
		_exit(2);
	execvp(argv[1], argv + 1);
	fprintf(stderr, "syspass: %s: %s\n", argv[1], strerror(errno));
	printf("454-%s (#4.3.0)\r\n", strerror(errno));
	fflush(stdout);
	_exit(111);
	/*- Not reached */
	return(0);
}
#endif

void
getversion_syspass_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
