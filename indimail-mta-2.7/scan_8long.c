/*
 * $Log: scan_8long.c,v $
 * Revision 1.3  2004-10-22 20:30:05+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-16 01:21:28+05:30  Cprogrammer
 * added scan_long and scan_plusminus()
 *
 */

#include "scan.h"

unsigned int
scan_8long(s, u)
	register char  *s;
	register unsigned long *u;
{
	register unsigned int pos;
	register unsigned long result;
	register unsigned long c;
	pos = 0;
	result = 0;
	while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 8)
	{
		result = result * 8 + c;
		++pos;
	}
	*u = result;
	return pos;
}


unsigned int
scan_long(s, i)
	register char  *s;
	register long  *i;
{
	int             sign;
	unsigned long   u;
	register unsigned int len;

	len = scan_plusminus(s, &sign);
	s += len;
	len += scan_ulong(s, &u);
	if (sign < 0)
		*i = -u;
	else
		*i = u;
	return len;
}

unsigned int
scan_plusminus(s, sign)
	register char  *s;
	register int   *sign;
{
	if (*s == '+')
	{
		*sign = 1;
		return 1;
	}
	if (*s == '-')
	{
		*sign = -1;
		return 1;
	}
	*sign = 1;
	return 0;
}

void
getversion_scan_8long_c()
{
	static char    *x = "$Id: scan_8long.c,v 1.3 2004-10-22 20:30:05+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
