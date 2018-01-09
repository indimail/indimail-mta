/*
 * $Log: byte_rcspn.c,v $
 * Revision 1.2  2004-10-22 20:23:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-08-15 19:52:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

unsigned int
byte_rcspn(s, n, c)
	register char  *s;
	register unsigned int n;
	register char  *c;
{
	unsigned int    ret, pos, i;

	for (ret = n, pos = 0; *c; ++c)
	{
		if ((i = byte_rchr(s + pos, n - pos, *c) + pos) < n)
			ret = pos = i;
	}
	return ret;
}

void
getversion_byte_rcspn_c()
{
	static char    *x = "$Id: byte_rcspn.c,v 1.2 2004-10-22 20:23:01+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
