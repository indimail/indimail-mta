/*
 * $Log: socket_opts.c,v $
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
socket_ipoptionskill(int s)
{
	return setsockopt(s, IPPROTO_IP, 1, (char *) 0, 0);	/*- 1 == IP_OPTIONS */
}
