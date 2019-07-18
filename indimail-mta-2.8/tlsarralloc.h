/*
 * $Log: tlsarralloc.h,v $
 * Revision 1.1  2018-05-26 12:38:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TLSARRALLOC_H
#define TLSARRALLOC_H
#include <stdint.h>

typedef struct tlsa_rdata {
	uint8_t         usage;
	uint8_t         selector;
	uint8_t         mtype;
	unsigned long   data_len;
	uint8_t        *data;
	char           *host;
	unsigned long   hostlen;
	uint32_t        ttl;
	struct tlsa_rdata *next;
} tlsarr;

#include "gen_alloc.h"

GEN_ALLOC_typedef(tlsarralloc,tlsarr,rr,len,a)
#ifndef _TLSARRALLOC_
int             tlsarralloc_readyplus(tlsarralloc *, int);
int             tlsarralloc_append(tlsarralloc *, tlsarr *);
#else
int             tlsarralloc_readyplus();
int             tlsarralloc_append();
#endif

#endif /*- TLSARRALLOC_H */
