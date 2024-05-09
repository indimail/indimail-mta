/*
 * $Log: socket_tcp.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2005-06-15 22:35:58+05:30  Cprogrammer
 * added RCS version information
 *
 * Revision 1.1  2005-06-15 22:12:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
socket_tcp4(void)
{
	int             s;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	return s;
}

void
getversion_socket_tcp4_c()
{
	const char     *x = "$Id: socket_tcp.c,v 1.3 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
