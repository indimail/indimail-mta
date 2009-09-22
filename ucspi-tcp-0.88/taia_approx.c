/*
 * $Log: taia_approx.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

double
taia_approx(struct taia *t)
{
	return tai_approx(&t->sec) + taia_frac(t);
}
