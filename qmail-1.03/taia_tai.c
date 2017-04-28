/*
 * $Log: taia_tai.c,v $
 * Revision 1.1  2016-01-28 23:43:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

void
taia_tai(ta, t)
	const struct taia *ta;
	struct tai     *t;
{
	*t = ta->sec;
}

void
getversion_taia_tai_c()
{
	static char    *x = "$Id: taia_tai.c,v 1.1 2016-01-28 23:43:18+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
