/*
 * $Log: taia_less.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

/*- XXX: breaks tai encapsulation */

int
taia_less(struct taia *t, struct taia *u)
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
