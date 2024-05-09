/*
 * $Log: socket_unix.c,v $
 * Revision 1.1  2023-06-16 23:46:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ndelay.h>
#include "socket.h"

int
socket_unix(void)
{
	int             s;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;
	if (ndelay_on(s) == -1) {
		close(s);
		return -1;
	}
	return s;
}
