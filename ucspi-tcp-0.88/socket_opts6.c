/*
 * $Log: socket_opts6.c,v $
 * Revision 1.1  2013-08-06 07:55:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket.h"

int
socket_ip6optionskill(int s)
{
	return setsockopt(s, IPPROTO_IPV6, 1, (char *) 0, 0);
}
