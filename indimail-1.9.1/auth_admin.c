/*
 * $Log: auth_admin.c,v $
 * Revision 2.9  2016-01-28 16:11:50+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 2.8  2015-08-21 10:45:40+05:30  Cprogrammer
 * fix for 'stack smashing detected' when compiled with -fstack-protector
 *
 * Revision 2.7  2009-09-17 10:33:22+05:30  Cprogrammer
 * fixed errno getting clobbered by ssl_free
 * display error from the server
 *
 * Revision 2.6  2009-09-16 10:20:24+05:30  Cprogrammer
 * fixed compilation problem if HAVE_SSL was not defined
 *
 * Revision 2.5  2009-08-11 16:57:37+05:30  Cprogrammer
 * added configurable admin_timeout for admin commands
 *
 * Revision 2.4  2009-08-11 16:21:59+05:30  Cprogrammer
 * added ssl_free()
 *
 * Revision 2.3  2009-08-11 16:15:12+05:30  Cprogrammer
 * added TLS encryption
 *
 * Revision 2.2  2003-02-19 22:23:46+05:30  Cprogrammer
 * added errno=EPROTO for protocol error
 *
 * Revision 2.1  2002-07-15 18:40:25+05:30  Cprogrammer
 * admin authentication routine
 *
 */
#include "indimail.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SSL
#include <sys/types.h>
#endif

#ifdef HAVE_SSL
ssize_t         saferead(int, char *, int, int);
ssize_t         safewrite(int, char *, int, int);
int             tls_init(int, char *);
void            ssl_free();
#endif

#ifndef lint
static char     sccsid[] = "$Id: auth_admin.c,v 2.9 2016-01-28 16:11:50+05:30 Cprogrammer Exp mbhangui $";
#endif

int
auth_admin(char *admin_user, char *admin_pass, char *admin_host, char *admin_port, char *clientcert)
{
	int             sfd, len, admin_timeout;
	char            buffer[MAX_BUFF];

	if ((sfd = tcpopen(admin_host, 0, atoi(admin_port))) == -1)
	{
		fprintf(stderr, "tcpopen: %s:%s: %s\n", admin_host, admin_port, strerror(errno));
		return (-1);
	}
	getEnvConfigInt(&admin_timeout, "ADMIN_TIMEOUT", 120);
#ifdef HAVE_SSL
	if (clientcert && tls_init(sfd, clientcert))
		return (-1);
#endif
	if ((len = saferead(sfd, buffer, sizeof(buffer) - 1, admin_timeout)) == -1 || !len)
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	buffer[len] = 0;
	if (!strstr(buffer, "Login: "))
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	len = strlen(admin_user);
	if (safewrite(sfd, admin_user, len, admin_timeout) != len || safewrite(sfd, "\n", 1, admin_timeout) != 1)
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if ((len = saferead(sfd, buffer, sizeof(buffer) - 1, admin_timeout)) == -1 || !len)
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	buffer[len] = 0;
	if (!strstr(buffer, "Password: "))
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	len = strlen(admin_pass);
	if (safewrite(sfd, admin_pass, len, admin_timeout) != len || safewrite(sfd, "\n", 1, admin_timeout) != 1)
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if ((len = saferead(sfd, buffer, sizeof(buffer) - 1, admin_timeout)) == -1 || !len)
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if (!strstr(buffer, "OK"))
	{
		(void) close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		(void) write(1, buffer, len);
		errno = EPROTO;
		return (-1);
	}
	return (sfd);
}

void
getversion_auth_admin_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
