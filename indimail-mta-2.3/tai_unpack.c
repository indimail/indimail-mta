/*
 * $Log: tai_unpack.c,v $
 * Revision 1.2  2004-10-22 20:31:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-05-14 00:45:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "tai.h"

void
tai_unpack(s, t)
	char           *s;
	struct tai     *t;
{
	uint64          x;

	x = (unsigned char) s[0];
	x <<= 8;
	x += (unsigned char) s[1];
	x <<= 8;
	x += (unsigned char) s[2];
	x <<= 8;
	x += (unsigned char) s[3];
	x <<= 8;
	x += (unsigned char) s[4];
	x <<= 8;
	x += (unsigned char) s[5];
	x <<= 8;
	x += (unsigned char) s[6];
	x <<= 8;
	x += (unsigned char) s[7];
	t->x = x;
}

void
getversion_tai_unpack_c()
{
	static char    *x = "$Id: tai_unpack.c,v 1.2 2004-10-22 20:31:36+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
