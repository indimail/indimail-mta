/*
 * $Log: socket_delay.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket.h"

int
socket_tcpnodelay(int s)
{
	int             opt = 1;
	return setsockopt(s, IPPROTO_TCP, 1, &opt, sizeof opt);	/*- 1 == TCP_NODELAY */
}
