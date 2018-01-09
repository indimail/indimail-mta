/*
 * $Log: case_diffb.c,v $
 * Revision 1.3  2004-10-22 20:23:15+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "case.h"

int
case_diffb(s, len, t)
	register char  *s;
	unsigned int    len;
	register char  *t;
{
	register unsigned char x;
	register unsigned char y;

	while (len > 0)
	{
		--len;
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
			return ((int) (unsigned int) x) - ((int) (unsigned int) y);
	}
	return 0;
}

void
getversion_case_diffb_c()
{
	static char    *x = "$Id: case_diffb.c,v 1.3 2004-10-22 20:23:15+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
