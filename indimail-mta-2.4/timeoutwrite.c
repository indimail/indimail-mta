/*
 * $Log: timeoutwrite.c,v $
 * Revision 1.4  2004-10-22 20:31:46+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:40:04+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:54+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "timeoutwrite.h"
#include "select.h"
#include "error.h"

int
timeoutwrite(t, fd, buf, len)
	int             t;
	int             fd;
	char           *buf;
	int             len;
{
	fd_set          wfds;
	struct timeval  tv;

	tv.tv_sec = t;
	tv.tv_usec = 0;

	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);

	if (select(fd + 1, (fd_set *) 0, &wfds, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(fd, &wfds))
		return write(fd, buf, len);

	errno = error_timeout;
	return -1;
}

void
getversion_timeoutwrite_c()
{
	static char    *x = "$Id: timeoutwrite.c,v 1.4 2004-10-22 20:31:46+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
