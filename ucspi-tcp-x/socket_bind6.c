/*
 * $Log: socket_bind6.c,v $
 * Revision 1.2  2020-08-03 17:29:50+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2005-06-12 23:31:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <byte.h>
#include "socket.h"
#include "ip6.h"
#include "error.h"

extern int          noipv6;

int
socket_bind6(int s, char ip[16], uint16 port, uint32 scope_id)
{
#ifdef LIBC_HAS_IP6
	struct sockaddr_in6 sa;

	if (noipv6) {
		int             i;
		for (i = 0; i < 16; i++) {
			if (ip[i] != 0)
				break;
		}
		if (i == 16 || ip6_isv4mapped(ip))
			return socket_bind4(s, ip + 12, port);
	}
	byte_zero((char *) &sa, sizeof sa);
	sa.sin6_family = AF_INET6;
	uint16_pack_big((char *) &sa.sin6_port, port);
	/*
	 * implicit: sa.sin6_flowinfo = 0; 
	 */
	byte_copy((char *) &sa.sin6_addr, 16, ip);
	sa.sin6_scope_id = scope_id;
	return bind(s, (struct sockaddr *) &sa, sizeof sa);
#else
	int             i;
	for (i = 0; i < 16; i++) {
		if (ip[i] != 0)
			break;
	}
	if (i == 16 || ip6_isv4mapped(ip))
		return socket_bind4(s, ip + 12, port);
	errno = error_proto;
	return -1;
#endif
}

int
socket_bind6_reuse(int s, char ip[16], uint16 port, uint32 scope_id)
{
	int             opt = 1;

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	return socket_bind6(s, ip, port, scope_id);
}
#endif
