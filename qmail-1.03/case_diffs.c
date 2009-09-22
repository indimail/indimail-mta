/*
 * $Log: case_diffs.c,v $
 * Revision 1.3  2004-10-22 20:23:16+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:06+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "case.h"

int
case_diffs(s, t)
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
		if (x != y)
			break;
		if (!x)
			break;
	}
	return ((int) (unsigned int) x) - ((int) (unsigned int) y);
}

void
getversion_case_diffs_c()
{
	static char    *x = "$Id: case_diffs.c,v 1.3 2004-10-22 20:23:16+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
