/*
 * $Log: ip.c,v $
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
#include "byte.h"
#include "ip.h"

unsigned int
ip_fmt(s, ip)
	char           *s;
	ip_addr        *ip;
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
ip_scan(s, ip)
	char           *s;
	ip_addr        *ip;
{
	unsigned int    i;
	unsigned int    len;
	unsigned long   u;

	len = 0;
	if (!(i = scan_ulong(s, &u)))
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
	ip->d[1] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	if (!(i = scan_ulong(s, &u)))
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
	ip->d[3] = u;
	s += i;
	len += i;
	return len;
}

unsigned int
ip4_scan(char *s, char ip[4])
{
	unsigned int    i;
	unsigned int    len;
	unsigned long   u;

	len = 0;
	i = scan_ulong(s, &u);
	if (!i)
		return 0;
	ip[0] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	i = scan_ulong(s, &u);
	if (!i)
		return 0;
	ip[1] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	i = scan_ulong(s, &u);
	if (!i)
		return 0;
	ip[2] = u;
	s += i;
	len += i;
	if (*s != '.')
		return 0;
	++s;
	++len;
	i = scan_ulong(s, &u);
	if (!i)
		return 0;
	ip[3] = u;
	s += i;
	len += i;
	return len;
}

unsigned int
ip_scanbracket(s, ip)
	char           *s;
	ip_addr        *ip;
{
	unsigned int    len;

	if (*s != '[')
		return 0;
	if (!(len = ip4_scan(s + 1, (char *) ip->d)))
		return 0;
	if (s[len + 1] != ']')
		return 0;
	return len + 2;
}

#ifdef IPV6
int
fmt_hexbyte(char *s, unsigned char byte)
{
	static char     data[] = "0123456789abcdef";

	if (s)
	{
		*s++ = data[(byte >> 4) & 0xf];
		*s = data[byte & 0xf];
	}
	return 2;
}

unsigned int
ip6_fmt(s, ip6)
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

/*
 * IPv6 addresses are really ugly to parse.
 * Syntax: (h = hex digit)
 *   1. hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 *   2. any number of 0000 may be abbreviated as "::", but only once
 *   3. The last two words may be written as IPv4 address
 */

unsigned int
ip6_scan(char *s, ip6_addr *ip6)
{
	unsigned int    i;
	unsigned int    len = 0;
	unsigned long   u;

	char            suffix[16];
	unsigned char  *ip;
	int             prefixlen = 0, suffixlen = 0;

	ip = ip6->d;
	if ((i = ip4_scan((char *) s, (char *) ip + 12)))
	{
		unsigned char *c = V4mappedprefix;
		if (byte_equal((char *) ip + 12, 4, (char *) V6any))
			c = V6any;
		for (len = 0; len < 12; ++len)
			ip[len]= c[len];
		return i;
	}
	for (i = 0; i < 16; i++)
		ip[i] = 0;
	for (;;)
	{
		if (*s == ':')
		{
			len++;
			if (s[1] == ':') /*- Found "::", skip to part 2 */
			{
				s += 2;
				len++;
				break;
			}
			s++;
		}
		if (!(i = scan_xlong((char *) s, &u)))
			return 0;
		if (prefixlen == 12 && s[i] == '.')
		{
			/*
			 * the last 4 bytes may be written as IPv4 address 
			 */
			if ((i = ip4_scan((char *) s, (char *) ip)))
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
	/*
	 * part 2, after "::" 
	 */
	for (;;)
	{
		if (*s == ':')
		{
			if (suffixlen == 0)
				break;
			s++;
			len++;
		} else
		if (suffixlen != 0)
			break;
		i = scan_xlong((char *) s, &u);
		if (!i)
		{
			len--;
			break;
		}
		if (suffixlen + prefixlen <= 12 && s[i] == '.')
		{
			int             j = ip4_scan((char *) s, suffix + suffixlen);
			if (j)
			{
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

void
getversion_ip_c()
{
	static char    *x = "$Id: ip.c,v 1.7 2008-09-16 16:28:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
