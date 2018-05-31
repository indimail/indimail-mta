/*
 * $Log: stralloc_opyb.c,v $
 * Revision 1.4  2004-10-22 20:30:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-09 23:34:13+05:30  Cprogrammer
 * prevent inclusion of function prototypes for stralloc functions
 *
 * Revision 1.2  2004-07-17 21:24:05+05:30  Cprogrammer
 * added RCS log
 *
 */
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "byte.h"

int
stralloc_copyb(sa, s, n)
	stralloc       *sa;
	char           *s;
	unsigned int    n;
{
	if (!stralloc_ready(sa, n + 1))
		return 0;
	byte_copy(sa->s, n, s);
	sa->len = n;
	sa->s[n] = 'Z';				/*- ``offensive programming'' */
	return 1;
}

void
getversion_stralloc_opyb_c()
{
	static char    *x = "$Id: stralloc_opyb.c,v 1.4 2004-10-22 20:30:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
