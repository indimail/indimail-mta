/*
 * $Log: tcpopen.c,v $
 * Revision 2.11  2015-08-21 10:47:49+05:30  Cprogrammer
 * fixed compilation error
 *
 * Revision 2.10  2015-08-19 16:32:59+05:30  Cprogrammer
 * added missing call to getservbyname()
 *
 * Revision 2.9  2011-04-08 17:27:15+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.8  2011-04-03 09:37:23+05:30  Cprogrammer
 * try next address record when ipv6 socket fails
 *
 * Revision 2.7  2011-04-03 09:33:44+05:30  Cprogrammer
 * set fd to -1 when connect fails for ipv6
 *
 * Revision 2.6  2011-04-03 09:28:33+05:30  Cprogrammer
 * fix for trying the next IPv6 address record in list
 *
 * Revision 2.5  2010-03-26 19:17:41+05:30  Cprogrammer
 * return correct error instead of EINVAL
 *
 * Revision 2.4  2009-07-09 15:55:33+05:30  Cprogrammer
 * ipv6 ready code
 *
 * Revision 2.3  2008-07-13 19:48:07+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.2  2002-12-11 10:28:57+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.1  2002-12-02 21:48:45+05:30  Cprogrammer
 * added check for in_addr_t
 *
 * Revision 1.5  2002-03-03 15:40:47+05:30  Cprogrammer
 * changed strncpy to scopy
 *
 * Revision 1.4  2001-12-19 16:29:10+05:30  Cprogrammer
 * added code to connect to unix domain sockets
 *
 * Revision 1.3  2001-12-13 11:52:27+05:30  Cprogrammer
 * variable for inaddr changed to in_addr_t
 *
 * Revision 1.2  2001-12-11 11:32:53+05:30  Cprogrammer
 * inclusion of systeminfo.h for solaris
 *
 * Revision 1.1  2001-12-07 22:29:09+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: tcpopen.c,v 2.11 2015-08-21 10:47:49+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef sun
#include <sys/systeminfo.h>
#endif

#define FMT_ULONG 40

static unsigned sleeptime = MAXSLEEP + 1; /*- 0 for infinite connect */

int
tcpopen(host, service, port) /*- Thanks to Richard's Steven */
	char           *host;
	char           *service;
	int             port;
