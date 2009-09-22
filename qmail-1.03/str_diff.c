/*
 * $Log: str_diff.c,v $
 * Revision 1.3  2004-10-22 20:30:56+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:16+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"

int
str_diff(s, t)
	register char  *s;
	register char  *t;
{
	register char   x;

	for (;;)
	{
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
	}
	return ((int) (unsigned int) (unsigned char) x) - ((int) (unsigned int) (unsigned char) *t);
}

void
getversion_str_diff_c()
{
	static char    *x = "$Id: str_diff.c,v 1.3 2004-10-22 20:30:56+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
