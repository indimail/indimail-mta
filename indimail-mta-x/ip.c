/*
 * $Log: ip.c,v $
 * Revision 1.13  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2020-07-04 16:51:06+05:30  Cprogrammer
 * removed extra inclusion of fmt.h
 *
 * Revision 1.10  2016-04-10 13:27:49+05:30  Cprogrammer
 * fixed mapped ipv4 address in ip6_scan()
 *
 * Revision 1.9  2015-08-27 00:28:51+05:30  Cprogrammer
 * added ip6_fmt_flat(), ip6_fmt_exp() functions
 *
 * Revision 1.8  2015-08-24 19:13:36+05:30  Cprogrammer
 * use compressed ipv6 address
 *
 * Revision 1.7  2008-09-16 16:28:27+05:30  Cprogrammer
 * BUG - ip4_scan was wrong
 *
 * Revision 1.6  2005-08-23 17:30:50+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.5  2005-06-17 21:48:44+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.4  2005-06-11 21:30:17+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.3  2004-10-22 20:25:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:19:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "scan.h"
#include "ip.h"
#include "byte.h"
#include "str.h"
#include "fmt.h"

unsigned int
ip4_fmt(char *s, ip_addr *ip)
{
	unsigned int    len;
	unsigned int    i;

	len = 0;
	i = fmt_ulong(s, (unsigned long) ip->d[0]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[1]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[2]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[3]);
	len += i;
	if (s)
		s += i;
	return len;
}

unsigned int
ip4_scan(const char *s, ip_addr *ip)
{
	unsigned int    i;
	unsigned int    len;
	unsigned long   u;

	len = 0;
	if (!(i = scan_ulong(s, &u)))
		return 0;
	if (u > 255)
		return 0;
	ip->d[0] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	if (!(i = scan_ulong(s, &u)))
		return 0;
	if (u > 255)
		return 0;
	ip->d[1] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	if (!(i = scan_ulong(s, &u)))
		return 0;
	if (u > 255)
		return 0;
	ip->d[2] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	if (!(i = scan_ulong(s, &u)))
		return 0;
	if (u > 255)
		return 0;
	ip->d[3] = u;
	s += i;
	len += i;
	return len;
}

unsigned int
ip4_scanbracket(const char *s, ip_addr *ip)
{
	unsigned int    len;

	if (*s != '[')
		return 0;
	if (!(len = ip4_scan(s + 1, ip)))
		return 0;
	if (s[len + 1] != ']')
		return 0;
	return len + 2;
}

#ifdef IPV6
unsigned int
ip6_fmt_flat(char *s, ip6_addr *ip6)
{
	unsigned int    len;
	unsigned int    i, j, k;

	for (j = len = k = 0; j < 8; j++)
	{
		i = fmt_hexbyte(s, ip6->d[k++]);
		len += i;
		if (s)
			s += i;
		i = fmt_hexbyte(s, ip6->d[k++]);
		len += i;
		if (s)
			s += i;
	}
	return len - 1;
}

unsigned int
ip6_fmt_exp(char *s, ip6_addr *ip6)
{
	unsigned int    len;
	unsigned int    i, j, k;

	for (j = len = k = 0; j < 8; j++)
	{
		i = fmt_hexbyte(s, ip6->d[k++]);
		len += i;
		if (s)
			s += i;
		i = fmt_hexbyte(s, ip6->d[k++]);
		len += i;
		if (s)
			s += i;
		i = fmt_str(s, ":");
		len += i;
		if (s)
			s += i;
	}
	return len - 1;
}

unsigned int
ip6_fmt(char *s, ip6_addr *ip6)
{
	unsigned int    len, i, temp, compressing, compressed;
	int             j;
	ip_addr         ip4;

	len = compressing = compressed = 0;
	for (j = 0; j < 16; j += 2) {
		if (j == 12 && byte_equal((char *) ip6, 12, (char *) V4mappedprefix)) {
			for (i = 0; i < 4; i++)
				ip4.d[i] = ip6->d[i + 12];
			len += ip4_fmt(s, &ip4);
			break;
		}
		temp = ((unsigned long) (unsigned char) ip6->d[j] << 8) + (unsigned long) (unsigned char) ip6->d[j + 1];
		if (temp == 0 && !compressed) {
			if (!compressing) {
				compressing = 1;
				if (j == 0) {
					if (s)
						*s++ = ':';
					++len;
				}
			}
		} else {
			if (compressing) {
				compressing = 0;
				++compressed;
				if (s)
					*s++ = ':';
				++len;
			}
			i = fmt_xlong(s, temp);
			len += i;
			if (s)
				s += i;
			if (j < 14) {
				if (s)
					*s++ = ':';
				++len;
			}
		}
	}
	if (compressing) {
		if (s)
			*s++ = ':';
		++len;
	}
	return len;
}

unsigned int
ip6_scan(const char *s, ip6_addr *ip6)
{
	char            suffix[16];
	unsigned int    i, x, len = 0;
	unsigned long   u;
	int             prefixlen = 0, suffixlen = 0;
	ip_addr         ip4;

#if 0
	for (x = 0; x < 4; x++) {	/* Mapped IPv4 addresses */
		ip4.d[x] = ip6->d[x + 12];
	}
