/*
 * $Log: getln.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "getln.h"

int
getln(buffer *ss, stralloc *sa, int *match, int sep)
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
