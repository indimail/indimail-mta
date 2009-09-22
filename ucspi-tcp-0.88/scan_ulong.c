/*
 * $Log: scan_ulong.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "scan.h"

unsigned int
scan_ulong(register char *s, register unsigned long *u)
{
	register unsigned int pos = 0;
	register unsigned long result = 0;
	register unsigned long c;
	while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 10)
	{
		result = result * 10 + c;
		++pos;
	}
	*u = result;
	return pos;
}
