/*
 * $Log: substdio_copy.c,v $
 * Revision 1.3  2004-10-22 20:31:10+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:38+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "substdio.h"

int
substdio_copy(ssout, ssin)
	register substdio *ssout;
	register substdio *ssin;
{
	register int    n;
	register char  *x;

	for (;;)
	{
		n = substdio_feed(ssin);
		if (n < 0)
			return -2;
		if (!n)
			return 0;
		x = substdio_PEEK(ssin);
		if (substdio_put(ssout, x, n) == -1)
			return -3;
		substdio_SEEK(ssin, n);
	}
}

void
getversion_substdio_copy_c()
{
	static char    *x = "$Id: substdio_copy.c,v 1.3 2004-10-22 20:31:10+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
