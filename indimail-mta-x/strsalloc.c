/*
 * $Log: strsalloc.c,v $
 * Revision 1.5  2020-11-22 23:12:15+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.4  2020-05-10 17:47:16+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.3  2004-10-22 20:31:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-11 14:09:08+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.1  2004-08-15 19:52:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "gen_allocdefs.h"
#include "stralloc.h"
#include "strsalloc.h"

GEN_ALLOC_readyplus(strsalloc, stralloc, sa, len, a, 10, strsalloc_readyplus)
GEN_ALLOC_append(strsalloc, stralloc, sa, len, a, 10, strsalloc_readyplus, strsalloc_append)

void
getversion_strsalloc_c()
{
	static char    *x = "$Id: strsalloc.c,v 1.5 2020-11-22 23:12:15+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
