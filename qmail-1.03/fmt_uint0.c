/*
 * $Log: fmt_uint0.c,v $
 * Revision 1.3  2004-10-22 20:25:20+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:18:55+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "fmt.h"

unsigned int
fmt_uint0(s, u, n)
	char           *s;
	unsigned int    u;
	unsigned int    n;
{
	unsigned int    len;
	len = fmt_uint(FMT_LEN, u);
	while (len < n)
	{
		if (s)
			*s++ = '0';
		++len;
	}
	if (s)
		fmt_uint(s, u);
	return len;
}

void
getversion_fmt_uint0_c()
{
	static char    *x = "$Id: fmt_uint0.c,v 1.3 2004-10-22 20:25:20+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
