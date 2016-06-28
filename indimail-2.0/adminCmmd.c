/*
 * $Log: adminCmmd.c,v $
 * Revision 2.12  2016-01-28 16:11:43+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 2.11  2015-08-21 10:44:34+05:30  Cprogrammer
 * fix for 'stack smashing detected' when compiled with -fstack-protector
 *
 * Revision 2.10  2009-09-16 09:04:12+05:30  Cprogrammer
 * fixed error when SSL was not defined
 *
 * Revision 2.9  2009-08-11 16:57:23+05:30  Cprogrammer
 * added timeout for admin commands
 *
 * Revision 2.8  2009-08-11 16:22:10+05:30  Cprogrammer
 * added tls encryption
 *
 * Revision 2.7  2009-02-06 11:36:06+05:30  Cprogrammer
 * check return value of read
 *
 * Revision 2.6  2008-07-13 19:15:25+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.5  2005-12-29 22:39:05+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.4  2003-03-24 19:15:33+05:30  Cprogrammer
 * compilation fix for solaris
 *
 * Revision 2.3  2003-02-03 21:53:59+05:30  Cprogrammer
 * added select() to allow passing of data from stdin to indisrvr
 *
 * Revision 2.2  2002-07-25 10:11:43+05:30  Cprogrammer
 * added printing of messages from sifsrvr
 *
 * Revision 2.1  2002-07-15 18:57:45+05:30  Cprogrammer
 * function to send command to indisrvr
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef sun
#include <sys/filio.h>
#endif

ssize_t         saferead(int, char *, int, int);
ssize_t         safewrite(int, char *, int, int);
#ifdef HAVE_SSL
void            ssl_free();
#endif

#ifndef lint
static char     sccsid[] = "$Id: adminCmmd.c,v 2.12 2016-01-28 16:11:43+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      IOPlex(int, int);

int
adminCmmd(int sfd, int inputRead, char *cmdbuf, int len)
{
	int             retval, n, admin_timeout;
	char            buffer[MAX_BUFF];
	char           *ptr;

	getEnvConfigInt(&admin_timeout, "ADMIN_TIMEOUT", 120);
	if ((n = safewrite(sfd, cmdbuf, len, admin_timeout)) != len)
	{
		fprintf(stderr, "safewrite: %d bytes: %s\n", n, strerror(errno));
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = safewrite(sfd, "\n", 1, admin_timeout)) != 1) /*- To send the command */
	{
		fprintf(stderr, "safewrite: %d bytes: %s\n", n, strerror(errno));
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if ((n = safewrite(sfd, "\n", 1, admin_timeout)) != 1) /*- to make indisrvr go forward after wait() */
	{
		fprintf(stderr, "safewrite: %d bytes: %s\n", n, strerror(errno));
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	if (inputRead)
		IOPlex(sfd, admin_timeout);
	for (retval = -1;;)
	{
		if ((n = saferead(sfd, buffer, sizeof(buffer) - 1, admin_timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			perror("read");
#ifdef HAVE_SSL
			ssl_free();
#endif
			return (-1);
		} else
		if (!n)
			break;
		buffer[n] = 0;
		if ((ptr = strstr(buffer, "RETURNSTATUS")))
		{
			*ptr = 0;
			retval = atoi(buffer + 12);
			filewrt(1, "%s", buffer);
			continue;
		} else
			(void) write(1, buffer, n);
	}
#ifdef HAVE_SSL
	ssl_free();
#endif
	return (retval);
}

static int
IOPlex(int sockfd, int admin_timeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout, Timeout;
	time_t          idle_time;
	int             retval, retrycount, dataTimeout;
	char           *ptr;
	char            buffer[SOCKBUF + 1];
	void            (*pstat) ();

	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
		return (-1);
	retval = 1;
#ifdef linux
#include <asm/ioctls.h>
#endif
	if (ioctl(0, FIONBIO, &retval) != -1)
	{
		retval = 0;
		if (read(0, "", 0) == -1 || ioctl(0, FIONBIO, &retval) == -1)
		{
			fprintf(stderr, "monkey-read-ioctl-FIONBIO: %s\n", strerror(errno));
			(void) close(sockfd);
			(void) signal(SIGPIPE, pstat);
			return(-1);
		}
	}  else
	{
		(void) close(sockfd);
		(void) signal(SIGPIPE, pstat);
		return(-1);
	}
	Timeout.tv_sec = timeout.tv_sec = SELECTTIMEOUT;
	Timeout.tv_usec = timeout.tv_usec = 0;
	getEnvConfigStr(&ptr, "DATA_TIMEOUT", "1800");
	idle_time = 0;
	dataTimeout = atoi(ptr);
	for (retrycount = 0;;)
	{
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &Timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			fprintf(stderr, "IOPlex: %s\n", strerror(errno));
			(void) signal(SIGPIPE, pstat);
			return (-1);
		} else
		if (!retval)
		{
			idle_time += timeout.tv_sec;
			if (idle_time > dataTimeout)
			{
				close(sockfd);
				(void) signal(SIGPIPE, pstat);
				fprintf(stderr, "monkey-select: Timeout %ld\n", timeout.tv_sec);
				return(2);
			}
			timeout.tv_sec += 2 * timeout.tv_sec;
			Timeout.tv_sec = timeout.tv_sec;
			continue;
		} else
		{
			timeout.tv_sec = SELECTTIMEOUT;
			idle_time = 0;
		}
		if (FD_ISSET(0, &rfds))
		{
			/*- Data from Client -*/
			for (;;)
			{
				errno = 0;
				if ((retval = read(0, buffer, SOCKBUF)) == -1)
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY)
					{
						(void) usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE)
					{
						retval = 0;
						break;
					}
#endif
					fprintf(stderr, "IOPlex: %s\n", strerror(errno));
					(void) signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from client -*/
				break;
			/*- Write to Remote Server -*/
			if (safewrite(sockfd, buffer, retval, admin_timeout) != retval)
			{
				fprintf(stderr, "IOPlex: %s\n", strerror(errno));
				(void) signal(SIGPIPE, pstat);
				return (-1);
			}
		} /*- if (FD_ISSET(0, &rfds)) -*/
	}
	usleep(1000);
	shutdown(sockfd, SHUT_WR);
	(void) signal(SIGPIPE, pstat);
	return(0);
}

void
getversion_adminCmmd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
