/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"rfc1035.h"
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<errno.h>
#include	<string.h>


/*
**	We have two socket addresses.  Did they come from the same IP
**	address?
*/

#if RFC1035_IPV6

int rfc1035_same_ip(const void *a, int al, const void *b, int bl)
{
char	bufa[INET6_ADDRSTRLEN];
char	bufb[INET6_ADDRSTRLEN];
struct	in6_addr in6a;
struct	in6_addr in6b;
const char *as, *bs;

	if (rfc1035_sockaddrip(a, al, &in6a) ||
		rfc1035_sockaddrip(b, bl, &in6b) ||
		(as=inet_ntop(AF_INET6, &in6a, bufa, sizeof(bufa))) == 0 ||
		(bs=inet_ntop(AF_INET6, &in6b, bufb, sizeof(bufb))) == 0 ||
		strcmp(as, bs))
		return (0);
	return (1);
}

#else

int rfc1035_same_ip(const void *a, int al, const void *b, int bl)
{
	if ( ((const struct sockaddr_in *)a)->sin_family != AF_INET ||
		((const struct sockaddr_in *)b)->sin_family != AF_INET ||
		al < sizeof(struct sockaddr_in) ||
		bl < sizeof(struct sockaddr_in))
	{
		return (0);
	}

	return ( ((const struct sockaddr_in *)a)->sin_addr.s_addr ==
		((const struct sockaddr_in *)b)->sin_addr.s_addr);
}

#endif

