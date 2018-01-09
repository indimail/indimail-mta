/*
 * $Log: uint32_pack.c,v $
 * Revision 1.1  2008-09-15 22:11:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "uint32.h"

void
uint32_pack(char s[4], uint32 u)
{
	s[0] = u & 255;
	u >>= 8;
	s[1] = u & 255;
	u >>= 8;
	s[2] = u & 255;
	s[3] = u >> 8;
}

void
uint32_pack_big(char s[4], uint32 u)
{
	s[3] = u & 255;
	u >>= 8;
	s[2] = u & 255;
	u >>= 8;
	s[1] = u & 255;
	s[0] = u >> 8;
}

void
getversion_uint32_pack_c()
{
	static char    *x = "$Id: uint32_pack.c,v 1.1 2008-09-15 22:11:15+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
