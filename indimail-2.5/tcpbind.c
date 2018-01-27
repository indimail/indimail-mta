/*
 * $Log: tcpbind.c,v $
 * Revision 2.6  2015-12-31 09:07:54+05:30  Cprogrammer
 * pass hostname parameter to getaddrinfo
 *
 * Revision 2.5  2015-08-19 16:32:00+05:30  Cprogrammer
 * fixed errno getting clobbered
 *
 * Revision 2.4  2011-04-08 17:27:12+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.3  2009-09-02 08:24:21+05:30  Cprogrammer
 * passed sizeof(struct sockaddr_in) to 3rd argument of bind
 *
 * Revision 2.2  2009-07-09 15:55:27+05:30  Cprogrammer
 * ipv6 ready code
 *
 * Revision 2.1  2002-12-02 21:48:16+05:30  Cprogrammer
 * added check for in_addr_t
 *
 * Revision 1.5  2002-03-03 15:40:37+05:30  Cprogrammer
 * changed strncpy to scopy
 *
 * Revision 1.4  2001-12-19 16:28:56+05:30  Cprogrammer
 * added code to bind on unix domain sockets
 *
 * Revision 1.3  2001-12-13 11:52:07+05:30  Cprogrammer
 * variable type for inaddr changed to in_addr_t
 *
 * Revision 1.2  2001-12-13 01:16:59+05:30  Cprogrammer
 * added argument hostname to allow binding on the ip address of the host
 *
 * Revision 1.1  2001-12-08 12:50:40+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: tcpbind.c,v 2.6 2015-12-31 09:07:54+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

int
tcpbind(hostname, servicename, backlog)
	char           *hostname, *servicename;
	int             backlog;
{
	int             listenfd;	/*- listen socket descriptor */
#ifdef ENABLE_IPV6
	struct addrinfo hints, *res, *res0;
	int             bind_fail = -1;
#else
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	struct sockaddr_in localinaddr;	/*- for local socket address */
	struct servent *sp;	/*- pointer to service information */
	struct hostent *hp;
#endif /*- #ifdef ENABLE_IPV6 */
	struct sockaddr_un localunaddr;	/*- for local socket address */
	char           *dir;
	int             idx, socket_type;
	struct linger cflinger;

	if ((dir = Dirname(hostname)) && !access(dir, F_OK))
		socket_type = AF_UNIX;
	else
		socket_type = AF_INET;
	if (socket_type == AF_UNIX)
	{
		(void) memset((char *) &localunaddr, 0, sizeof(struct sockaddr_un));
    	localunaddr.sun_family = AF_UNIX;
    	scopy(localunaddr.sun_path, hostname, sizeof(localunaddr.sun_path));
		if (!access(hostname, F_OK) && unlink(hostname))
		{
			errno = EEXIST;
			return(-1);
		}
		if ((listenfd = socket(socket_type, SOCK_STREAM, 0)) == -1)
			return (-1);
		idx = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof(idx)) < 0)
		{
			(void) close(listenfd);
			return (-1);
		}
		if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
		{
			(void) close(listenfd);
			return (-1);
		}
		if (bind(listenfd, (struct sockaddr *) &localunaddr, idx) == -1)
		{
			(void) close(listenfd);
			return (-1);
		}
	}
	/*- Set up address structure for the listen socket. */
	cflinger.l_onoff = 1;
	cflinger.l_linger = 60;
#ifdef ENABLE_IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((idx = getaddrinfo(hostname, servicename, &hints, &res0)))
	{
		fprintf(stderr, "tcpbind: getaddrinfo: %s\n", gai_strerror(idx));
		return (-1);
	}
	listenfd = -1;
	for (res = res0; res; res = res->ai_next) {
		if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		{
			freeaddrinfo(res0);
			return (-1);
		}
		idx = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof (int)) == -1)
		{
			close(listenfd);
			freeaddrinfo(res0);
			return (-1);
		}
		if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
		{
			close(listenfd);
			freeaddrinfo(res0);
			return (-1);
		}
		if ((bind_fail = bind(listenfd, res->ai_addr, res->ai_addrlen)) == 0)
			break;
		close(listenfd);
	} /*- for (res = res0; res; res = res->ai_next) */
	if (bind_fail == -1) {
		bind_fail = errno;
		freeaddrinfo(res0);
		errno = bind_fail;
		return (-1);
	}
	freeaddrinfo(res0);
#else
	(void) memset((char *) &localinaddr, 0, sizeof(struct sockaddr_in));
	localinaddr.sin_family = AF_INET;
	/*- It's a dotted decimal */
	if ((inaddr = inet_addr(hostname)) != INADDR_NONE)
		localinaddr.sin_addr.s_addr = inaddr;
	else
	if ((hp = gethostbyname(hostname)) != NULL)
		(void) memcpy((char *) &localinaddr.sin_addr, hp->h_addr, hp->h_length);
	else
	{
		(void) fprintf(stderr, "gethostbyname: %s: No such Host\n", hostname);
		errno = EINVAL;
		return (-1);
	}
	if (isnum(servicename))
		localinaddr.sin_port = htons(atoi(servicename));
	else
	if ((sp = getservbyname(servicename, "tcp")) != (struct servent *) 0)
		localinaddr.sin_port = sp->s_port;
	else
		return (-1);
	if ((listenfd = socket(socket_type, SOCK_STREAM, 0)) == -1)
		return (-1);
	idx = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof(idx)) < 0)
	{
		(void) close(listenfd);
		return (-1);
	}
	if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
	if (bind(listenfd, (struct sockaddr *) &localinaddr, sizeof(localinaddr)) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
#endif
	if (listen(listenfd, backlog) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
	return (listenfd);
}

void
getversion_tcpbind_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
