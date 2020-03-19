/*
 * $Log: tlsarralloc.c,v $
 * Revision 1.2  2019-07-19 13:49:15+05:30  Cprogrammer
 * 2nd argument of tlsarralloc_readyplus() should be unsigned int
 *
 * Revision 1.1  2018-05-26 12:38:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "gen_allocdefs.h"
#include "tlsarralloc.h"

GEN_ALLOC_readyplus(tlsarralloc,struct tlsa_rdata,rr,len,a,i,n,x,10,tlsarralloc_readyplus)
GEN_ALLOC_append(tlsarralloc,struct tlsa_rdata,rr,len,a,i,n,x,10,tlsarralloc_readyplus,tlsarralloc_append)

void
getversion_tlsarralloc_c()
{
	static char    *x = "$Id: tlsarralloc.c,v 1.2 2019-07-19 13:49:15+05:30 Cprogrammer Exp mbhangui $";

	x++;
}