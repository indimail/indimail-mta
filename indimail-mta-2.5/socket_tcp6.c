/*
 * $Log: socket_tcp6.c,v $
 * Revision 1.3  2018-05-30 23:26:21+05:30  Cprogrammer
 * moved noipv6 variable to variables.c
 *
 * Revision 1.2  2005-06-15 22:32:24+05:30  Cprogrammer
 * added rcs version
 *
 * Revision 1.1  2005-06-15 22:12:24+05:30  Cprogrammer
 * Initial revision
 *
 * Revision 1.2  2005-06-10 12:18:57+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:02:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "socket.h"

extern int      noipv6;

int
socket_tcp6(void)
{
#ifdef LIBC_HAS_IP6
	int             s;

	if (noipv6)
		goto compat;
	if ((s = socket(PF_INET6, SOCK_STREAM, 0)) == -1)
	{
		if (errno == EINVAL || errno == EAFNOSUPPORT)
		{
compat:
			noipv6 = 1;
			if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				return -1;
		} else
			return -1;
	}
	return s;
#else
	return socket_tcp4();
#endif
}
#endif

void
getversion_socket_tcp6_c()
{
	static char    *x = "$Id: socket_tcp6.c,v 1.3 2018-05-30 23:26:21+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
