/*
 * $Log: str_end.c,v $
 * Revision 1.1  2016-02-08 18:31:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

int
str_end(s, t)
	register char  *s;
	register char  *t;
{
	register char   x, y, *z;

	for (y = *(z = t);;) {
		x = *s;
		y = *t;
		if (!x)
			break;
		if (!y)
			y = *(t = z);
		if (x != y) {
			s++;
			t = z;
			continue;
		} else {
			s++;
			t++;
		}
		if (!x)
			break;

		x = *s;
		y = *t;
		if (!x)
			break;
		if (!y)
			y = *(t = z);
		if (x != y) {
			s++;
			t = z;
			continue;
		} else {
			s++;
			t++;
		}
		if (!x)
			break;

		x = *s;
		y = *t;
		if (!x)
			break;
		if (!y)
			y = *(t = z);
		if (x != y) {
			s++;
			t = z;
			continue;
		} else {
			s++;
			t++;
		}
		if (!x)
			break;
	}
	return ((int) (unsigned int) (unsigned char) x) - ((int) (unsigned int) (unsigned char) *t);
}

void
getversion_str_end_c()
{
	static char    *x = "$Id: str_end.c,v 1.1 2016-02-08 18:31:17+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
