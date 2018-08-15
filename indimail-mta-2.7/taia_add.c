/*
 * $Log: taia_add.c,v $
 * Revision 1.2  2004-10-22 20:31:27+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:32:11+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Public domain. 
 */

#include "taia.h"

/*
 * XXX: breaks tai encapsulation 
 */

void
taia_add(struct taia *t, const struct taia *u, const struct taia *v)
{
	t->sec.x = u->sec.x + v->sec.x;
	t->nano = u->nano + v->nano;
	t->atto = u->atto + v->atto;
	if (t->atto > 999999999UL)
	{
		t->atto -= 1000000000UL;
		++t->nano;
	}
	if (t->nano > 999999999UL)
	{
		t->nano -= 1000000000UL;
		++t->sec.x;
	}
}

void
getversion_taia_add_c()
{
	static char    *x = "$Id: taia_add.c,v 1.2 2004-10-22 20:31:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
