/*
 * $Log: socket_tcp6.c,v $
 * Revision 1.3  2007-06-10 10:17:00+05:30  Cprogrammer
 * fixed compilation warning
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
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <ndelay.h>
#include "socket.h"

#ifdef LIBC_HAS_IP6
int             noipv6 = 0;
#else
int             noipv6 = 1;
#endif

int
socket_tcp6(void)
{
#ifdef LIBC_HAS_IP6
	int             s;

	if (noipv6)
		goto compat;
	if ((s = socket(PF_INET6, SOCK_STREAM, 0)) == -1) {
		if (errno == EINVAL || errno == EAFNOSUPPORT) {
compat:
			s = socket(AF_INET, SOCK_STREAM, 0);
			noipv6 = 1;
			if (s == -1)
				return -1;
		} else
			return -1;
	}
	if (ndelay_on(s) == -1) {
		close(s);
		return -1;
	}
	return s;
#else
	return socket_tcp();
#endif
}
#endif
