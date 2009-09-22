/*
 * $Log: taia_less.c,v $
 * Revision 1.2  2004-10-22 20:31:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:32:15+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Public domain. 
 */

#include "taia.h"

/*
 * XXX: breaks tai encapsulation 
 */

int
taia_less(const struct taia *t, const struct taia *u)
{
	if (t->sec.x < u->sec.x)
		return 1;
	if (t->sec.x > u->sec.x)
		return 0;
	if (t->nano < u->nano)
		return 1;
	if (t->nano > u->nano)
		return 0;
	return t->atto < u->atto;
}

void
getversion_taia_less_c()
{
	static char    *x = "$Id: taia_less.c,v 1.2 2004-10-22 20:31:29+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
