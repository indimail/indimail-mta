/*
 * $Log: str_start.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

int
str_start(register char *s, register char *t)
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
