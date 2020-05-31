/*
 * $Log: socket_tcp.c,v $
 * Revision 1.3  2007-06-10 10:17:25+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2005-06-10 12:19:07+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ndelay.h"

int
socket_tcp(void)
{
	int             s;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1)
		return -1;
	if (ndelay_on(s) == -1)
	{
		close(s);
		return -1;
	}
	return s;
}
