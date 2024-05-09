/*
 * $Log: timeoutconn_un.c,v $
 * Revision 1.1  2023-06-16 23:47:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <ndelay.h>
#include <iopause.h>
#include <error.h>
#include "socket.h"
#include "timeoutconn.h"

int
timeoutconn_un(int s, const char *socket, unsigned int timeout)
{
	struct taia     now;
	struct taia     deadline;
	iopause_fd      x;

	if (socket_connect_un(s, socket) == -1) {
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

