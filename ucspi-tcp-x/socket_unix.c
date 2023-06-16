/*
 * $Log: $
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ndelay.h>

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
