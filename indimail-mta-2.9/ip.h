/*
 * $Log: ip.h,v $
 * Revision 1.10  2015-08-27 00:29:09+05:30  Cprogrammer
 * added ip6_fmt_flat(), ip6_fmt_exp() functions
 *
 * Revision 1.9  2015-08-24 19:06:36+05:30  Cprogrammer
 * use compressed ip6 address
 *
 * Revision 1.8  2005-08-23 17:30:55+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.7  2005-06-17 12:01:31+05:30  Cprogrammer
 * added typedefs for ip_address, ip6_address
 *
 * Revision 1.6  2005-06-15 22:57:25+05:30  Cprogrammer
 * made ip4_scan() visible for both v4 and v6
 *
 * Revision 1.5  2005-06-15 22:37:41+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.4  2005-06-11 21:30:24+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.3  2004-10-11 13:54:37+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:00:13+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef IP_H
#define IP_H

#ifdef IPV6
#define IPFMT 72
#define HOSTNAMELEN	1025
#else
#define IPFMT 19
#endif
extern char     ip4loopback[4]; /* = {127,0,0,1}; */

struct ip_address
{
	unsigned char   d[4];
};
typedef struct  ip_address ip_addr;
extern int      noipv6;

#ifdef IPV6
/*
 * ip6 address syntax: (h = hex digit), no leading '0' required
 * 1. hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 * 2. any number of 0000 may be abbreviated as "::", but only once
 * 3. flat ip6 address syntax: hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
 */
struct ip6_address
{
	unsigned char d[16];
};    /* 16 hex pieces */
typedef struct  ip6_address ip6_addr;

extern unsigned char V4mappedprefix[12]; /*- ={0,0,0,0,0,0,0,0,0,0,0xff,0xff}; */
extern unsigned char V6loopback[16];	 /*- ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}; */
extern unsigned char V6any[16];			 /*- ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; */
#define ip6_isv4mapped(ip) (byte_equal((char *) ip, 12, (char *) V4mappedprefix))
#endif

union v46addr
{
	struct ip_address ip;
#ifdef IPV6
	struct ip6_address ip6;
#endif
};

#ifdef IPV6
unsigned int    ip6_fmt(char *, ip6_addr *);
unsigned int    ip6_fmt_exp(char *, ip6_addr *);
unsigned int    ip6_fmt_flat(char *, ip6_addr *);
unsigned int    ip6_scan(char *, ip6_addr *);
unsigned int    ip6_scanbracket(char *, ip6_addr *);
#endif
unsigned int    ip4_fmt(char *, ip_addr *);
unsigned int    ip4_scan(char *, ip_addr *);
unsigned int    ip4_scanbracket(char *, ip_addr *);

#endif
