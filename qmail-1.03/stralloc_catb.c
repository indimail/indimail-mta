/*
 * $Log: stralloc_catb.c,v $
 * Revision 1.4  2004-10-22 20:30:46+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-09 23:33:34+05:30  Cprogrammer
 * prevent inclusion of function prototypes for stralloc functions
 *
 * Revision 1.2  2004-07-17 21:23:56+05:30  Cprogrammer
 * added RCS log
 *
 */
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "byte.h"

int
stralloc_catb(sa, s, n)
	stralloc       *sa;
	char           *s;
	unsigned int    n;
{
	if (!sa->s)
		return stralloc_copyb(sa, s, n);
	if (!stralloc_readyplus(sa, n + 1))
		return 0;
	byte_copy(sa->s + sa->len, n, s);
	sa->len += n;
	sa->s[sa->len] = 'Z';		/*- ``offensive programming'' */
	return 1;
}

void
getversion_stralloc_catb_c()
{
	static char    *x = "$Id: stralloc_catb.c,v 1.4 2004-10-22 20:30:46+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
