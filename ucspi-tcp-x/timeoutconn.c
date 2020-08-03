/*
 * $Log: timeoutconn.c,v $
 * Revision 1.2  2005-06-10 12:19:59+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <ndelay.h>
#include <iopause.h>
#include <error.h>
#include "socket.h"
#include "timeoutconn.h"

int
#ifdef IPV6
timeoutconn6(int s, char ip[16], uint16 port, unsigned int timeout, uint32 netif)
#else
timeoutconn(int s, char ip[4], uint16 port, unsigned int timeout)
#endif
{
	struct taia     now;
	struct taia     deadline;
	iopause_fd      x;

#ifdef IPV6
	if (socket_connect6(s, ip, port, netif) == -1)
#else
	if (socket_connect4(s, ip, port) == -1)
#endif
	{
		if ((errno != error_wouldblock) && (errno != error_inprogress))
			return -1;
		x.fd = s;
		x.events = IOPAUSE_WRITE;
		taia_now(&now);
		taia_uint(&deadline, timeout);
		taia_add(&deadline, &now, &deadline);
		for (;;) {
			taia_now(&now);
			iopause(&x, 1, &deadline, &now);
			if (x.revents)
				break;
			if (taia_less(&deadline, &now)) {
				errno = error_timeout;	/*- note that connect attempt is continuing */
				return -1;
			}
		}
		if (!socket_connected(s))
			return -1;
	}

	if (ndelay_off(s) == -1)
		return -1;
	return 0;
}
