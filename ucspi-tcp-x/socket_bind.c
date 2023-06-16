/*
 * $Log: socket_bind.c,v $
 * Revision 1.3  2020-08-03 17:26:17+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.2  2005-06-10 12:16:58+05:30  Cprogrammer
 * removed unneeded header files
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <byte.h>
#include "socket.h"

int
socket_bind4(int s, char ip[4], uint16 port)
{
	struct sockaddr_in sa;

	byte_zero((char *) &sa, sizeof sa);
	sa.sin_family = AF_INET;
	uint16_pack_big((char *) &sa.sin_port, port);
	byte_copy((char *) &sa.sin_addr, 4, ip);

	return bind(s, (struct sockaddr *) &sa, sizeof sa);
}

void
socket_tryreservein(int s, int size)
{
	while (size >= 1024) {
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) == 0)
			return;
		size -= (size >> 5);
	}
}
