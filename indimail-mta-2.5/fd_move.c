/*
 * $Log: fd_move.c,v $
 * Revision 1.4  2004-10-22 20:25:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:18:46+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "fd.h"
#include <unistd.h>

int
fd_move(to, from)
	int             to;
	int             from;
{
	if (to == from)
		return 0;
	if (fd_copy(to, from) == -1)
		return -1;
	close(from);
	return 0;
}

void
getversion_fd_move_c()
{
	static char    *x = "$Id: fd_move.c,v 1.4 2004-10-22 20:25:03+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
