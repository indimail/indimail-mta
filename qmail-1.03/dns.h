#include <arpa/nameser_compat.h>
/*
 * $Log: dns.h1,v $
 * Revision 1.8  2005-06-15 20:28:08+05:30  Cprogrammer
 * ipv6 implementation
 *
 * Revision 1.7  2005-06-11 21:29:08+05:30  Cprogrammer
 * added dns_ptr6() for ipv6 support
 *
 * Revision 1.6  2004-10-20 20:02:21+05:30  Cprogrammer
 * different prototypes of dns_ptr for spf and nonspf code
 *
 * Revision 1.5  2004-10-11 13:52:33+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.4  2004-08-14 02:18:06+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.3  2004-06-18 22:58:25+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef DNS_H
#define DNS_H

#define DNS_SOFT -1
#define DNS_HARD -2
#define DNS_MEM  -3
#include "ipalloc.h"
#include "stralloc.h"
#include "strsalloc.h"

void            dns_init(int);
int             dns_cname(stralloc *);
int             dns_mxip(ipalloc *, stralloc *, unsigned long);
int             dns_ip(ipalloc *, stralloc *);
#ifdef USE_SPF
int             dns_ptr(strsalloc *, struct ip_address *);
#else
int             dns_ptr(stralloc *, struct ip_address *);
#endif
int             dns_txt(strsalloc *, stralloc *);

#ifdef IPV6
#ifdef USE_SPF
int             dns_ptr6(strsalloc *, struct ip6_address *);
#else
int             dns_ptr6(stralloc *, struct ip6_address *);
#endif /*- USE_SPF */
int             dns_maps(stralloc *, struct ip6_address *, char *);
#else
int             dns_maps(stralloc *, struct ip_address *, char *);
#endif

#endif
