/*
 * $Log: ip6_scan.c,v $
 * Revision 1.4  2017-03-30 22:56:58+05:30  Cprogrammer
 * prefix rbl with ip6_scan(), ip4_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.3  2015-08-27 00:23:23+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.2  2008-09-17 09:32:49+05:30  Cprogrammer
 * corrections for ip4_scan()
 *
 * Revision 1.1  2005-06-10 12:12:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <scan.h>
#include "ip4.h"
#include "ip6.h"

/*
 * IPv6 addresses are really ugly to parse.
 * Syntax: (h = hex digit)
 *   1. hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 *   2. any number of 0000 may be abbreviated as "::", but only once
 *   3. The last two words may be written as IPv4 address
 */

unsigned int
rblip6_scan(char *s, char ip[16])
{
	unsigned int    i;
	unsigned int    len = 0;
	unsigned long   u;

	char            suffix[16];
	int             prefixlen = 0;
	int             suffixlen = 0;

	if ((i = rblip4_scan((char *) s, ip + 12))) {
		unsigned char *c = V4mappedprefix;
		if (byte_equal((char *) ip + 12, 4, (char *) V6any))
			c = V6any;
		for (len = 0; len < 12; ++len)
			ip[len]= c[len];
		return i;
	}
	for (i = 0; i < 16; i++)
		ip[i] = 0;
	for (;;) {
		if (*s == ':') {
			len++;
			if (s[1] == ':') {	/*- Found "::", skip to part 2 */
				s += 2;
				len++;
				break;
			}
			s++;
		}
		if (!(i = scan_xlong((char *) s, &u)))
			return 0;
		if (prefixlen == 12 && s[i] == '.') { /*- the last 4 bytes may be written as IPv4 address */
			if ((i = rblip4_scan((char *) s, ip + 12)))
				return i + len;
			else
				return 0;
		}
		ip[prefixlen++] = (u >> 8);
		ip[prefixlen++] = (u & 255);
		s += i;
		len += i;
		if (prefixlen == 16)
			return len;
	}
	/*- part 2, after "::" */
	for (;;) {
		if (*s == ':') {
			if (suffixlen == 0)
				break;
			s++;
			len++;
		} else
		if (suffixlen != 0)
			break;
		if (!(i = scan_xlong((char *) s, &u))) {
			len--;
			break;
		}
		if (suffixlen + prefixlen <= 12 && s[i] == '.') {
			int             j = rblip4_scan((char *) s, suffix + suffixlen);
			if (j) {
				suffixlen += 4;
				len += j;
				break;
			} else
				prefixlen = 12 - suffixlen;	/*- make end-of-loop test true */
		}
		suffix[suffixlen++] = (u >> 8);
		suffix[suffixlen++] = (u & 255);
		s += i;
		len += i;
		if (prefixlen + suffixlen == 16)
			break;
	}
	for (i = 0; i < suffixlen; i++)
		ip[16 - suffixlen + i] = suffix[i];
	return len;
}
#endif
