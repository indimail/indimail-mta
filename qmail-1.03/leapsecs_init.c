/*
 * $Log: leapsecs_init.c,v $
 * Revision 1.2  2004-10-22 20:26:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-05-14 00:44:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "leapsecs.h"

static int      flaginit = 0;

int
leapsecs_init()
{
	if (flaginit)
		return 0;
	if (leapsecs_read() == -1)
		return -1;
	flaginit = 1;
	return 0;
}

void
getversion_leapsecs_init_c()
{
	static char    *x = "$Id: leapsecs_init.c,v 1.2 2004-10-22 20:26:03+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
