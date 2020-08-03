/*
 * $Log: socket_remote.c,v $
 * Revision 1.2  2005-06-10 12:18:25+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <byte.h>
#include "socket.h"
#ifdef IPV6
#include "ip6.h"
#endif

#ifdef IPV6
extern int          noipv6;

int
socket_remote6(int s, char ip[16], uint16 *port, uint32 *scope_id)
{
#ifdef LIBC_HAS_IP6
	struct sockaddr_in6 sa;
#else
	struct sockaddr_in sa;
#endif
	unsigned int    dummy = sizeof sa;

	if (getpeername(s, (struct sockaddr *) &sa, &dummy) == -1)
		return -1;
#ifdef LIBC_HAS_IP6
	if (sa.sin6_family == AF_INET) {
		struct sockaddr_in *sa4 = (struct sockaddr_in *) &sa;
		byte_copy(ip, 12, (char *) V4mappedprefix);
		byte_copy(ip + 12, 4, (char *) &sa4->sin_addr);
		uint16_unpack_big((char *) &sa4->sin_port, port);
		return 0;
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
	return 0;
}
#else
int
socket_remote4(int s, char ip[4], uint16 * port)
{
	struct sockaddr_in sa;
	unsigned int    dummy = sizeof sa;

	if (getpeername(s, (struct sockaddr *) &sa, (socklen_t *) &dummy) == -1)
		return -1;
	byte_copy(ip, 4, (char *) &sa.sin_addr);
	uint16_unpack_big((char *) &sa.sin_port, port);
	return 0;
}
#endif
