/*
 * $Log: byte_cspn.c,v $
 * Revision 1.2  2004-10-22 20:22:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-08-15 19:52:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

unsigned int
byte_cspn(s, n, c)
	register char  *s;
	register unsigned int n;
	register char  *c;
{
	while (*c)
		n = byte_chr(s, n, *c++);
	return n;
}

void
getversion_byte_cspn_c()
{
	static char    *x = "$Id: byte_cspn.c,v 1.2 2004-10-22 20:22:32+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
