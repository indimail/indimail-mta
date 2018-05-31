/*
 * $Log: open_excl.c,v $
 * Revision 1.3  2004-10-22 20:27:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:20:01+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int
open_excl(fn)
	char           *fn;
{
	return open(fn, O_WRONLY | O_EXCL | O_CREAT, 0644);
}

void
getversion_open_excl_c()
{
	static char    *x = "$Id: open_excl.c,v 1.3 2004-10-22 20:27:48+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
