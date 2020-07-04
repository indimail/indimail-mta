/*
 * $Log: remoteinfo.h,v $
 * Revision 1.5  2020-07-04 21:43:38+05:30  Cprogrammer
 * removed usage of INET6 define
 *
 * Revision 1.4  2005-06-15 22:35:38+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.3  2004-10-11 14:01:25+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:42+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef REMOTEINFO_H
#define REMOTEINFO_H

union sockunion
{
	struct sockaddr     sa;
	struct sockaddr_in  sa4;
#ifdef IPV6
	struct sockaddr_in6 sa6;
#endif
};

char           *remoteinfo_get(union sockunion *, union sockunion *, int);

#endif
