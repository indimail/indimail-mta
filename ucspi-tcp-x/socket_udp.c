/*
 * $Log: socket_udp.c,v $
 * Revision 1.2  2005-06-10 12:19:17+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket.h"
#ifdef IPV6
#include <errno.h>

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT EINVAL
#endif

extern int          noipv6;

int
socket_udp6(void)
{
#ifdef LIBC_HAS_IP6
	int             s;

	if (noipv6)
		goto compat;
	if ((s = socket(PF_INET6, SOCK_DGRAM, 0)) == -1)
	{
		if (errno == EINVAL || errno == EAFNOSUPPORT)
		{
compat:
			s = socket(AF_INET, SOCK_DGRAM, 0);
			noipv6 = 1;
			if (s == -1)
				return -1;
		} else
			return -1;
	}
	return s;
#else
	return socket_udp();
#endif
}
#else
#include "ndelay.h"
#include <unistd.h>

int
socket_udp(void)
{
	int             s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		return -1;
	if (ndelay_on(s) == -1)
	{
		close(s);
		return -1;
	}
	return s;
}

#endif
