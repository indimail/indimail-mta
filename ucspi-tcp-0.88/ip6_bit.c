/*
 * $Log: ip6_bit.c,v $
 * Revision 1.3  2020-04-30 18:01:01+05:30  Cprogrammer
 * changed scope of variable strnum to local
 *
 * Revision 1.2  2017-03-30 22:52:17+05:30  Cprogrammer
 * prefix rbl with ip6_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so"
 *
 * Revision 1.1  2013-08-06 07:56:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "ip6.h"
#include "byte.h"
#include "str.h"
#include "fmt.h"
#include "hexconversion.h"
#include "stralloc.h"

#define BITSUBSTITUTION

static char     strnum[FMT_ULONG];

/**
 * This function converts a IPv6 address into its binary representation.
 * @param ip6address 	The source address.
 * @param ip6string	The destination address.
 * @param prefix 	The net prefix bits (maximum 128 bits for IPv6).
 * @param bitsub 	Enum type which is used in order to turn off/on the substitution of all bits one.
 * @return -1:lack of memory,0: non valid IPv6 address,1: successful converted.
 */

int
ip6tobitstring(char *ip6address, stralloc * ip6string, unsigned int prefix)
{
	char            ip6[16];
	int             bit, octettbitpos, number, shiftedvalue;
	int             i, slashpos, ip6len;
	stralloc        ip6addr = { 0 };
#ifdef BITSUBSTITUTION
	char            subvalueforbitone[1];
	subvalueforbitone[0] = 96;	/* substitution starts from token '_' = 96 */
#endif

	ip6len = str_len(ip6address);
	slashpos = byte_chr(ip6address, ip6len, '/');
	if (!stralloc_copyb(&ip6addr, ip6address, slashpos))
		return -1;
	if (!stralloc_0(&ip6addr))
		return -1;
	for (i = 0; i < ip6addr.len; i++) {
		if (ip6addr.s[i] == 'Z') {
			ip6addr.s[i] = '\0';
			break;
		}
	}
	if (!rblip6_scan(ip6addr.s, ip6))
		return 0;
	if (!stralloc_copys(ip6string, ""))
		return -1;
	for (i = 0; i < 16; i++) {
		number = (unsigned char) ip6[i];
		for (octettbitpos = 7; octettbitpos >= 0; octettbitpos--) {
			shiftedvalue = 1 << octettbitpos;
			bit = number / shiftedvalue;
			number = number - bit * (shiftedvalue);
			if (bit) {
#ifdef BITSUBSTITUTION
				if (!stralloc_catb(ip6string, subvalueforbitone, 1))
					return -1;
				subvalueforbitone[0]++;
#else
				if (!stralloc_cats(ip6string, "1"))
					return -1;
#endif
			} else
			if (!stralloc_cats(ip6string, "0"))
				return -1;
			prefix--;
			if (prefix == 0)
				break;
		}
		if (prefix == 0)
			break;
	}
	return 1;
}

/**
 * This function converts a bit string which is produced by ip6tobitstring() 
 * into an IPv6 address. The string may start with a '^'.
 * @param ip6string 	Source string which need to be converted.
 * @param ip6addr 	Destination string.
 * @return -1: No memory could allocated,0: Failure,1: Success.
 */

int
bitstringtoip6(stralloc * ip6string, stralloc * ip6addr)
{
	int             j = 0;
	int             i = 0;
	int             len, prefix, shiftedvalue;
	int             bitpos = 7;
	int             decimalnumber = 0;
	char            ip6asbytes[16] = { 0 };
	char            ip6compact[40];

	stralloc        bitstemp = { 0 };
	prefix = ip6string->len - 1;
	if (prefix > 128)
		return 0;
	if (!stralloc_copys(&bitstemp, ""))
		return -1;
	if (!stralloc_copys(ip6addr, ""))
		return -1;
	if (ip6string->s[0] == '^')
		j = 1;
	for (i = j, j = 0; i <= prefix; i++) {
		if (ip6string->s[i] != '0') {
			shiftedvalue = 1 << bitpos;
			decimalnumber += shiftedvalue;
		}
		bitpos--;
		if (bitpos == -1) {		/* Put each converted byte into the array. */
			if (j < 16) {
				ip6asbytes[j] = (unsigned char) decimalnumber;
				j++;
				bitpos = 7;
				decimalnumber = 0;
			}
		}
	}
	if (bitpos < 7) {			/* Last bit was read,but the number was not converted. */
		ip6asbytes[j] = (unsigned char) decimalnumber;
		j++;
	}
	len = ip6_compactaddr(ip6compact, ip6asbytes);
	if (!len)
		return 0;
	if (!stralloc_copyb(ip6addr, ip6compact, len))
		return -1;
	if (!stralloc_cats(ip6addr, "/"))
		return -1;
	if (!stralloc_catb(ip6addr, strnum, fmt_ulong(strnum, prefix)))
		return -1;
	if (!stralloc_0(ip6addr))
		return -1;
	return 1;
}
