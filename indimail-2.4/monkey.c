/*
 * $Log: monkey.c,v $
 * Revision 2.14  2009-02-18 21:29:51+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.13  2009-02-06 11:39:33+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.12  2008-07-13 19:45:29+05:30  Cprogrammer
 * port for Mac OS X
 * use ERESTART only if available
 *
 * Revision 2.11  2007-12-22 00:27:25+05:30  Cprogrammer
 * timeout is undefined in linux after select() returns. Set it to a sane value
 *
 * Revision 2.10  2005-12-29 22:46:43+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.9  2005-01-17 16:04:34+05:30  Cprogrammer
 * set the select timeout to DATA_TIMEOUT if DATA_TIMEOUT > SELECTTIMEOUT
 *
 * Revision 2.8  2003-01-08 20:01:50+05:30  Cprogrammer
 * added sys/systeminfo.h for solaris SI_HOSTNAME definition
 *
 * Revision 2.7  2002-12-12 17:03:30+05:30  Cprogrammer
 * improved idle time logic
 *
 * Revision 2.6  2002-12-11 10:28:20+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.5  2002-12-06 14:28:29+05:30  Cprogrammer
 * Return error code 2 in case of data timeout
 *
 * Revision 2.4  2002-12-06 09:42:39+05:30  Cprogrammer
 * made select timeout configurable
 *
 * Revision 2.3  2002-11-21 00:57:42+05:30  Cprogrammer
 * timeout and close connection if idle for more than 1800 secs
 *
 * Revision 2.2  2002-08-09 20:20:48+05:30  Cprogrammer
 * corrected extra lines getting skipped
 *
 * Revision 2.1  2002-06-22 01:58:04+05:30  Cprogrammer
 * corrected logic for skipping newlines
 *
 * Revision 1.3  2002-03-28 05:10:37+05:30  Cprogrammer
 * added code to skip duplicate new lines when doing proxy
 *
 * Revision 1.2  2001-12-11 11:32:11+05:30  Cprogrammer
 * inclusion of fileio.h and param.h
 *
 * Revision 1.1  2001-12-08 00:31:48+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: monkey.c,v 2.14 2009-02-18 21:29:51+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#ifdef sun
#include <sys/filio.h>
#include <sys/systeminfo.h>
#endif
#ifdef linux
#include <sys/param.h>
#endif

int
monkey(host, servicename, startbuf, skip_nl)
	char           *host, *servicename, *startbuf;
	int             skip_nl;
{
	int             sockfd, retval, retrycount, gotNl, dataTimeout;
	fd_set          rfds;	/*- File descriptor mask for select -*/
	char            buffer[SOCKBUF + 1];
#ifdef _SC_HOST_NAME_MAX
	char            localhost[_SC_HOST_NAME_MAX];
#else
	char            localhost[MAXHOSTNAMELEN];
#endif /*- SC_HOST_NAME_MAX */
	struct timeval  timeout;
	time_t          idle_time, last_timeout;
	char           *ptr, *cptr;
	void            (*pstat) ();

#ifdef SOLARIS
#ifdef _SC_HOST_NAME_MAX
	if (sysinfo(SI_HOSTNAME, localhost, _SC_HOST_NAME_MAX) > _SC_HOST_NAME_MAX)
#else
	if (sysinfo(SI_HOSTNAME, localhost, MAXHOSTNAMELEN) > MAXHOSTNAMELEN)
#endif /*- SC_HOST_NAME_MAX */
#else
#ifdef _SC_HOST_NAME_MAX
	if (gethostname(localhost, _SC_HOST_NAME_MAX))
#else
	if (gethostname(localhost, MAXHOSTNAMELEN))
#endif /*- SC_HOST_NAME_MAX */
#endif
		return (-1);
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
		return (-1);
	if (!strcmp(host, localhost))
		ptr = "localhost";
	else
		ptr = host;
	/*- IP connection service provided by inetd -*/
	if (isnum(servicename))
		sockfd = tcpopen(ptr, (char *) 0, atoi(servicename));
	else
		sockfd = tcpopen(ptr, servicename, 0);
	if (sockfd == -1)
	{
		fprintf(stderr, "monkey-%s:%s: %s\n", ptr, servicename, strerror(errno));
		(void) signal(SIGPIPE, pstat);
		return (-1);
	} 
	retval = 1;
