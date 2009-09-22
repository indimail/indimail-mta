/*
 * $Log: timeoutread.c,v $
 * Revision 1.4  2004-10-22 20:31:45+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:40:02+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:52+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "timeoutread.h"
#include "select.h"
#include "error.h"

int
timeoutread(t, fd, buf, len)
	int             t;
	int             fd;
	char           *buf;
	int             len;
{
	fd_set          rfds;
	struct timeval  tv;

	tv.tv_sec = t;
	tv.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(fd, &rfds))
		return read(fd, buf, len);

	errno = error_timeout;
	return -1;
}

void
getversion_timeoutread_c()
{
	static char    *x = "$Id: timeoutread.c,v 1.4 2004-10-22 20:31:45+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
