/*
 * $Log: str_diffn.c,v $
 * Revision 1.1  2003-12-27 17:16:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

int
str_diffn(s, t, len)
	register char  *s;
	register char  *t;
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
