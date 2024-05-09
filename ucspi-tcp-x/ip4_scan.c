/*
 * $Log: ip4_scan.c,v $
 * Revision 1.3  2020-08-03 17:24:09+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.2  2017-03-30 22:50:41+05:30  Cprogrammer
 * prefix rbl with ip4_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <scan.h>
#include "ip4.h"

unsigned int
rblip4_scan(const char *s, char ip[4])
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
