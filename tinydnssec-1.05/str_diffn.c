/*
 * $Log: str_diffn.c,v $
 * Revision 1.3  2004-10-22 20:30:57+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"

int
str_diffn(s, t, len)
	const char  *s;
	const char  *t;
	unsigned int    len;
{
	register char   x;

	for (;;)
	{
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
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
