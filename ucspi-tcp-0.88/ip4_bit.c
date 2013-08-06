/*
 * $Log: ip4_bit.c,v $
 * Revision 1.1  2013-08-06 07:56:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"
#include "fmt.h"
#include "stralloc.h"
#include "scan.h"
#include "ip4_bit.h"
#include "ip4.h"
#include "byte.h"

#define BITSUBSTITUTION

stralloc        sanumber = { 0 };

char            strnum[FMT_ULONG];

int
getnumber(char *buf, int len, unsigned long *u)
{
	if (!stralloc_copyb(&sanumber, buf, len))
		return -1;
	if (!stralloc_0(&sanumber))
		return -1;
	if (sanumber.s[scan_ulong(sanumber.s, u)])
		return -1;
	return (0);
}

int
getaddressasbit(char *ip, int prefix, stralloc * ip4string)
{
	int             count;
	int             i;
	int             num;
	int             sufcount = 0;
	int             pos = 0;
	int             len = byte_chr(ip, str_len(ip), '/');
	int             posl = byte_chr(ip, len, '.');
#ifdef BITSUBSTITUTION
	char           *letterarray = "abcdefghijklmnopqrstuvwxyz123456";
#endif

	if (!stralloc_copys(ip4string, ""))
		return -1;

	for (;;) {
		num = 0;
		count = 1;
		if (getnumber(ip + pos, posl, (long unsigned int *) &num) == -1)
			return 2;
		if (num > 255 || num < 0)
			return 2;

		for (i = 1; i < 9; i++) {
			if (sufcount >= prefix)
				return 0;
			count *= 2;
			if (num >= 256 / count) {
				num -= (256 / count);
#ifdef BITSUBSTITUTION
				if (!stralloc_catb(ip4string, letterarray + sufcount, 1))
					return -1;
#else
				if (!stralloc_cats(ip4string, "1"))
					return -1;
#endif
			} else
			if (!stralloc_cats(ip4string, "0"))
				return -1;
			++sufcount;
		}
		pos += posl + 1;
		if (pos < len + 1) {
			posl = byte_chr(ip + pos + 1, len - pos - 1, '.');
			++posl;
		} else
			return 2;
	}
	return 0;
}

int
getbitasaddress(stralloc * ip4string)
{
	stralloc        ipaddr = { 0 };
	stralloc        buffer = { 0 };
	int             iplen;
	int             num = 0;
	int             value = 256;
	int             prefix = ip4string->len - 1;

	if (!stralloc_copys(&buffer, ""))
		return -1;
	if (!stralloc_copys(&ipaddr, ""))
		return -1;
	for (iplen = 1; iplen <= prefix; iplen++) {
		if (!stralloc_copyb(&buffer, ip4string->s + iplen, 1))
			return -1;
		if (byte_diff(buffer.s, 1, "0") != 0) {
			num += (value / 2);
			value /= 2;
		} else
			value /= 2;
		if (iplen % 8 == 0 || iplen == prefix) {
			if (!stralloc_catb(&ipaddr, strnum, fmt_ulong(strnum, num)))
				return -1;
			if (iplen < 32 && !stralloc_cats(&ipaddr, "."))
				return -1;
			num = 0;
			value = 256;
		}
	}
	if (!stralloc_copy(ip4string, &ipaddr))
		return -1;
	if (!stralloc_cats(ip4string, "/"))
		return -1;
	if (!stralloc_catb(ip4string, strnum, fmt_ulong(strnum, prefix)))
		return -1;
	return 0;
}
