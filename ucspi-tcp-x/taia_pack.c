/*
 * $Log: taia_pack.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "taia.h"

void
taia_pack(char *s, struct taia *t)
{
	unsigned long   x;

	tai_pack(s, &t->sec);
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
