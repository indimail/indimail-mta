/*
 * $Log: byte_cr.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
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
