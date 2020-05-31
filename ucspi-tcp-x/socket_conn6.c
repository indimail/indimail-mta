/*
 * $Log: socket_conn6.c,v $
 * Revision 1.2  2005-06-10 12:17:10+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:01:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "byte.h"
#include "uint32.h"
#include "socket.h"
#include "ip4.h"
#include "ip6.h"

extern int          noipv6;

int
socket_connect6(int s, char ip[16], uint16 port, uint32 scope_id)
{
#ifdef LIBC_HAS_IP6
	struct sockaddr_in6 sa;

	if (noipv6)
	{
		if (ip6_isv4mapped(ip))
			return socket_connect4(s, ip + 12, port);
		if (byte_equal(ip, 16, V6loopback))
			return socket_connect4(s, ip4loopback, port);
	}
	byte_zero(&sa, sizeof sa);
	sa.sin6_family = PF_INET6;
	uint16_pack_big((char *) &sa.sin6_port, port);
	sa.sin6_flowinfo = 0;
	sa.sin6_scope_id = scope_id;
	byte_copy((char *) &sa.sin6_addr, 16, ip);
	return connect(s, (struct sockaddr *) &sa, sizeof sa);
#else
	if (ip6_isv4mapped(ip))
		return socket_connect4(s, ip + 12, port);
	if (byte_equal(ip, 16, V6loopback))
		return socket_connect4(s, ip4loopback, port);
	errno = EPROTONOSUPPORT;
	return -1;
#endif
}
#endif
