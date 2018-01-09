/*
 * $Log: coe.c,v $
 * Revision 1.3  2004-10-22 20:32:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:39+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <fcntl.h>
#include "coe.h"

int
coe(fd)
	int             fd;
{
	return fcntl(fd, F_SETFD, 1);
}

void
getversion_coe_c()
{
	static char    *x = "$Id: coe.c,v 1.3 2004-10-22 20:32:29+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
