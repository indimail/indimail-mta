/*
 * $Log: case_starts.c,v $
 * Revision 1.3  2004-10-22 20:23:19+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:15+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "case.h"

int
case_starts(s, t)
	register char  *s;
	register char  *t;
{
	register unsigned char x;
	register unsigned char y;

	for (;;)
	{
		x = *s++ - 'A';
		if (x <= 'Z' - 'A')
			x += 'a';
		else
			x += 'A';
		y = *t++ - 'A';
		if (y <= 'Z' - 'A')
			y += 'a';
		else
			y += 'A';
		if (!y)
			return 1;
		if (x != y)
			return 0;
	}
}

void
getversion_case_starts_c()
{
	static char    *x = "$Id: case_starts.c,v 1.3 2004-10-22 20:23:19+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
