/*
 * $Log: taia_add.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

/*
 * XXX: breaks tai encapsulation 
 */

void
taia_add(struct taia *t, struct taia *u, struct taia *v)
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
