/*
 * $Log: case_lowerb.c,v $
 * Revision 1.3  2004-10-22 20:23:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:08+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "case.h"

void
case_lowerb(s, len)
	char           *s;
	unsigned int    len;
{
	unsigned char   x;
	while (len > 0)
	{
		--len;
		x = *s - 'A';
		if (x <= 'Z' - 'A')
			*s = x + 'a';
		++s;
	}
}

void
getversion_case_lowerb_c()
{
	static char    *x = "$Id: case_lowerb.c,v 1.3 2004-10-22 20:23:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
