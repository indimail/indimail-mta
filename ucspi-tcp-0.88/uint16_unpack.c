/*
 * $Log: uint16_unpack.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "uint16.h"

void
uint16_unpack(char s[2], uint16 * u)
{
	uint16          result;

	result = (unsigned char) s[1];
	result <<= 8;
	result += (unsigned char) s[0];

	*u = result;
}

void
uint16_unpack_big(char s[2], uint16 * u)
{
	uint16          result;

	result = (unsigned char) s[0];
	result <<= 8;
	result += (unsigned char) s[1];

	*u = result;
}
