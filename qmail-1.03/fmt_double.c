/*
 * $Log: fmt_double.c,v $
 * Revision 1.1  2013-08-29 18:26:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fmt.h"

unsigned int
fmt_double(register char *s, double d, unsigned long precision)
{
	unsigned int    len;
	double          p;
	unsigned long   q, r, t, u;

	len = 1;
	if (d < 0) {
		t = u = (unsigned long) (0.0 - d);
		if (s)
			*s = '-';
		len++;
	} else
		t = u = (unsigned long) d;
	q = u;
	while (q > 9) {
		++len;
		q /= 10;
	}
	if (s) {
		s += len;
		do {
			*--s = '0' + (u % 10);
			u /= 10;
		} while (u); /*- handles u == 0 */
	}
	/*---------------*/
	if (s) {
		s += (d < 0) ? len - 1 : len;
		*s++ = '.';
	}
	len++;
	if (d < 0) {
		p = -d -t;
	} else
		p = d - t;
	for (r = 0; r < (precision < 20 ? precision : FMT_DOUBLE - (FMT_ULONG + 3));r++) {
		p *= 10;
		if (s) {
			*s++ = '0' + ((unsigned long) p % 10);
		}
		len++;
	}
	return len;

}

void
getversion_fmt_double_c()
{
	static char    *x = "$Id: fmt_double.c,v 1.1 2013-08-29 18:26:46+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
