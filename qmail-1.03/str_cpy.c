/*
 * $Log: str_cpy.c,v $
 * Revision 1.3  2004-10-22 20:30:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:13+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"

unsigned int
str_copy(s, t)
	register char  *s;
	register char  *t;
{
	register int    len;

	len = 0;
	for (;;)
	{
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
		if (!(*s = *t))
			return len;
		++s;
		++t;
		++len;
	}
}

void
getversion_str_cpy_c()
{
	static char    *x = "$Id: str_cpy.c,v 1.3 2004-10-22 20:30:55+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
