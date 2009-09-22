/*
 * $Log: case_diffb.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "case.h"

int
case_diffb(register char *s, register unsigned int len, register char *t)
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
