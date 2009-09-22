/*
 * $Log: ip4_scan.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "scan.h"
#include "ip4.h"

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
