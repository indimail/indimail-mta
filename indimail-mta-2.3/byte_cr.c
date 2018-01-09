/*
 * $Log: byte_cr.c,v $
 * Revision 1.3  2004-10-22 20:22:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:16:39+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"

void
byte_copyr(to, n, from)
	register char  *to;
	register unsigned int n;
	register char  *from;
{
	to += n;
	from += n;
	for (;;)
	{
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
	}
}

void
getversion_byte_cr_c()
{
	static char    *x = "$Id: byte_cr.c,v 1.3 2004-10-22 20:22:29+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
