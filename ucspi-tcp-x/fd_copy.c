/*
 * $Log: fd_copy.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <fcntl.h>
#include "fd.h"
#include <unistd.h>

int
fd_copy(int to, int from)
{
	if (to == from)
		return 0;
	if (fcntl(from, F_GETFL, 0) == -1)
		return -1;
	close(to);
	if (fcntl(from, F_DUPFD, to) == -1)
		return -1;
	return 0;
}
