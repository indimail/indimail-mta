/*
 * $Log: byte_zero.c,v $
 * Revision 1.3  2004-10-22 20:23:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:16:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"

void
byte_zero(s, n)
	char           *s;
	register unsigned int n;
{
	for (;;)
	{
		if (!n)
			break;
		*s++ = 0;
		--n;
		if (!n)
			break;
		*s++ = 0;
		--n;
		if (!n)
			break;
		*s++ = 0;
		--n;
		if (!n)
			break;
		*s++ = 0;
		--n;
	}
}

void
getversion_byte_zero_c()
{
	static char    *x = "$Id: byte_zero.c,v 1.3 2004-10-22 20:23:04+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
