/*
 * $Log: taia_sub.c,v $
 * Revision 1.2  2004-10-22 20:31:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:32:19+05:30  Cprogrammer
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
taia_sub(struct taia *t, const struct taia *u, const struct taia *v)
{
	unsigned long   unano = u->nano;
	unsigned long   uatto = u->atto;

	t->sec.x = u->sec.x - v->sec.x;
	t->nano = unano - v->nano;
	t->atto = uatto - v->atto;
	if (t->atto > uatto)
	{
		t->atto += 1000000000UL;
		--t->nano;
	}
	if (t->nano > unano)
	{
		t->nano += 1000000000UL;
		--t->sec.x;
	}
}

void
getversion_taia_sub_c()
{
	static char    *x = "$Id: taia_sub.c,v 1.2 2004-10-22 20:31:32+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
