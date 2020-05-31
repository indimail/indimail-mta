/*
 * $Log: byte_chr.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

unsigned int
byte_chr(s, n, c)
	char           *s;
	register unsigned int n;
	int             c;
{
	register char   ch;
	register char  *t;

	ch = c;
	t = s;
	for (;;)
	{
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
	}
	return t - s;
}
