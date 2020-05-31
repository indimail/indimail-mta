/*
 * $Log: socket_conn.c,v $
 * Revision 1.5  2008-07-25 16:50:04+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.4  2008-07-17 23:04:17+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.3  2007-06-10 10:16:53+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2005-06-10 12:17:19+05:30  Cprogrammer
 * removed unneeded header files
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"

int
socket_connect4(int s, char ip[4], uint16 port)
{
	struct sockaddr_in sa;

	byte_zero(&sa, sizeof sa);
	sa.sin_family = AF_INET;
	uint16_pack_big((char *) &sa.sin_port, port);
	byte_copy((char *) &sa.sin_addr, 4, ip);

	return connect(s, (struct sockaddr *) &sa, sizeof sa);
}

int
socket_connected(int s)
{
	struct sockaddr_in sa;
#if defined(__socklen_t_defined) || defined(_SOCKLEN_T)
	socklen_t       dummy;
#else
	int             dummy;
#endif
	char            ch;

	dummy = sizeof sa;
	if (getpeername(s, (struct sockaddr *) &sa, &dummy) == -1)
	{
		read(s, &ch, 1);		/*- sets errno */
		return 0;
	}
	return 1;
}
