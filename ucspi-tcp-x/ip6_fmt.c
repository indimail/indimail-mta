/*
 * $Log: ip6_fmt.c,v $
 * Revision 1.5  2020-10-08 20:03:07+05:30  Cprogrammer
 * fixed bug in ip6_fmt() - Erwin Hoffman
 *
 * Revision 1.4  2020-08-03 17:24:33+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.3  2015-08-27 00:21:29+05:30  Cprogrammer
 * fixed ip6_fmt() function.
 *
 * Revision 1.2  2005-06-10 12:10:40+05:30  Cprogrammer
 * conditional ipv6 support
 *
 * Revision 1.1  2005-06-10 09:04:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include <fmt.h>
#include <byte.h>
#include "ip4.h"
#include "ip6.h"

/*
 * authors fefe, Erwin Hoffman
 */

unsigned int
ip6_fmt(char *s, char ip[16])
{
	unsigned int    len, i, temp, temp0, compressing, compressed;
	int             j;

	len = compressing = compressed = 0;
	for (j = 0; j < 16; j += 2) {
		if (j == 12 && ip6_isv4mapped(ip)) {
			len += ip4_fmt(s, ip + 12);
			break;
		}
		temp = ((unsigned long) (unsigned char) ip[j] << 8) + (unsigned long) (unsigned char) ip[j + 1];
		temp0 = 0;
		if (!compressing && j < 16)
			temp0 = ((unsigned long) (unsigned char) ip[j + 2] << 8) + (unsigned long) (unsigned char) ip[j + 3];
		if (temp == 0 && temp0 == 0 && !compressed) {
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

	return len;
}

unsigned int
ip6_fmt_flat(char *s, char ip[16])
{
	int             i;

	if (!s)
		return (32);
	for (i = 0; i < 16; i++) {
		*s++ = tohex((unsigned char) ip[i] >> 4);
		*s++ = tohex((unsigned char) ip[i] & 15);
	}
	return 32;
}

unsigned int
ip6_fmt_exp(char *s, char ip[16])
{
	int             i, j;

	if (!s)
		return 39;
	for (i = 0, j = 1; i < 16; i++, j++) {
		*s++ = tohex((unsigned char) ip[i] >> 4);
		*s++ = tohex((unsigned char) ip[i] & 15);
		if (!((i + 1) % 2) && (i + 1) < 16)
			*s++ = ':';
	}
	return 39;
}
#endif
