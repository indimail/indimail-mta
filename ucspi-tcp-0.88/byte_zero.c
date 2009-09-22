/*
 * $Log: byte_zero.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
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
