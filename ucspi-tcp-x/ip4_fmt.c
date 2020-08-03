/*
 * $Log: ip4_fmt.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <fmt.h>
#include "ip4.h"

unsigned int
ip4_fmt(char *s, char ip[4])
{
	unsigned int    len;
	unsigned int    i;

	len = 0;
	i = fmt_ulong(s, (unsigned long) (unsigned char) ip[0]);
	len += i;
	if (s)
		s += i;
	if (s)
		*s++ = '.';
	++len;
	i = fmt_ulong(s, (unsigned long) (unsigned char) ip[1]);
	len += i;
	if (s)
		s += i;
	if (s)
		*s++ = '.';
	++len;
	i = fmt_ulong(s, (unsigned long) (unsigned char) ip[2]);
	len += i;
	if (s)
		s += i;
	if (s)
		*s++ = '.';
	++len;
	i = fmt_ulong(s, (unsigned long) (unsigned char) ip[3]);
	len += i;
	if (s)
		s += i;
	return len;
}
