/*
 * $Log: hexconversion.c,v $
 * Revision 1.1  2013-08-06 07:55:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hexconversion.h"

/**
 * Convert a number of max 255 to hex.
 * @param decimal The decimal number.
 * @param hex The converted hex value.
 */
void
bytetohex(unsigned char decimal, char hex[3])
{

	char           *hexdigits = "0123456789ABCDEF";
	int             rest, number;
	hex[0] = '0';
	hex[1] = '0';
	hex[2] = '\0';
	number = decimal / 16;
	rest = decimal % 16;
	hex[0] = hexdigits[number];
	hex[1] = hexdigits[rest];
}

char
tohex(char num)
{
	if (num < 10)
		return num + '0';
	else
	if (num < 16)
		return num - 10 + 'a';
	else
		return -1;
}

int
fromhex(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}
