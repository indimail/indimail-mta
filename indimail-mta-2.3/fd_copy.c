/*
 * $Log: fd_copy.c,v $
 * Revision 1.4  2004-10-22 20:25:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:18:43+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <fcntl.h>
#include <unistd.h>
#include "fd.h"

int
fd_copy(to, from)
	int             to;
	int             from;
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

void
getversion_fd_copy_c()
{
	static char    *x = "$Id: fd_copy.c,v 1.4 2004-10-22 20:25:02+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
