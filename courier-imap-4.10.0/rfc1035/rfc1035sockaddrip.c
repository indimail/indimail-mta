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


int rfc1035_sockaddrip(const RFC1035_NETADDR *a, int al, RFC1035_ADDR *ip)
{
int	af=((const struct sockaddr_in *)a)->sin_family;

	if ( af == AF_INET )
	{
		if (al >= sizeof(struct sockaddr_in))
		{
#if RFC1035_IPV6
			rfc1035_ipv4to6(ip,
				&((const struct sockaddr_in *)a)->sin_addr);
#else
			*ip=((const struct sockaddr_in *)a)->sin_addr;
#endif
			return (0);
		}
	}

#if RFC1035_IPV6

	if ( af == AF_INET6 )
	{
		if (al >= sizeof(struct sockaddr_in6))
		{
			*ip=((const struct sockaddr_in6 *)a)->sin6_addr;
			return (0);
		}
	}
#endif
	return (-1);
}

