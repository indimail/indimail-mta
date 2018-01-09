/*
 * $Log: scan_ulong.c,v $
 * Revision 1.5  2004-10-24 21:39:31+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.4  2004-10-22 20:30:07+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:22:41+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "scan.h"

unsigned int
scan_ulong(s, u)
	register char  *s;
	register unsigned long *u;
{
	register unsigned int pos;
	register unsigned long result;
	register unsigned long c;

	pos = 0;
	result = 0;
	while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 10)
	{
		result = result * 10 + c;
		++pos;
	}
	*u = result;
	return pos;
}

unsigned int
scan_int(s, i)
	register char  *s;
	register int   *i;
{
	register unsigned int pos;
	register int result;
	register unsigned char c;
	int             sign;

	pos = 0;
	result = 0;
	sign = 1;
	/*-
	 * determine sign of the number
	 */
	switch (s[0])
	{
		case '\0':
			return 0;
		case '-':
			++pos;
			sign = -1;
			break;
		case '+':
			++pos;
			sign = 1;
			break;
		default:
			break;
	}
	while ((c = (unsigned char)(s[pos] - '0')) < 10)
	{
		result = result * 10 + c;
		++pos;
	}
	*i = result * sign;
	return pos;
}

void
getversion_scan_ulong_c()
{
	static char    *x = "$Id: scan_ulong.c,v 1.5 2004-10-24 21:39:31+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
