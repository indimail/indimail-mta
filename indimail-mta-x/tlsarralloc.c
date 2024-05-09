/*
 * $Log: tlsarralloc.c,v $
 * Revision 1.4  2020-11-22 23:12:18+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.3  2020-05-10 17:47:21+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.2  2019-07-19 13:49:15+05:30  Cprogrammer
 * 2nd argument of tlsarralloc_readyplus() should be unsigned int
 *
 * Revision 1.1  2018-05-26 12:38:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "gen_allocdefs.h"
#include "tlsarralloc.h"

GEN_ALLOC_readyplus(tlsarralloc,struct tlsa_rdata,rr,len,a,10,tlsarralloc_readyplus)
GEN_ALLOC_append(tlsarralloc,struct tlsa_rdata,rr,len,a,10,tlsarralloc_readyplus,tlsarralloc_append)

void
getversion_tlsarralloc_c()
{
	const char     *x = "$Id: tlsarralloc.c,v 1.4 2020-11-22 23:12:18+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
