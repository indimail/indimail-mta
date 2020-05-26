/*
 * $Log: socket_send6.c,v $
 * Revision 1.2  2005-06-10 12:18:47+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:02:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"
#include "ip4.h"
#include "ip6.h"
#include "error.h"

extern int          noipv6;

int
socket_send6(int s, char *buf, unsigned int len, char ip[16], uint16 port, uint32 scope_id)
{
#ifdef LIBC_HAS_IP6
	struct sockaddr_in6 sa;
#else
	struct sockaddr_in sa;
#endif

	byte_zero(&sa, sizeof sa);
#ifdef LIBC_HAS_IP6
	if (noipv6)
	{
		if (ip6_isv4mapped(ip))
			return socket_send4(s, buf, len, ip + 12, port);
		if (byte_equal(ip, 16, V6loopback))
			return socket_send4(s, buf, len, ip4loopback, port);
		errno = error_proto;
		return -1;
	}
	sa.sin6_family = AF_INET6;
	uint16_pack_big((char *) &sa.sin6_port, port);
	byte_copy((char *) &sa.sin6_addr, 16, ip);
	return sendto(s, buf, len, 0, (struct sockaddr *) &sa, sizeof sa);
#else
		if (ip6_isv4mapped(ip))
			return socket_send4(s, buf, len, ip + 12, port);
		if (byte_equal(ip, 16, V6loopback))
			return socket_send4(s, buf, len, ip4loopback, port);
		errno = error_proto;
		return -1;
#endif
}
#endif
