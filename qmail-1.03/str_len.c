/*
 * $Log: str_len.c,v $
 * Revision 1.3  2004-10-22 20:30:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:22+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"

unsigned int
str_len(s)
	register char  *s;
{
	register char  *t;

	t = s;
	for (;;)
	{
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
	}
}

void
getversion_str_len_c()
{
	static char    *x = "$Id: str_len.c,v 1.3 2004-10-22 20:30:59+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
