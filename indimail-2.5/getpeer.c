/*
 * $Log: getpeer.c,v $
 * Revision 2.6  2011-04-08 17:26:12+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.5  2009-12-11 13:11:15+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.4  2009-08-11 13:41:34+05:30  Cprogrammer
 * use sockaddr_storage instead of sockaddr to prevent truncation of ipv6 addresses
 *
 * Revision 2.3  2009-07-09 15:54:39+05:30  Cprogrammer
 * ipv6 ready code
 *
 * Revision 2.2  2005-12-21 09:46:13+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.1  2002-12-26 03:28:14+05:30  Cprogrammer
 * added function getremoteip()
 *
 * Revision 1.5  2001-12-03 00:08:56+05:30  Cprogrammer
 * replaced ifndef linux with ifdef sun
 *
 * Revision 1.4  2001-11-28 22:58:59+05:30  Cprogrammer
 * added unistd.h and stropts.h for solaris
 *
 * Revision 1.3  2001-11-20 21:55:21+05:30  Cprogrammer
 * conditional inclusion of header file sys/filio.h for linux
 *
 * Revision 1.2  2001-11-20 10:54:57+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#ifdef sun
#include <sys/filio.h>
#include <stropts.h>
#endif
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#ifndef	lint
static char     sccsid[] = "$Id: getpeer.c,v 2.6 2011-04-08 17:26:12+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

/*-
 * Function: GetIpaddr
 *
 * Description:
 *     Returns pointers to the inet address string and the host string
 *     for the system that contacted inetd. Special code handles both
 *     tcp and udp connections.
-*/
char *
GetIpaddr()
{
#ifdef ENABLE_IPV6
	static char     addrBuf[INET6_ADDRSTRLEN];
	struct sockaddr_storage sa;
#else
	struct sockaddr sa;
#endif
	int             length = sizeof(sa);
	int             on;
	char            buf[256];

	/*
	 * Translate the internet address to a string 
	 */
	if (getpeername(0, (struct sockaddr *) &sa, (socklen_t *) &length) == -1)
	{
		/*
		 * If getpeername() fails we assume a udp socket... 
		 */
		switch (errno)
		{
		case ENOTSOCK:			/*- stdin is not a socket -*/
			return ("127.0.0.1");
		case ENOTCONN:			/*- assume UDP request -*/
			/*
			 * Set I/O to non-blocking 
			 */
			on = 1;
			if (ioctl(0, FIONBIO, &on) < 0)
				return ("?.?.?.?");
			on = 0;
			if (recvfrom(0, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) &sa, (socklen_t *) &length) < 0)
			{
				(void) ioctl(0, FIONBIO, &on);
				return ("?.?.?.?");
			}
			(void) ioctl(0, FIONBIO, &on);
			break;
		default:
			return ("?.?.?.?");
		}
	}
	/*
	 * Now that we have the remote host address, look up the remote host
	 * name. Use the address if name lookup fails. At present, we can
	 * only handle names or addresses that belong to the AF_INET and
	 * AF_CCITT family.
	 */
#ifdef ENABLE_IPV6
	if (((struct sockaddr *) &sa)->sa_family == AF_INET)
	{
		return ((char *) inet_ntop(AF_INET, (void *) &((struct sockaddr_in *) &sa)->sin_addr,
			addrBuf, INET_ADDRSTRLEN));
	} else
	if (((struct sockaddr *)&sa)->sa_family == AF_INET6)
	{
		return ((char *) inet_ntop(AF_INET6, (void *) &((struct sockaddr_in6 *) &sa)->sin6_addr,
			addrBuf, INET6_ADDRSTRLEN));
	} else
	if (((struct sockaddr *) &sa)->sa_family == AF_UNSPEC)
		return ("127.0.0.1");
#else
	if (sa.sa_family == AF_INET)
		return (inet_ntoa(((struct sockaddr_in *) &sa)->sin_addr));
	else
	if (sa.sa_family == AF_UNSPEC)
		return ("127.0.0.1");
#endif
	return ("0.0.0.0");
}

char           *
getremoteip()
{
	char           *tcpremoteip;
	struct sockaddr sa;
	int             i, status, dummy;
#ifdef ENABLE_IPV6
	static char     addrBuf[INET6_ADDRSTRLEN];
#endif

	dummy = sizeof(sa);
	for(tcpremoteip = (char *) 0, errno = i = 0;i < 128;i++)
	{
		if (!(status = getpeername(i, (struct sockaddr *) &sa, (socklen_t *) &dummy)))
		{
#ifdef ENABLE_IPV6
			if (sa.sa_family == AF_INET)
				tcpremoteip = (char *) inet_ntop(AF_INET, (void *) &((struct sockaddr_in *) &sa)->sin_addr,
					addrBuf, INET_ADDRSTRLEN);
			else
			if (sa.sa_family == AF_INET6)
				tcpremoteip = (char *) inet_ntop(AF_INET6, (void *) &((struct sockaddr_in6 *) &sa)->sin6_addr,
					addrBuf, INET6_ADDRSTRLEN);
#else
			if (sa.sa_family == AF_INET)
				tcpremoteip = inet_ntoa(((struct sockaddr_in *) &sa)->sin_addr);
#endif
			else
			if (sa.sa_family == AF_UNSPEC)
				return ("127.0.0.1");
			else
				return ("0.0.0.0");
			break;
		}
		if (errno == EBADF)
			return ((char *) 0);
	}
	return (tcpremoteip);
}

#include <stdio.h>
void
getversion_getpeer_c()
{
	printf("%s\n", sccsid);
}
