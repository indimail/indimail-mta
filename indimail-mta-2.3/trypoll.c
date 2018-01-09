/*
 * $Log: trypoll.c,v $
 * Revision 1.2  2008-08-03 18:55:46+05:30  Cprogrammer
 * poll is broken on Darwin
 *
 * Revision 1.1  2008-08-03 18:45:20+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Public domain. 
 */

#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>

int
main()
{
	struct pollfd   x;

#ifdef DARWIN
	x /*- Prevent poll bug on Mac OS X */
#endif
	x.fd = open("trypoll.c", O_RDONLY);
	if (x.fd == -1)
		_exit(111);
	x.events = POLLIN;
	if (poll(&x, 1, 10) == -1)
		_exit(1);
	if (x.revents != POLLIN)
		_exit(1);

	/*
	 * XXX: try to detect and avoid poll() imitation libraries 
	 */

	_exit(0);
}
