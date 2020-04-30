/*
 * $Log: dns_name.c,v $
 * Revision 1.4  2020-04-30 22:05:56+05:30  Cprogrammer
 * use dns_resolve_tx from dns_resolve.c
 *
 * Revision 1.3  2020-04-30 18:00:40+05:30  Cprogrammer
 * change scope of variable dns_resolve_tx to local
 *
 * Revision 1.2  2005-06-10 12:09:40+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "uint16.h"
#include "byte.h"
#include "dns.h"
#ifdef IPV6
#include "ip6.h"
#endif

static char    *q = 0;

int
dns_name_packet(stralloc *out, char *buf, unsigned int len)
{
	unsigned int    pos;
	char            header[12];
	uint16          numanswers;
	uint16          datalen;

	if (!stralloc_copys(out, ""))
		return -1;
	pos = dns_packet_copy(buf, len, 0, header, 12);
	if (!pos)
		return -1;
	uint16_unpack_big(header + 6, &numanswers);
	pos = dns_packet_skipname(buf, len, pos);
	if (!pos)
		return -1;
	pos += 4;
	while (numanswers--) {
		pos = dns_packet_skipname(buf, len, pos);
		if (!pos)
			return -1;
		pos = dns_packet_copy(buf, len, pos, header, 10);
		if (!pos)
			return -1;
		uint16_unpack_big(header + 8, &datalen);
		if (byte_equal(header, 2, DNS_T_PTR)) {
			if (byte_equal(header + 2, 2, DNS_C_IN)) {
				if (!dns_packet_getname(buf, len, pos, &q))
					return -1;
				if (!dns_domain_todot_cat(out, q))
					return -1;
				return 0;
			}
		}
		pos += datalen;
	}
	return 0;
}

int
dns_name4(stralloc *out, char ip[4])
{
	char            name[DNS_NAME4_DOMAIN];

	dns_name4_domain(name, ip);
	if (dns_resolve(name, DNS_T_PTR) == -1)
		return -1;
	if (dns_name_packet(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) == -1)
		return -1;
	dns_transmit_free(&dns_resolve_tx);
	dns_domain_free(&q);
	return 0;
}

#ifdef IPV6
int
dns_name6(stralloc *out, char ip[16])
{
	char            name[DNS_NAME6_DOMAIN];

	if (ip6_isv4mapped(ip))
		return dns_name4(out, ip + 12);
	dns_name6_domain(name,ip);
	if (dns_resolve(name, DNS_T_PTR) == -1)
		return -1;
	if (dns_name_packet(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) == -1)
		return -1;
	dns_transmit_free(&dns_resolve_tx);
	dns_domain_free(&q);
	return 0;
}
#endif
