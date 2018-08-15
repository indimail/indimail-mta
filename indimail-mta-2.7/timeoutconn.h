/*
 * $Log: timeoutconn.h,v $
 * Revision 1.4  2005-06-15 22:36:47+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.3  2004-10-11 14:15:22+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:11+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef TIMEOUTCONN_H
#define TIMEOUTCONN_H
#include "ip.h"

int             timeoutconn4(int, struct ip_address *, union v46addr *, unsigned int, int);
#ifdef IPV6
int             timeoutconn6(int, struct ip6_address *, union v46addr *, unsigned int, int);
#endif

#endif
