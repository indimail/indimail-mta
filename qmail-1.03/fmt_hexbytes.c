/*
 * $Log: fmt_hexbytes.c,v $
 * Revision 1.1  2015-08-24 19:03:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fmt.h"

unsigned int
fmt_hexbyte(char *s, unsigned char byte)
{
	static char     data[] = "0123456789abcdef";

	if (s)
	{
		*s++ = data[(byte >> 4) & 0xf];
		*s = data[byte & 0xf];
	}
	return 2;
}


void
getversion_fmt_hexbytes_c()
{
	static char    *x = "$Id: fmt_hexbytes.c,v 1.1 2015-08-24 19:03:00+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
