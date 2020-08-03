/*
 * $Log: socket_getifidx.c,v $
 * Revision 1.3  2020-08-03 17:26:33+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.2  2005-06-10 12:17:33+05:30  Cprogrammer
 * added uint32.h
 *
 * Revision 1.1  2005-06-10 09:03:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <uint32.h>
#include "socket.h"

uint32
socket_getifidx(char *ifname)
{
	return if_nametoindex(ifname);
}
