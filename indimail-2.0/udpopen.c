/*
 * $Log: udpopen.c,v $
 * Revision 2.4  2015-04-14 20:05:21+05:30  Cprogrammer
 * use udp service and htons for port
 *
 * Revision 2.3  2015-04-10 19:01:17+05:30  Cprogrammer
 * use SOCK_DRAM for UDP
 *
 * Revision 2.2  2011-04-03 09:38:29+05:30  Cprogrammer
 * ported for ipv6
 *
 * Revision 2.1  2002-07-31 14:54:50+05:30  Cprogrammer
 * function to open a udp connection
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: udpopen.c,v 2.4 2015-04-14 20:05:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#include "indimail.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define FMT_ULONG 40

int
udpopen(rhost, servicename)
	char           *rhost, *servicename;
{
	int             fd, retval;
	struct servent *sp;	/*- pointer to service information */
#ifdef ENABLE_IPV6
	struct addrinfo hints, *res, *res0;
	char            serv[FMT_ULONG];
#else
	struct hostent *hp;	/*- pointer to host info for nameserver host */
	struct sockaddr_in tcp_srv_addr;
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
#endif

#ifdef ENABLE_IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (!servicename)
	{
		errno = EINVAL;
		return (-1);
	} 
	if (isnum(servicename))
		scopy(serv, servicename, FMT_ULONG);
	else
	{
		if ((sp = getservbyname(servicename, "udp")) == NULL)
		{
			errno = EINVAL;
			return (-1);
		}
		snprintf(serv, FMT_ULONG, "%d", htons(sp->s_port));
	}
	if ((retval = getaddrinfo(rhost, serv, &hints, &res0)))
	{
		fprintf(stderr, "tcpopen: getaddrinfo: %s\n", gai_strerror(retval));
		return (-1);
	}
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next)
	{
		if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
			continue; /*- Try the next address record in the list. */
		if (!(retval = connect(fd, res->ai_addr, res->ai_addrlen)))
			break;
		close(fd);
		fd = -1;
	} /*- for (res = res0; res; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	/*- clear out address structures */
	memset((char *) &tcp_srv_addr, 0, sizeof(struct sockaddr_in));
	tcp_srv_addr.sin_family = AF_INET;
	if (isnum(servicename))
		tcp_srv_addr.sin_port = htons(atoi(servicename));
	else
	if ((sp = getservbyname(servicename, "udp")))
		tcp_srv_addr.sin_port = sp->s_port;
	else
	{
		fprintf(stderr, "%s/udp: No such service\n", servicename);
		errno = EINVAL;
		return (-1);
	}
	if ((inaddr = inet_addr(rhost)) != INADDR_NONE) /*- It's a dotted decimal */
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
	else
	if ((hp = gethostbyname(rhost)) == NULL)
	{
		fprintf(stderr, "gethostbyname: %s: No such Host\n", rhost);
		errno = EINVAL;
		return (-1);
	} else
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, hp->h_addr, hp->h_length);
	/*- Create the socket. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);
	if ((retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))) == -1)
	{
		close(fd);
		return (-1);
	}
#endif
	return(fd);
}

void
getversion_udpopen_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
