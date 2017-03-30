/*
 * $Log: ip6_expandaddr.c,v $
 * Revision 1.3  2017-03-30 22:55:53+05:30  Cprogrammer
 * prefix rbl with ip6_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.2  2015-08-27 00:20:30+05:30  Cprogrammer
 * check for stralloc failure
 *
 * Revision 1.1  2013-08-06 00:49:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "stralloc.h"
#include "str.h"
#include "hexconversion.h"
#include "ip6.h"

/**
 * This function expands any valid IPv6 address into its full format of 16 bytes.
 * It returns the number of processed tokens on success.
 * @param src 		Source IPv6 address.
 * @param destination	Expanded IPv6 address.
 * @return -1: No memory could allocated, 0:failure, 1: success
 */

unsigned int
ip6_expandaddr(char *src, stralloc *destination)
{
	stralloc        addresstemp = { 0 };
	char            ip6[16] = { 0 };
	char            hexvalue[3] = { 0, 0, 0 };
	int             i;


	if (!stralloc_copys(&addresstemp, src))
		return -1;
	if (!stralloc_0(&addresstemp))
		return -1;
	if (rblip6_scan(addresstemp.s, ip6) == 0)
		return 0;
	if (!stralloc_copys(destination, ""))
		return -1;
	for (i = 0; i < 16; i++) {
		bytetohex((unsigned char) ip6[i], hexvalue);
		if (!stralloc_catb(destination, hexvalue, 2))
			return -1;
		if (!((i + 1) % 2) && (i + 1) < 16 && !stralloc_cats(destination, ":"))
			return -1; /*- Append ':' after every two bytes. */
	}
	return 1;
}
