/*
 * $Log: uint32_unpack.c,v $
 * Revision 1.1  2008-09-15 22:11:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "uint32.h"

void
uint32_unpack(char s[4], uint32 * u)
{
	uint32          result;

	result = (unsigned char) s[3];
	result <<= 8;
	result += (unsigned char) s[2];
	result <<= 8;
	result += (unsigned char) s[1];
	result <<= 8;
	result += (unsigned char) s[0];

	*u = result;
}

void
uint32_unpack_big(char s[4], uint32 * u)
{
	uint32          result;

	result = (unsigned char) s[0];
	result <<= 8;
	result += (unsigned char) s[1];
	result <<= 8;
	result += (unsigned char) s[2];
	result <<= 8;
	result += (unsigned char) s[3];

	*u = result;
}

void
getversion_uint32_unpack_c()
{
	static char    *x = "$Id: uint32_unpack.c,v 1.1 2008-09-15 22:11:24+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
