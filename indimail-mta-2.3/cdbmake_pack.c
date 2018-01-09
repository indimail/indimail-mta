/*
 * $Log: cdbmake_pack.c,v $
 * Revision 1.3  2004-10-22 20:23:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:25+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "cdbmake.h"

void
cdbmake_pack(buf, num)
	unsigned char  *buf;
	uint32          num;
{
	*buf++ = num;
	num >>= 8;
	*buf++ = num;
	num >>= 8;
	*buf++ = num;
	num >>= 8;
	*buf = num;
}

void
getversion_cdbmake_pack_c()
{
	static char    *x = "$Id: cdbmake_pack.c,v 1.3 2004-10-22 20:23:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
