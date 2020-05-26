/*
 * $Log: socket_bind.c,v $
 * Revision 1.2  2005-06-10 12:16:58+05:30  Cprogrammer
 * removed unneeded header files
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"

int
socket_bind4(int s, char ip[4], uint16 port)
{
	struct sockaddr_in sa;

	byte_zero(&sa, sizeof sa);
	sa.sin_family = AF_INET;
	uint16_pack_big((char *) &sa.sin_port, port);
	byte_copy((char *) &sa.sin_addr, 4, ip);

	return bind(s, (struct sockaddr *) &sa, sizeof sa);
}

int
socket_bind4_reuse(int s, char ip[4], uint16 port)
{
	int             opt = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	return socket_bind4(s, ip, port);
}

void
socket_tryreservein(int s, int size)
{
	while (size >= 1024)
	{
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof size) == 0)
			return;
		size -= (size >> 5);
	}
}
