/*
 * $Log: dns_ip.c,v $
 * Revision 1.6  2020-04-30 22:05:30+05:30  Cprogrammer
 * use dns_resolve_tx in dns_resolve.c
 *
 * Revision 1.5  2020-04-30 18:00:15+05:30  Cprogrammer
 * change scope of variable dns_resolve_tx to local
 *
 * Revision 1.4  2017-03-30 22:45:17+05:30  Cprogrammer
 * renamed ip6_scan() to rblip6_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.3  2007-06-10 10:14:25+05:30  Cprogrammer
 * beautify
 *
 * Revision 1.2  2005-06-10 12:08:49+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stralloc.h>
#include <uint16.h>
#include <byte.h>
#include "dns.h"
#ifdef IPV6
#include "ip4.h"
#include "ip6.h"

static int
dns_ip6_packet_add(stralloc *out, char *buf, unsigned int len)
{
	unsigned int    pos;
	char            header[16];
	uint16          numanswers;
	uint16          datalen;

	if (!(pos = dns_packet_copy(buf, len, 0, header, 12)))
		return -1;
	uint16_unpack_big(header + 6, &numanswers);
	if (!(pos = dns_packet_skipname(buf, len, pos)))
		return -1;
	pos += 4;
	while (numanswers--) {
		if (!(pos = dns_packet_skipname(buf, len, pos)))
			return -1;
		if (!(pos = dns_packet_copy(buf, len, pos, header, 10)))
			return -1;
		uint16_unpack_big(header + 8, &datalen);
		if (byte_equal(header, 2, DNS_T_AAAA)) {
			if (byte_equal(header + 2, 2, DNS_C_IN) && datalen == 16) {
				if (!dns_packet_copy(buf, len, pos, header, 16))
					return -1;
				if (!stralloc_catb(out, header, 16))
					return -1;
			}
		} else
		if (byte_equal(header, 2, DNS_T_A) && byte_equal(header + 2, 2, DNS_C_IN) && datalen == 4) {
			byte_copy(header, 12, (char *) V4mappedprefix);
			if (!dns_packet_copy(buf, len, pos, header + 12, 4))
				return -1;
			if (!stralloc_catb(out, header, 16))
				return -1;
		}
		pos += datalen;
	}
	dns_sortip6(out->s, out->len);
	return 0;
}

int
dns_ip6_packet(stralloc *out, char *buf, unsigned int len)
{
	if (!stralloc_copys(out, ""))
		return -1;
	return dns_ip6_packet_add(out, buf, len);
}

static char    *q = 0;

int
dns_ip6(stralloc *out, stralloc *fqdn)
{
	unsigned int    i;
	char            code;
	char            ch;
	char            ip[16];

	if (!stralloc_copys(out, ""))
		return -1;
	if (!stralloc_readyplus(fqdn, 1))
		return -1;
	fqdn->s[fqdn->len] = 0;
	if ((i = rblip6_scan(fqdn->s, ip))) {
		if (fqdn->s[i])
			return -1;
		stralloc_copyb(out, ip, 16);
		return 0;
	}
	code = 0;
	for (i = 0; i <= fqdn->len; ++i) {
		if (i < fqdn->len)
			ch = fqdn->s[i];
		else
			ch = '.';
		if ((ch == '[') || (ch == ']'))
			continue;
		if (ch == '.') {
			if (!stralloc_append(out, &code))
				return -1;
			code = 0;
			continue;
		}
		if ((ch >= '0') && (ch <= '9')) {
			code *= 10;
			code += ch - '0';
			continue;
		}
		if (!dns_domain_fromdot(&q, fqdn->s, fqdn->len))
			return -1;
		if (!stralloc_copys(out, ""))
			return -1;
		if (dns_resolve(q, DNS_T_AAAA) != -1) {
			if (dns_ip6_packet_add(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) != -1) {
				dns_transmit_free(&dns_resolve_tx);
				dns_domain_free(&q);
			}
		}
		if (!dns_domain_fromdot(&q, fqdn->s, fqdn->len))
			return -1;
		if (dns_resolve(q, DNS_T_A) != -1) {
			if (dns_ip6_packet_add(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) != -1) {
				dns_transmit_free(&dns_resolve_tx);
				dns_domain_free(&q);
			}
		}
		return out->a > 0 ? 0 : -1;
	}
	out->len &= ~3;
	return 0;
}
#else
int
dns_ip4_packet(stralloc * out, char *buf, unsigned int len)
{
	unsigned int    pos;
	char            header[12];
	uint16          numanswers;
	uint16          datalen;

	if (!stralloc_copys(out, ""))
		return -1;
	if (!(pos = dns_packet_copy(buf, len, 0, header, 12)))
		return -1;
	uint16_unpack_big(header + 6, &numanswers);
	if (!(pos = dns_packet_skipname(buf, len, pos)))
		return -1;
	pos += 4;
	while (numanswers--) {
		if (!(pos = dns_packet_skipname(buf, len, pos)))
			return -1;
		if (!(pos = dns_packet_copy(buf, len, pos, header, 10)))
			return -1;
		uint16_unpack_big(header + 8, &datalen);
		if (byte_equal(header, 2, DNS_T_A) && byte_equal(header + 2, 2, DNS_C_IN) && datalen == 4) {
			if (!dns_packet_copy(buf, len, pos, header, 4))
				return -1;
			if (!stralloc_catb(out, header, 4))
				return -1;
		}
		pos += datalen;
	}
	dns_sortip(out->s, out->len);
	return 0;
}

static char    *q = 0;

int
dns_ip4(stralloc *out, stralloc *fqdn)
{
	unsigned int    i;
	char            code;
	char            ch;

	if (!stralloc_copys(out, ""))
		return -1;
	code = 0;
	for (i = 0; i <= fqdn->len; ++i) {
		if (i < fqdn->len)
			ch = fqdn->s[i];
		else
			ch = '.';
		if ((ch == '[') || (ch == ']'))
			continue;
		if (ch == '.')
		{
			if (!stralloc_append(out, &code))
				return -1;
			code = 0;
			continue;
		}
		if ((ch >= '0') && (ch <= '9'))
		{
			code *= 10;
			code += ch - '0';
			continue;
		}
		if (!dns_domain_fromdot(&q, fqdn->s, fqdn->len))
			return -1;
		if (dns_resolve(q, DNS_T_A) == -1)
			return -1;
		if (dns_ip4_packet(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) == -1)
			return -1;
		dns_transmit_free(&dns_resolve_tx);
		dns_domain_free(&q);
		return 0;
	}
	out->len &= ~3;
	return 0;
}
#endif
