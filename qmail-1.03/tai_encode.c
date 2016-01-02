/*-
 * $Log: tai_encode.c,v $
 * Revision 1.1  2016-01-02 19:21:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "tai2.h"

int
tai_encode(tai * t, char *buf)
{
	unsigned long   s = t->seconds;
	unsigned long   n = t->nanoseconds;
	int             i;

	for (i = 9; i >= 0; i--) {
		buf[i] = '0' + s % 10;
		s /= 10;
	}
	buf[10] = '.';
	for (i = 19; i >= 11; i--) {
		buf[i] = '0' + n % 10;
		n /= 10;
	}
	buf[20] = 0;
	return 0;
}

void
getversion_tai_encode_c()
{
	static char    *x = "$Id: tai_encode.c,v 1.1 2016-01-02 19:21:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
