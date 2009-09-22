/*
 * $Log: ipme.h,v $
 * Revision 1.4  2005-06-15 22:34:28+05:30  Cprogrammer
 * added prototype for ipme_is6()
 *
 * Revision 1.3  2004-10-11 13:54:49+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:00:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef IPME_H
#define IPME_H

#include "ip.h"
#include "ipalloc.h"

extern ipalloc  ipme;

int             ipme_init(void);
int             ipme_is(struct ip_address *);
#ifdef IPV6
int             ipme_is6(struct ip6_address *);
#endif

#endif
