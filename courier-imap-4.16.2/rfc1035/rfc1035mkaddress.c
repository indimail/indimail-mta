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
**	Ok, take an address, and a port, and come back with a socket address
**	in the specified address family.
*/

int	rfc1035_mkaddress(int af,
			RFC1035_NETADDR *buf,
			const RFC1035_ADDR *addr,
			int port,
			const struct sockaddr **ptr,
			int *len)
{
struct sockaddr_in sin;

#if	RFC1035_IPV6

	if (af == AF_INET6)
	{
	struct sockaddr_in6 sin6;

		memset(&sin6, 0, sizeof(sin6));
		sin6.sin6_family=af;
		sin6.sin6_addr= *addr;
		sin6.sin6_port=port;
		memcpy(buf, &sin6, sizeof(sin6));
		*ptr=(struct sockaddr *)buf;
		*len=sizeof(sin6);
		return (0);
	}

	if (af != AF_INET || (!IN6_IS_ADDR_V4MAPPED(addr) && memcmp(addr, &in6addr_any, sizeof(*addr))))
	{
		errno=EAFNOSUPPORT;
		return (-1);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family=af;
	rfc1035_ipv6to4(&sin.sin_addr, addr);
	sin.sin_port=port;
#else
	if (af != AF_INET)
	{
		errno=EINVAL;
		return (-1);
	}
	memset(&sin, 0, sizeof(sin));
	sin.sin_family=af;
	sin.sin_addr= *addr;
	sin.sin_port=port;
#endif
	memcpy(buf, &sin, sizeof(sin));
	*ptr=(struct sockaddr *)buf;
	*len=sizeof(sin);
	return (0);
}
