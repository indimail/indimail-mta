/*
 * $Log: ip4_bit.c,v $
 * Revision 1.5  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2020-08-03 17:23:58+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.3  2020-04-30 18:00:50+05:30  Cprogrammer
 * changed scope of variable strnum to local
 *
 * Revision 1.2  2016-05-05 01:20:20+05:30  Cprogrammer
 * fix stack smashing - num defined as int instead of unsigned long
 *
 * Revision 1.1  2013-08-06 07:56:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <str.h>
#include <fmt.h>
#include <stralloc.h>
#include <scan.h>
#include <byte.h>
#include "ip4_bit.h"
#include "ip4.h"

#define BITSUBSTITUTION

int
getaddressasbit(char *ip, int prefix, stralloc *ip4string)
{
	int             count, i, sufcount = 0, pos = 0, len, posl;
	unsigned long   num;
#ifdef BITSUBSTITUTION
	const char     *letterarray = "abcdefghijklmnopqrstuvwxyz123456";
#endif

	len = byte_chr(ip, str_len(ip), '/');
	posl = byte_chr(ip, len, '.');
	ip4string->len = 0;
	for (;;) {
		num = 0;
		count = 1;
		i = ip[pos + posl];
		ip[pos + posl] = 0;
		if (!scan_ulong(ip + pos, &num)) {
			ip[pos + posl] = i;
			return (2);
		}
		ip[pos + posl] = i;
		if (num > 255 || num < 0)
			return (2);
		for (i = 1; i < 9; i++) {
			if (sufcount >= prefix)
				return 0;
			count *= 2;
			if (num >= 256 / count) {
				num -= (256 / count);
#ifdef BITSUBSTITUTION
				if (!stralloc_catb(ip4string, letterarray + sufcount, 1))
					return (-1);
#else
				if (!stralloc_cats(ip4string, "1"))
					return (-1);
#endif
			} else
			if (!stralloc_cats(ip4string, "0"))
				return (-1);
			++sufcount;
		}
		pos += posl + 1;
		if (pos < len + 1) {
			posl = byte_chr(ip + pos + 1, len - pos - 1, '.');
			++posl;
		} else
			return (2);
	}
	return 0;
}

static char     strnum[FMT_ULONG];
int
getbitasaddress(stralloc *ip4string)
{
	static stralloc ipaddr = { 0 };
	static stralloc buffer = { 0 };
	int             iplen, prefix, num = 0, value = 256;

	prefix = ip4string->len - 1;
	if (!stralloc_copys(&buffer, ""))
		return (-1);
	if (!stralloc_copys(&ipaddr, ""))
		return (-1);
	for (iplen = 1; iplen <= prefix; iplen++) {
		if (!stralloc_copyb(&buffer, ip4string->s + iplen, 1))
			return (-1);
		if (byte_diff(buffer.s, 1, "0") != 0) {
			num += (value / 2);
			value /= 2;
		} else
			value /= 2;
		if (iplen % 8 == 0 || iplen == prefix) {
			if (!stralloc_catb(&ipaddr, strnum, fmt_ulong(strnum, num)))
				return (-1);
			if (iplen < 32 && !stralloc_cats(&ipaddr, "."))
				return (-1);
			num = 0;
			value = 256;
		}
	}
	if (!stralloc_copy(ip4string, &ipaddr))
		return (-1);
	if (!stralloc_cats(ip4string, "/"))
		return (-1);
	if (!stralloc_catb(ip4string, strnum, fmt_ulong(strnum, prefix)))
		return (-1);
	return 0;
}
