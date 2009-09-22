/*
 * $Log: cdb_hash.c,v $
 * Revision 1.4  2008-09-16 08:24:30+05:30  Cprogrammer
 * added cdb_hashadd()
 *
 * Revision 1.3  2004-10-22 20:23:20+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:17:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "cdb.h"

uint32
cdb_hashadd(uint32 h, unsigned char c)
{
	h += (h << 5);
	return h ^ c;
}

uint32
cdb_hash(buf, len)
	unsigned char  *buf;
	unsigned int    len;
{
	uint32          h;

	h = 5381;
	while (len)
	{
		--len;
		h += (h << 5);
		h ^= (uint32) * buf++;
	}
	return h;
}

void
getversion_cdb_hash_c()
{
	static char    *x = "$Id: cdb_hash.c,v 1.4 2008-09-16 08:24:30+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
