/*
 * $Log: open_append.c,v $
 * Revision 1.3  2004-10-22 20:27:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:20:00+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int
open_append(fn)
	char           *fn;
{
	return open(fn, O_WRONLY | O_NDELAY | O_APPEND | O_CREAT, 0600);
}

void
getversion_open_append_c()
{
	static char    *x = "$Id: open_append.c,v 1.3 2004-10-22 20:27:47+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
