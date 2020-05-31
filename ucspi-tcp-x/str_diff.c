/*
 * $Log: str_diff.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

int
str_diff(register char *s, register char *t)
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
