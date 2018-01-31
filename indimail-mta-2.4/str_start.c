/*
 * $Log: str_start.c,v $
 * Revision 1.3  2004-10-22 20:31:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:25+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"

int
str_start(s, t)
	register char  *s;
	register char  *t;
{
	register char   x;

	for (;;)
	{
		x = *t++;
		if (!x)
			return 1;
		if (x != *s++)
			return 0;
		x = *t++;
		if (!x)
			return 1;
		if (x != *s++)
			return 0;
		x = *t++;
		if (!x)
			return 1;
		if (x != *s++)
			return 0;
		x = *t++;
		if (!x)
			return 1;
		if (x != *s++)
			return 0;
	}
}

void
getversion_str_start_c()
{
	static char    *x = "$Id: str_start.c,v 1.3 2004-10-22 20:31:02+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
