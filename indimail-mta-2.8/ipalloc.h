/*
 * $Log: ipalloc.h,v $
 * Revision 1.5  2005-06-11 21:30:03+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.4  2004-10-09 23:24:21+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-06-18 23:00:07+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef IPALLOC_H
#define IPALLOC_H

#include "ip.h"

struct ip_mx
{
	unsigned short af;
	union
	{
		struct ip_address ip;
#ifdef IPV6
		struct ip6_address ip6;
#endif
	} addr;
	int pref;
#ifdef TLS
	char           *fqdn;
#endif
};

#include "gen_alloc.h"

GEN_ALLOC_typedef(ipalloc, struct ip_mx, ix, len, a)
#ifndef _IPALLOC_
int             ipalloc_readyplus(ipalloc *, int);
int             ipalloc_append(ipalloc *, struct ip_mx *);
#else
int             ipalloc_readyplus();
int             ipalloc_append();
#endif
#endif /*- #ifndef IPALLOC_H */
