/*
 * $Log: getln.c,v $
 * Revision 1.3  2004-10-22 20:25:38+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:19:02+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "substdio.h"
#include "byte.h"
#include "stralloc.h"
#include "getln.h"

int
getln(ss, sa, match, sep)
	register substdio *ss;
	register stralloc *sa;
	int            *match;
	int             sep;
{
	char           *cont;
	unsigned int    clen;

	if (getln2(ss, sa, &cont, &clen, sep) == -1)
		return -1;
	if (!clen)
	{
		*match = 0;
		return 0;
	}
	if (!stralloc_catb(sa, cont, clen))
		return -1;
	*match = 1;
	return 0;
}

void
getversion_getln_c()
{
	static char    *x = "$Id: getln.c,v 1.3 2004-10-22 20:25:38+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
