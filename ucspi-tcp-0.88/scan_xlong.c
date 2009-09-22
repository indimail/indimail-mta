/*
 * $Log: scan_xlong.c,v $
 * Revision 1.1  2005-06-10 09:04:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "scan.h"

static int
fromhex(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

unsigned int
scan_xlong(char *src, unsigned long *dest)
{
	register const char *tmp = src;
	register int    l = 0;
	register unsigned char c;
	while ((c = fromhex(*tmp)) < 16)
	{
		l = (l << 4) + c;
		++tmp;
	}
	*dest = l;
	return tmp - src;
}
