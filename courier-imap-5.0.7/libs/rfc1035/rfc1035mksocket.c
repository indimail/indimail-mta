/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"rfc1035.h"
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<errno.h>


/*
**	Create a socket.  Duh.  If we've compiled IPv6 support, but we can't
**	create an IPv6 socket, create an IPv4 socket.  This can happen, say,
**	on Linux with IPv6 runtime libraries, but without IPv6 in the kernel.
*/

int	rfc1035_mksocket(int sock_type, int sock_protocol, int *af)
{
#if	RFC1035_IPV6
	int	s;
	int	on=0;

	*af=AF_INET6;
	if ( (s=socket(PF_INET6, sock_type, sock_protocol)) >= 0)
	{
#ifdef IPV6_V6ONLY

		setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
			   (char *)&on, sizeof(on));
#endif

		return (s);
	}
#endif
	*af=AF_INET;
	return (socket(PF_INET, sock_type, sock_protocol));
}
