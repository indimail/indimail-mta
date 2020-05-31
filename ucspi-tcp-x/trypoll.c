/*
 * $Log: trypoll.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>

main()
{
	struct pollfd   x;

	x.fd = open("trypoll.c", O_RDONLY);
	if (x.fd == -1)
		_exit(111);
	x.events = POLLIN;
	if (poll(&x, 1, 10) == -1)
		_exit(1);
	if (x.revents != POLLIN)
		_exit(1);

	/*- XXX: try to detect and avoid poll() imitation libraries */
	_exit(0);
}
