/*
 * $Log: taia_uint.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

/*- XXX: breaks tai encapsulation */

void
taia_uint(struct taia *t, unsigned int s)
{
	t->sec.x = s;
	t->nano = 0;
	t->atto = 0;
}
