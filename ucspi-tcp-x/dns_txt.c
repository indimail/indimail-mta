/*
 * $Log: dns_txt.c,v $
 * Revision 1.6  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2020-08-03 17:23:31+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.4  2020-04-30 22:06:11+05:30  Cprogrammer
 * use dns_resolve_tx from dns_resolve.c
 *
 * Revision 1.3  2020-04-30 18:00:45+05:30  Cprogrammer
 * change scope of variable dns_resolve_tx to local
 *
 * Revision 1.2  2017-03-30 22:48:35+05:30  Cprogrammer
 * prefix rbl with dns_txt() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stralloc.h>
#include <uint16.h>
#include <byte.h>
#include "dns.h"

int
dns_txt_packet(stralloc *out, const char *buf, unsigned int len)
{
	unsigned int    pos;
	char            header[12];
	uint16          numanswers;
	uint16          datalen;
	char            ch;
	unsigned int    txtlen;
	int             i;

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
		if (byte_equal(header, 2, DNS_T_TXT)) {
			if (byte_equal(header + 2, 2, DNS_C_IN)) {
				if (pos + datalen > len)
					return -1;
				txtlen = 0;
				for (i = 0; i < datalen; ++i) {
					ch = buf[pos + i];
					if (!txtlen)
						txtlen = (unsigned char) ch;
					else {
						--txtlen;
						if (ch < 32)
							ch = '?';
						if (ch > 126)
							ch = '?';
						if (!stralloc_append(out, &ch))
							return -1;
					}
				}
			}
		}
		pos += datalen;
	}

	return 0;
}

static char    *q = 0;

int
rbl_dns_txt(stralloc * out, stralloc * fqdn)
{
	if (!dns_domain_fromdot(&q, fqdn->s, fqdn->len))
		return -1;
	if (dns_resolve(q, DNS_T_TXT) == -1)
		return -1;
	if (dns_txt_packet(out, dns_resolve_tx.packet, dns_resolve_tx.packetlen) == -1)
		return -1;
	dns_transmit_free(&dns_resolve_tx);
	dns_domain_free(&q);
	return 0;
}
