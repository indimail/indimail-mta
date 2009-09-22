/*
 * $Log: socket_getifname.c,v $
 * Revision 1.2  2005-06-10 12:17:40+05:30  Cprogrammer
 * added uint32.h
 *
 * Revision 1.1  2005-06-10 09:03:34+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "uint32.h"
#include "socket.h"

static char     ifname[IFNAMSIZ];

char     *
socket_getifname(uint32 interface)
{
	char           *tmp = if_indextoname(interface, ifname);

	if (tmp)
		return tmp;
	else
		return "[unknown]";
}
