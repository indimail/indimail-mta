/*
 * $Log: socket_local.c,v $
 * Revision 1.3  2008-07-25 16:50:09+05:30  Cprogrammer
 * remove compilation warning
 *
 * Revision 1.2  2005-06-10 12:18:04+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef IPV6
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <byte.h>
#include "socket.h"

int
socket_local4(int s, char ip[4], uint16 * port)
{
	struct sockaddr_in sa;
	unsigned int    dummy = sizeof sa;

	if (getsockname(s, (struct sockaddr *) &sa, (socklen_t *) &dummy) == -1)
		return -1;
	byte_copy(ip, 4, (char *) &sa.sin_addr);
	uint16_unpack_big((char *) &sa.sin_port, port);
	return 0;
}
#endif

void
getversion_socket_local4_c()
{
	static char    *x = "$Id: socket_local.c,v 1.3 2008-07-25 16:50:09+05:30 Cprogrammer Stab mbhangui $";
	x++;
	return;
}
