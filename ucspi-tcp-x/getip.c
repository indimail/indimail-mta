/*
 * $Id: getip.c,v 1.1 2024-10-05 22:30:15+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <byte.h>
#include <stralloc.h>
#include <strerr.h>
#include "haveip6.h"
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#endif

int
isip(const char *addr)
{
	int                 i;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	struct in_addr      addr4;
	struct in6_addr     addr6;

	if ((i = inet_pton(AF_INET, addr, &addr4)) == 1)
		return 1;
	else
	if (i == -1)
		return -1;
	if ((i = inet_pton(AF_INET6, addr, &addr6)) == 1)
		return 1;
	else
	if (i == -1)
		return -1;
#else
	struct in_addr      addr4;

	if ((i = inet_aton(addr, &addr4)) == 1)
		return 1;
	else
	if (i == -1)
		return -1;
#endif
	return 0;
}

int
getip(const char *host, stralloc *insa)
{
	int                 i;
	struct in_addr      addr4;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	char                addrBuf[INET6_ADDRSTRLEN];
	struct addrinfo     hints = {0}, *addr_res = 0, *addr_res0 = 0;
	struct sockaddr     sa;
	struct in6_addr     addr6;
	struct sockaddr_in *in4 = (struct sockaddr_in *) &sa;
	struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) &sa;
#else
	char          **ptr;
	struct hostent *hp;
#endif

	insa->len = 0;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	if ((i = inet_pton(AF_INET, host, &addr4)) == 1) {
		if (!stralloc_ready(insa, 16))
			strerr_die1x(111, "tcpclient: fatal: out of memory");
		insa->len = 16;
		byte_copy(insa->s, 12, (char *) V4mappedprefix);
		byte_copy(insa->s + 12, 4, (char *) &addr4);
		return 0;
	} else
	if (i == -1)
		strerr_die1x(111, "tcpclient: fatal: invalid address family AF_INET");
	if ((i = inet_pton(AF_INET6, host, &addr6)) == 1) {
		if (!stralloc_ready(insa, 16))
			strerr_die1x(111, "tcpclient: fatal: out of memory");
		insa->len = 16;
		byte_copy(insa->s, 16, (char *) &addr6);
		return 0;
	} else
	if (i == -1)
		strerr_die1x(111, "tcpclient: fatal: invalid address family AF_INET6");
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = 0;
	hints.ai_addr = 0;
	hints.ai_next = 0;
	if ((i = getaddrinfo(host, 0, &hints, &addr_res0))) {
		if (i == EAI_NONAME)
			return 1;
		strerr_die2x(111, "tcpclient: fatal: getaddrinfo: ", gai_strerror(i));
	}
	for (addr_res = addr_res0; addr_res; addr_res = addr_res->ai_next) {
		byte_copy((char *) &sa, addr_res->ai_addrlen, (char *) addr_res->ai_addr);
		if (sa.sa_family == AF_INET) {
			in4 = (struct sockaddr_in *) &sa;
			if (!inet_ntop(AF_INET, (void *) &in4->sin_addr, addrBuf, INET_ADDRSTRLEN)) {
				freeaddrinfo(addr_res0);
				strerr_die1sys(111, "tcpclient: fatal: inet_ntop");
			}
			if (!stralloc_ready(insa, insa->len + 16))
				strerr_die1x(111, "tcpclient: fatal: out of memory");
			byte_copy(insa->s + insa->len, 12, (char *) V4mappedprefix);
			byte_copy(insa->s + insa->len + 12, 4, (char *) &in4->sin_addr);
			insa->len += 16;
		} else
		if (sa.sa_family == AF_INET6) {
			in6 = (struct sockaddr_in6 *) &sa;
			if (!inet_ntop(AF_INET6, (void *) &in6->sin6_addr, addrBuf, INET6_ADDRSTRLEN)) {
				freeaddrinfo(addr_res0);
				strerr_die1sys(111, "tcpclient: fatal: inet_ntop");
			}
			if (!stralloc_ready(insa, insa->len + 16))
				strerr_die1x(111, "tcpclient: fatal: out of memory");
			byte_copy(insa->s + insa->len, 16, (char *) &in6->sin6_addr);
			insa->len += 16;
		} else
			continue;
	}
	freeaddrinfo(addr_res0);
	return insa->len ? 0 : 1;
#else
	if ((i = inet_aton(host, &addr4)) == 1) {
		if (!stralloc_ready(insa, 4))
			strerr_die1x(111, "tcpclient: fatal: out of memory");
		insa->len = 4;
		byte_copy(insa->s, 4, (char *) &addr4);
		return 0;
	} else
	if (i == -1)
		return -1;
	if (!(hp = gethostbyname(host))) {
		if (h_errno == HOST_NOT_FOUND)
			return 1;
		else
			strerr_die2sys(111, "tcpclient: fatal: gethostbyname: ", hstrerror(h_errno));
	}
	for (ptr = hp->h_addr_list; addr_list[i]; ptr++) {
		if (!stralloc_ready(insa, insa->len + 4))
			strerr_die1x(111, "tcpclient: fatal: out of memory");
		byte_copy(insa->s + insa->len, 4, *ptr);
		insa->len += 4;
	}
	return 0;
#endif
}

/*
 * $Log: getip.c,v $
 * Revision 1.1  2024-10-05 22:30:15+05:30  Cprogrammer
 * Initial revision
 *
 */
