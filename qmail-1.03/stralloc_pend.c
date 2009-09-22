/*
 * $Log: stralloc_pend.c,v $
 * Revision 1.3  2004-10-22 20:30:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:10+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "alloc.h"
#include "stralloc.h"
#include "gen_allocdefs.h"

GEN_ALLOC_append(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus, stralloc_append)

void
getversion_stralloc_pend_c()
{
	static char    *x = "$Id: stralloc_pend.c,v 1.3 2004-10-22 20:30:53+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
