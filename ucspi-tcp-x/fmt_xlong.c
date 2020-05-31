/*
 * $Log: fmt_xlong.c,v $
 * Revision 1.2  2013-08-06 00:49:29+05:30  Cprogrammer
 * added tohex() function
 *
 * Revision 1.1  2005-06-10 09:04:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fmt.h"
#include "hexconversion.h"

unsigned int
fmt_xlong(register char *s, register unsigned long u)
{
	register unsigned int len;
	register unsigned long q;

	len = 1;
	q = u;
	while (q > 15)
	{
		++len;
		q /= 16;
	}
	if (s)
	{
		s += len;
		do
		{
			*--s = tohex(u % 16);
			u /= 16;
		}
		while (u);				/*- handles u == 0 */
	}
	return len;
}
