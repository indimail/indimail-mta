/*
 * $Log: dns_nd.c,v $
 * Revision 1.2  2005-06-10 12:09:56+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "fmt.h"
#include "dns.h"
#ifdef IPV6
/*
 * RFC1886:
 * *   4321:0:1:2:3:4:567:89ab
 * * ->
 * *   b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.IP6.INT.
 */

static inline char
tohex(char c)
{
	return c >= 10 ? c - 10 + 'a' : c + '0';
}

int
dns_name6_domain(char name[DNS_NAME6_DOMAIN], char ip[16])
{
	unsigned int    j;

	for (j = 0; j < 16; j++)
	{
		name[j * 4] = 1;
		name[j * 4 + 1] = tohex(ip[15 - j] & 15);
		name[j * 4 + 2] = 1;
		name[j * 4 + 3] = tohex((unsigned char) ip[15 - j] >> 4);
	}
	byte_copy(name + 4 * 16, 10, "\3ip6\4arpa\0");
	return 4 * 16 + 10;
}
#endif
void
dns_name4_domain(char name[DNS_NAME4_DOMAIN], char ip[4])
{
	unsigned int    namelen;
	unsigned int    i;

	namelen = 0;
	i = fmt_ulong(name + namelen + 1, (unsigned long) (unsigned char) ip[3]);
	name[namelen++] = i;
	namelen += i;
	i = fmt_ulong(name + namelen + 1, (unsigned long) (unsigned char) ip[2]);
	name[namelen++] = i;
	namelen += i;
	i = fmt_ulong(name + namelen + 1, (unsigned long) (unsigned char) ip[1]);
	name[namelen++] = i;
	namelen += i;
	i = fmt_ulong(name + namelen + 1, (unsigned long) (unsigned char) ip[0]);
	name[namelen++] = i;
	namelen += i;
	byte_copy(name + namelen, 14, "\7in-addr\4arpa\0");
}
