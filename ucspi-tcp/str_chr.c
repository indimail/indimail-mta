/*
 * $Log: str_chr.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

unsigned int
str_chr(register char *s, int c)
{
	register char   ch;
	register char  *t;

	ch = c;
	t = s;
	for (;;)
	{
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
	}
	return t - s;
}
