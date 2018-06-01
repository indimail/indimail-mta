/*
 * $Log: open_trunc.c,v $
 * Revision 1.3  2004-10-22 20:27:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:20:05+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int
open_trunc(fn)
	char           *fn;
{
	return open(fn, O_WRONLY | O_NDELAY | O_TRUNC | O_CREAT, 0644);
}

void
getversion_open_trunc_c()
{
	static char    *x = "$Id: open_trunc.c,v 1.3 2004-10-22 20:27:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
