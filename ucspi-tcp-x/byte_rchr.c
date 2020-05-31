/*
 * $Log: byte_rchr.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

unsigned int
byte_rchr(s, n, c)
	char           *s;
	register unsigned int n;
	int             c;
{
	register char   ch;
	register char  *t;
	register char  *u;

	ch = c;
	t = s;
	u = 0;
	for (;;)
	{
		if (!n)
			break;
		if (*t == ch)
			u = t;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			u = t;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			u = t;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			u = t;
		++t;
		--n;
	}
	if (!u)
		u = t;
	return u - s;
}
