/*
 * $Log: taia_frac.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

double
taia_frac(struct taia *t)
{
	return (t->atto * 0.000000001 + t->nano) * 0.000000001;
}
