/*
 * $Log: byte_copy.c,v $
 * Revision 1.3  2004-10-22 20:22:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:16:36+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"

void
byte_copy(to, n, from)
	register char  *to;
	register unsigned int n;
	register char  *from;
{
	for (;;)
	{
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
	}
}

void
getversion_byte_copy_c()
{
	static char    *x = "$Id: byte_copy.c,v 1.3 2004-10-22 20:22:26+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