#endif
	if ((i = ip4_scan(s, &ip4))) {
		const char     *c = (const char *) V4mappedprefix;
		if (byte_equal((char *) ip4.d, 4, (char *) V6any))
			c = (const char *) V6any;
		for (len = 0; len < 12; ++len)
			ip6->d[len] = c[len];
		for (x = 0; x < 4; x++) {	/* Mapped IPv4 addresses */
			ip6->d[x + 12] = ip4.d[x];
		}
		return i;
	}
	for (i = 0; i < 16; i++)
		ip6->d[i] = 0;
	for (;;) {
		if (*s == ':') {
			len++;
			if (s[1] == ':') {	/* Found "::", skip to part 2 */
				s += 2;
				len++;
				break;
			}
			s++;
		}
		if (!(i = scan_xlong(s, &u)))
			return 0;
		if (prefixlen == 12 && s[i] == '.') {
			/*- the last 4 bytes may be written as IPv4 address */
			if ((i = ip4_scan(s, &ip4))) {
				for (x = 0; x < 4; x++)
					ip6->d[x + 12] = ip4.d[x]; /*- copy into ip6->d+12 from ip4 */
				return i + len;
			} else
				return 0;
		}
		ip6->d[prefixlen++] = (u >> 8);
		ip6->d[prefixlen++] = (u & 255);
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
		if (!(i = scan_xlong(s, &u))) {
			len--;
			break;
		}
		if (suffixlen + prefixlen <= 12 && s[i] == '.') {
			int             j = ip4_scan(s, &ip4);
			if (j) {
				byte_copy((char *) suffix + suffixlen, 4, (char *) ip4.d);
				suffixlen += 4;
				len += j;
				break;
			} else
				prefixlen = 12 - suffixlen;	/* make end-of-loop test true */
		}
		suffix[suffixlen++] = (u >> 8);
		suffix[suffixlen++] = (u & 255);
		s += i;
		len += i;
		if (prefixlen + suffixlen == 16)
			break;
	}
	for (i = 0; i < suffixlen; i++)
		ip6->d[16 - suffixlen + i] = suffix[i];
	return len;
}

unsigned int
ip6_scanbracket(const char *s, ip6_addr *ip6)
{
	unsigned int    len;

	if (*s != '[')
		return 0;
	len = ip6_scan(s + 1, ip6);
	if (!len)
		return 0;
	if (s[len + 1] != ']')
		return 0;
	return len + 2;
}
#endif

void
getversion_ip_c()
{
	const char     *x = "$Id: ip.c,v 1.13 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
