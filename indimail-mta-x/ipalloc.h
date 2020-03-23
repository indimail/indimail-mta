/*
 * $Log: ipalloc.h,v $
 * Revision 1.6  2019-07-19 13:48:18+05:30  Cprogrammer
 * 2nd argument of ipalloc_readyplus() should be unsigned int
 *
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
int             ipalloc_readyplus(ipalloc *, unsigned int);
int             ipalloc_append(ipalloc *, struct ip_mx *);
#endif /*- #ifndef IPALLOC_H */
