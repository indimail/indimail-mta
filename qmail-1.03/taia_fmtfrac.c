/*
 * $Log: taia_fmtfrac.c,v $
 * Revision 1.1  2016-01-28 23:43:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

unsigned int
taia_fmtfrac(s, t)
	char           *s;
	const struct taia *t;
{
	unsigned long   x;

	if (s) {
		x = t->atto;
		s[17] = '0' + (x % 10);
		x /= 10;
		s[16] = '0' + (x % 10);
		x /= 10;
		s[15] = '0' + (x % 10);
		x /= 10;
		s[14] = '0' + (x % 10);
		x /= 10;
		s[13] = '0' + (x % 10);
		x /= 10;
		s[12] = '0' + (x % 10);
		x /= 10;
		s[11] = '0' + (x % 10);
		x /= 10;
		s[10] = '0' + (x % 10);
		x /= 10;
		s[9] = '0' + (x % 10);
		x = t->nano;
		s[8] = '0' + (x % 10);
		x /= 10;
		s[7] = '0' + (x % 10);
		x /= 10;
		s[6] = '0' + (x % 10);
		x /= 10;
		s[5] = '0' + (x % 10);
		x /= 10;
		s[4] = '0' + (x % 10);
		x /= 10;
		s[3] = '0' + (x % 10);
		x /= 10;
		s[2] = '0' + (x % 10);
		x /= 10;
		s[1] = '0' + (x % 10);
		x /= 10;
		s[0] = '0' + (x % 10);
	}
	return 18;
}

void
getversion_taia_fmtfrac_c()
{
	static char    *x = "$Id: taia_fmtfrac.c,v 1.1 2016-01-28 23:43:09+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
