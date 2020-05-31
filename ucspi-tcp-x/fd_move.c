/*
 * $Log: fd_move.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fd.h"
#include <unistd.h>

int
fd_move(int to, int from)
{
	if (to == from)
		return 0;
	if (fd_copy(to, from) == -1)
		return -1;
	close(from);
	return 0;
}