#ifdef linux
#include <asm/ioctls.h>
#endif
	if (ioctl(0, FIONBIO, &retval) != -1)
	{
		retval = 0;
		if (read(0, "", 0) == -1) ;
		if (ioctl(0, FIONBIO, &retval) == -1)
		{
			fprintf(stderr, "monkey-ioctl-FIONBIO: %s\n", strerror(errno));
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
	if(startbuf && *startbuf)
		sockwrite(sockfd, startbuf, strlen(startbuf));
	sockwrite(sockfd, "", 0);
	getEnvConfigStr(&ptr, "DATA_TIMEOUT", "1800");
	dataTimeout = atoi(ptr);
	timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
	timeout.tv_usec = 0;
	idle_time = 0; /*- for how long connection is idle */
	last_timeout = timeout.tv_sec;
	for (gotNl = retrycount = 0;;)
	{
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(sockfd, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			fprintf(stderr, "monkey-select: %s\n", strerror(errno));
			(void) close(sockfd);
			(void) signal(SIGPIPE, pstat);
			return (-1);
		} else
		if (!retval) /*- timeout occurred */
		{
			idle_time += last_timeout - timeout.tv_sec;
			if (idle_time >= dataTimeout)
			{
				close(sockfd);
				(void) signal(SIGPIPE, pstat);
				fprintf(stderr, "monkey-select: Timeout %ld\n", timeout.tv_sec);
				return(2);
			}
			last_timeout += 2 * last_timeout;
			timeout.tv_sec = last_timeout;
			continue;
		} else
		{
			last_timeout = timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
			timeout.tv_usec = idle_time = 0;
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
					fprintf(stderr, "monkey-read-src: %s\n", strerror(errno));
					(void) close(sockfd);
					(void) signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from client -*/
				break;
			/*- Write to Remote Server -*/
			if (sockwrite(sockfd, buffer, retval) != retval)
			{
				fprintf(stderr, "monkey-sockwrite-dst: %s\n", strerror(errno));
				(void) close(sockfd);
				(void) signal(SIGPIPE, pstat);
				return (-1);
			}
		} /*- if (FD_ISSET(0, &rfds)) -*/
		if (FD_ISSET(sockfd, &rfds))
		{
			/*- Data from Remote Server -*/
			for (;;)
			{
				errno = 0;
				if ((retval = read(sockfd, buffer, SOCKBUF)) < 0)
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
					fprintf(stderr, "monkey-read-dst: %s\n", strerror(errno));
					(void) close(sockfd);
					(void) signal(SIGPIPE, pstat);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from server -*/
				break;
			/*- Write data to Client -*/
			if(gotNl < skip_nl)
			{
				buffer[retval] = 0;
				for(ptr = buffer, cptr = strchr(buffer, '\n');*ptr && cptr;)
				{
					if((cptr = strchr(ptr, '\n')))
					{
						gotNl++;
						if(gotNl == skip_nl)
						{
							if((retval - (cptr - buffer + 1)) > 0)
								if (write(1, cptr + 1, retval - (cptr - buffer + 1)) == -1) ;
						}
						if(cptr < buffer + retval)
							ptr = cptr + 1;
					}
				}
				continue;
			}
			if (sockwrite(1, buffer, retval) != retval)
			{
				fprintf(stderr, "monkey-sockwrite-src: %s\n", strerror(errno));
				(void) close(sockfd);
				(void) signal(SIGPIPE, pstat);
				return (-1);
			}
		}	/*- end of FD_ISSET(sockfd, &rfds) -*/
	}	/*- end of for -*/
	(void) usleep(1000);
	(void) close(sockfd);
	(void) signal(SIGPIPE, pstat);
	return (0);
}

void
getversion_monkey_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
