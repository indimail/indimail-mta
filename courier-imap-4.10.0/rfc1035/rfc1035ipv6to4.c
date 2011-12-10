/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"rfc1035.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<arpa/inet.h>


#if RFC1035_FREEBSD40
#define	s6_addr16	__u6_addr.__u6_addr16
#define s6_addr32	__u6_addr.__u6_addr32
#endif

#if RFC1035_SOLARIS8
#define s6_addr32	_S6_un._S6_u32
#endif

#if RFC1035_IPV6

void rfc1035_ipv6to4(struct in_addr *ip4, const struct in6_addr *ip6)
{
	ip4->s_addr=ip6->s6_addr32[3];
}

void rfc1035_ipv4to6(struct in6_addr *ip6, const struct in_addr *ip4)
{
	memset(ip6, 0, sizeof(*ip6));

#if RFC1035_SOLARIS8

	/* No 16-bit union <grumble>... */

	ip6->_S6_un._S6_u8[10]= ~0;
	ip6->_S6_un._S6_u8[11]= ~0;

#else
	ip6->s6_addr16[5]= ~0;
#endif

	ip6->s6_addr32[3]= ip4->s_addr;

	if (ip4->s_addr == INADDR_ANY)
		*ip6= in6addr_any;
}
#endif

