/*
 * $Log: stralloc_eady.c,v $
 * Revision 1.5  2004-10-22 20:30:49+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-11 14:07:41+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.3  2004-10-09 23:33:51+05:30  Cprogrammer
 * prevent inclusion of function prototypes for stralloc functions
 *
 * Revision 1.2  2004-07-17 21:24:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "gen_allocdefs.h"

GEN_ALLOC_ready(stralloc, char, s, len, a, i, n, x, 30, stralloc_ready)
GEN_ALLOC_readyplus(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus)

void
getversion_stralloc_eady_c()
{
	static char    *x = "$Id: stralloc_eady.c,v 1.5 2004-10-22 20:30:49+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
