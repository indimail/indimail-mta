/*
 * $Log: byte_copy.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
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
