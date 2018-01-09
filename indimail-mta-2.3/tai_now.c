/*
 * $Log: tai_now.c,v $
 * Revision 1.2  2004-10-22 20:31:33+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-05-14 00:45:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <time.h>
#include "tai.h"

void
tai_now(t)
	struct tai     *t;
{
	t->x = 4611686018427387914ULL + (uint64) time((long *) 0);
}

void
getversion_tai_now_c()
{
	static char    *x = "$Id: tai_now.c,v 1.2 2004-10-22 20:31:33+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
