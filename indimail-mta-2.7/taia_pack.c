/*
 * $Log: taia_pack.c,v $
 * Revision 1.2  2004-10-22 20:31:31+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-19 18:53:41+05:30  Cprogrammer
 * Initial revision
 *
 */

/*- Public domain.  */
#include "taia.h"
void
taia_pack(char *s, const struct taia *t)
{
	unsigned long   x;

	tai_pack(s, (struct tai *) &t->sec);
	s += 8;

	x = t->atto;
	s[7] = x & 255;
	x >>= 8;
	s[6] = x & 255;
	x >>= 8;
	s[5] = x & 255;
	x >>= 8;
	s[4] = x;
	x = t->nano;
	s[3] = x & 255;
	x >>= 8;
	s[2] = x & 255;
	x >>= 8;
	s[1] = x & 255;
	x >>= 8;
	s[0] = x;
}

void
getversion_taia_pack_c()
{
	static char    *x = "$Id: taia_pack.c,v 1.2 2004-10-22 20:31:31+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
