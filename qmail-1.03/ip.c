/*
 * $Log: ip.c,v $
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
#include "fmt.h"
#include "scan.h"
#include "ip.h"
#include "byte.h"
#include "str.h"
#include "fmt.h"

unsigned int
ip4_fmt(char *s, struct ip_address *ip)
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
ip4_scan(char *s, struct ip_address *ip)
{
	unsigned int    i;
	unsigned int    len;
	unsigned long   u;

	len = 0;
	i = scan_ulong(s, &u);
	if (!i)
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
	i = scan_ulong(s, &u);
	if (!i)
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
	i = scan_ulong(s, &u);
	if (!i)
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
	i = scan_ulong(s, &u);
	if (!i)
		return 0;
	if (u > 255)
		return 0;
	ip->d[3] = u;
	s += i;
	len += i;
	return len;
}

unsigned int
ip4_scanbracket(char *s, struct ip_address *ip)
{
	unsigned int    len;

	if (*s != '[')
		return 0;
	len = ip4_scan(s + 1, ip);
	if (!len)
		return 0;
	if (s[len + 1] != ']')
		return 0;
	return len + 2;
}

#ifdef IPV6
unsigned int
ip6_fmtfull(s, ip6)
	char           *s;
	ip6_addr       *ip6;
{
	unsigned int    len;
	unsigned int    i, j, k;

	len = 0;
	for (j = 0, len = 0, k = 0; j < 8; j++)
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
ip6_fmt(char *s, struct ip6_address *ip)
{
	unsigned int    len;
	unsigned int    i;
	unsigned int    temp;
	unsigned int    compressing;
	unsigned int    compressed;
	int             j;
	struct ip_address ip4;

	len = 0;
	compressing = 0;
	compressed = 0;

	for (j = 0; j < 16; j += 2) {
		if (j == 12 && byte_equal((char *) ip, 12, (char *) V4mappedprefix)) {
			for (i = 0; i < 4; i++) {
				ip4.d[i] = ip->d[i + 12];
			}
			len += ip4_fmt(s, &ip4);
			break;
		}

		temp = ((unsigned long) (unsigned char) ip->d[j] << 8) + (unsigned long) (unsigned char) ip->d[j + 1];

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
		*s++ = ':';
		++len;
	}
//  if (s) *s = 0; 
	return len;
}

unsigned int
ip6_scan(char *s, struct ip6_address *ip)
{
	unsigned int    i;
	unsigned int    len = 0;
	unsigned long   u;

	char            suffix[16];
	int             prefixlen = 0;
	int             suffixlen = 0;

	unsigned int    x;
	struct ip_address ip4;

	for (x = 0; x < 4; x++) {	/* Mapped IPv4 addresses */
		ip4.d[x] = ip->d[x + 12];
	}

	if ((i = ip4_scan(s, &ip4))) {
		const char     *c = (const char *) V4mappedprefix;
		if (byte_equal((char *) ip4.d, 4, (char *) V6any))
			c = (const char *) V6any;
		for (len = 0; len < 12; ++len)
			ip->d[len] = c[len];
		return i;
	}

	for (i = 0; i < 16; i++)
		ip->d[i] = 0;

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
		i = scan_xlong(s, &u);
		if (!i)
			return 0;

		if (prefixlen == 12 && s[i] == '.') {
		/*
		 * the last 4 bytes may be written as IPv4 address 
		 */
			i = ip4_scan(s, &ip4);
			if (i) {
			/*
			 * copy into ip->d+12 from ip4 
			 */
				for (x = 0; x < 4; x++) {
					ip->d[x + 12] = ip4.d[x];
				}
				return i + len;
			} else
				return 0;
		}
		ip->d[prefixlen++] = (u >> 8);
		ip->d[prefixlen++] = (u & 255);
		s += i;
		len += i;
		if (prefixlen == 16)
			return len;
	}

/*
 * part 2, after "::" 
 */
	for (;;) {
		if (*s == ':') {
			if (suffixlen == 0)
				break;
			s++;
			len++;
		} else if (suffixlen != 0)
			break;
		i = scan_xlong(s, &u);
		if (!i) {
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
		ip->d[16 - suffixlen + i] = suffix[i];

	return len;
}

unsigned int
ip6_scanbracket(char *s, struct ip6_address *ip6)
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
	static char    *x = "$Id: ip.c,v 1.8 2015-08-24 19:13:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
