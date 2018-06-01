/*
 * $Log: byte_chr.c,v $
 * Revision 1.3  2004-10-22 20:22:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:16:33+05:30  Cprogrammer
 * added RCS log
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

void
getversion_byte_chr_c()
{
	static char    *x = "$Id: byte_chr.c,v 1.3 2004-10-22 20:22:24+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
