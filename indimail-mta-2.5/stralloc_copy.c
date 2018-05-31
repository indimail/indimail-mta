/*
 * $Log: stralloc_copy.c,v $
 * Revision 1.3  2004-10-22 20:30:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:02+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"
#include "stralloc.h"

int
stralloc_copy(sato, safrom)
	stralloc       *sato;
	stralloc       *safrom;
{
	return stralloc_copyb(sato, safrom->s, safrom->len);
}

void
getversion_stralloc_copy_c()
{
	static char    *x = "$Id: stralloc_copy.c,v 1.3 2004-10-22 20:30:48+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