/*-
 * host    - Name or dotted decimal address of the
 *           other system / Pathname for a unix domain socket
 * service - Name of the service being requested.
 *           Can be NULL, if port > 0.
 * port    - if == 0, nothin special, use port# of the service.
 *           if < 0, use a reserved port
 *           if > 0, it is the port# of server (host-byte-order)
 */
{
	int             resvport, fd = -1, optval, retval;
	char           *ptr, *hostptr;
	struct servent *sp;
#ifdef ENABLE_IPV6
	struct addrinfo hints, *res, *res0;
	char            serv[FMT_ULONG];
#else
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	struct hostent *hp;
	struct sockaddr_in tcp_srv_addr;/*- server's Internet socket address */
#endif
	struct sockaddr_un unixaddr;	/*- server's local unix socket address */
	struct linger   linger;
	char           *dir;
	char            localhost[MAXHOSTNAMELEN];

	if (host && *host && ((strchr(host, '/') || ((dir = Dirname(host)) && !access(dir, F_OK)))))
	{
		if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
        	return -1;
    	unixaddr.sun_family = AF_UNIX;
    	scopy (unixaddr.sun_path, host, sizeof(unixaddr.sun_path));
    	if (connect (fd, (struct sockaddr *) &unixaddr, sizeof(struct sockaddr_un) ) == -1)
        	return -1;
		return(fd);
	}
#ifdef sun
	if (sysinfo(SI_HOSTNAME, localhost, MAXHOSTNAMELEN) > MAXHOSTNAMELEN)
#else
	if (gethostname(localhost, MAXHOSTNAMELEN))
#endif
		return (-1);
	if (!strcmp(host, localhost) || !strcmp(host, "localhost"))
		hostptr = "localhost";
	else
		hostptr = host;
	if ((ptr = (char *) getenv("SLEEPTIME")) != (char *) 0)
	{
		if (isnum(ptr))
			sleeptime = atoi(ptr);
		else
		{
			errno = EINVAL;
			return (-1);
		}
	}
#ifdef ENABLE_IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (service != (char *) NULL)
	{
		if (port > 0)
			snprintf(serv, FMT_ULONG, "%d", port);
		else
		{
			if ((sp = getservbyname(service, "tcp")) == NULL)
			{
				errno = EINVAL;
				return (-1);
			}
			snprintf(serv, FMT_ULONG, "%d", htons(sp->s_port));	/*- service's value */
		}
	} else
	if (port <= 0)
	{
		errno = EINVAL;
		return (-1);
	} else
		snprintf(serv, FMT_ULONG, "%d", port);
	if ((retval = getaddrinfo(hostptr, serv, &hints, &res0)))
	{
		fprintf(stderr, "tcpopen: getaddrinfo: %s\n", gai_strerror(retval));
		return (-1);
	}
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next)
	{
		for (;;)
		{
			if (port >= 0)
			{
				if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
					break; /*- Try the next address record in the list. */
			}
#ifndef WindowsNT
			else /*- if (port < 0) */
			{
				resvport = IPPORT_RESERVED - 1;
				if ((fd = rresvport_af(&resvport, res->ai_family)) < 0) /*- RFC 2292 */
				{
					freeaddrinfo(res0);
					return (-1);
				}
			}
#endif /*- #ifndef WindowsNT */
			for (errno = 0;;)
			{
				if ((retval = connect(fd, res->ai_addr, res->ai_addrlen)) != -1)
					break;
				else
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					if (errno == ECONNREFUSED)
					{
						if (sleeptime <= MAXSLEEP)
						{
							if (sleeptime)
								(void) sleep(sleeptime);
							else
								(void) sleep(5);
							sleeptime += sleeptime;
							(void) close(fd);
							errno = ECONNREFUSED;
							break;
						} else
						{
							(void) close(fd);
							errno = ECONNREFUSED;
							freeaddrinfo(res0);
							return (-1);
						}
					}	/*- if (errno == ECONNREFUSED) */
					optval = errno;
					(void) close(fd);
					errno = optval;
					fd = -1;
					break;
				}
			} /*- for (errno = 0;;) */
	 		if (!retval || errno != ECONNREFUSED)
	 			break;
		} /*- for (;;) */
		/*- try the next address record in list */
	} /*- for (res = res0; res && fd == -1; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	/*
	 * Initialize the server's Internet address structure. We'll store
	 * the actual 4-byte Internet address and the 2-byte port # below.
	 */
	(void) memset((char *) &tcp_srv_addr, 0, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family = AF_INET;
	if (service != (char *) NULL)
	{
		if (port > 0)
			tcp_srv_addr.sin_port = htons(port);	/*- caller's value */
		else
		{
			if ((sp = getservbyname(service, "tcp")) == NULL)
			{
				errno = EINVAL;
				return (-1);
			}
			tcp_srv_addr.sin_port = htons(sp->s_port);	/*- service's value */
		}
	} else
	if (port <= 0)
	{
		errno = EINVAL;
		return (-1);
	} else
		tcp_srv_addr.sin_port = htons(port);
	/*
	 * First try to convert the hostname as the dotted decimal number.
	 * Only if that fails, call gethostbyname.
	 */
	if ((inaddr = inet_addr(hostptr)) != INADDR_NONE)
	{			/*- It's a dotted decimal */
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
	} else
	if ((hp = gethostbyname(hostptr)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	} else
	{
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, hp->h_addr, hp->h_length);
	}
	for (;;)
	{
		if (port >= 0)
		{
			if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				return (-1);
		}
#ifndef WindowsNT
		else /*- if (port < 0) */
		{
			resvport = IPPORT_RESERVED - 1;
			if ((fd = rresvport(&resvport)) < 0)
				return (-1);
		}
#endif /*- #ifndef WindowsNT */
#if !defined(linux) && !defined(CYGWIN) && !defined(WindowsNT)
		if (!strcmp(hostptr, "localhost"))
		{
			optval = 1;
			if (setsockopt(fd, SOL_SOCKET, SO_USELOOPBACK, (char *) &optval, sizeof(optval)) == -1)
			{
				(void) close(fd);
				return (-1);
			}
		}
#endif
		/*- Connect to the server. */
		for (errno = 0;;)
		{
			if ((retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))) != -1)
				break;
			else
			{
#ifdef ERESTART
				if (errno == EINTR || errno == ERESTART)
#else
				if (errno == EINTR)
#endif
					continue;
				if (errno == ECONNREFUSED)
				{
					if (sleeptime <= MAXSLEEP)
					{
						if (sleeptime)
							(void) sleep(sleeptime);
						else
							(void) sleep(5);
						sleeptime += sleeptime;
						(void) close(fd);
						errno = ECONNREFUSED;
						break;
					} else
					{
						(void) close(fd);
						errno = ECONNREFUSED;
						return (-1);
					}
				}	/*- if (errno == ECONNREFUSED) */
				optval = errno;
				(void) close(fd);
				errno = optval;
				return (-1);
			}
		} /*- for (errno = 0;;) */
	 	if (!retval)
	 		break;
	} /*- for (;;) */
#endif /*- #ifdef ENABLE_IPV6 */
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)) == -1)
	{
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_SNDBUF, SOCKBUF) == -1)
	{
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_RCVBUF, SOCKBUF) == -1)
	{
		(void) close(fd);
		return (-1);
	}
	return (fd);
}	/*- end tcp_open */

void
getversion_tcpopen_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
