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

static const char rcsid[]="$Id: rfc1035sockaddrport.c,v 1.3 2000/05/28 18:17:25 mrsam Exp $";

int rfc1035_sockaddrport(const RFC1035_NETADDR *a, int al, int *port)
{
int	af=((const struct sockaddr_in *)a)->sin_family;

	if ( af == AF_INET )
	{
		if (al >= sizeof(struct sockaddr_in))
		{
			*port=((const struct sockaddr_in *)a)->sin_port;
			return (0);
		}
	}

#if RFC1035_IPV6

	if ( af == AF_INET6 )
	{
		if (al >= sizeof(struct sockaddr_in6))
		{
			*port=((const struct sockaddr_in6 *)a)->sin6_port;
			return (0);
		}
	}
#endif
	return (-1);
}
