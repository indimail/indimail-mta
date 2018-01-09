/*
 * $Log: stralloc_num.c,v $
 * Revision 1.2  2004-10-22 20:30:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-04 23:18:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"

int
stralloc_catulong0(sa, u, n)
	stralloc       *sa;
	unsigned long   u;
	unsigned int    n;
{
	unsigned int    len;
	unsigned long   q;
	char           *s;

	len = 1;
	q = u;
	while (q > 9)
	{
		++len;
		q /= 10;
	}
	if (len < n)
		len = n;

	if (!stralloc_readyplus(sa, len))
		return 0;
	s = sa->s + sa->len;
	sa->len += len;
	while (len)
	{
		s[--len] = '0' + (u % 10);
		u /= 10;
	}

	return 1;
}

int
stralloc_catlong0(sa, l, n)
	stralloc       *sa;
	long            l;
	unsigned int    n;
{
	if (l < 0)
	{
		if (!stralloc_append(sa, "-"))
			return 0;
		l = -l;
	}
	return stralloc_catulong0(sa, l, n);
}

void
getversion_stralloc_num_c()
{
	static char    *x = "$Id: stralloc_num.c,v 1.2 2004-10-22 20:30:50+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
