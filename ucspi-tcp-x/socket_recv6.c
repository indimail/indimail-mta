/*
 * $Log: socket_recv6.c,v $
 * Revision 1.3  2010-04-06 08:59:54+05:30  Cprogrammer
 * minor change
 *
 * Revision 1.2  2005-06-10 12:18:15+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:02:06+05:30  Cprogrammer
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

extern int          noipv6;

int
socket_recv6(int s, char *buf, unsigned int len, char ip[16], uint16 *port, uint32 *scope_id)
{
#ifdef LIBC_HAS_IP6
	struct sockaddr_in6 sa;
#else
	struct sockaddr_in sa;
#endif
	unsigned int    dummy = sizeof sa;
	int             r;

	byte_zero((char *) &sa, dummy);
	if (-1 == (r = recvfrom(s, buf, len, 0, (struct sockaddr *) &sa, &dummy)))
		return -1;
#ifdef LIBC_HAS_IP6
	if (noipv6) {
		struct sockaddr_in *sa4 = (struct sockaddr_in *) &sa;
		byte_copy(ip, 12, (char *) V4mappedprefix);
		byte_copy(ip + 12, 4, (char *) &sa4->sin_addr);
		uint16_unpack_big((char *) &sa4->sin_port, port);
		return r;
	}
	byte_copy(ip, 16, (char *) &sa.sin6_addr);
	uint16_unpack_big((char *) &sa.sin6_port, port);
	if (scope_id)
		*scope_id = sa.sin6_scope_id;
#else
	byte_copy(ip, 12, (char *) V4mappedprefix);
	byte_copy(ip + 12, 4, (char *) &sa.sin_addr);
	uint16_unpack_big((char *) &sa.sin_port, port);
	if (scope_id)
		*scope_id = 0;
#endif
	return r;
}
#endif
