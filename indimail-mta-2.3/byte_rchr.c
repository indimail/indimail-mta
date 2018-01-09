/*
 * $Log: byte_rchr.c,v $
 * Revision 1.3  2004-10-22 20:32:22+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:16:50+05:30  Cprogrammer
 * added RCS log
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

void
getversion_byte_rchr_c()
{
	static char    *x = "$Id: byte_rchr.c,v 1.3 2004-10-22 20:32:22+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
