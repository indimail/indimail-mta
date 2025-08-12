/*
 * $Log: sorted.c,v $
 * Revision 1.5  2025-08-12 22:06:57+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.4  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2020-11-23 00:17:54+05:30  Cprogrammer
 * removed supression of ANSI C proto for alloc()
 *
 * Revision 1.2  2020-05-10 17:47:09+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.1  2008-06-03 23:21:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "alloc.h"
#include "gen_allocdefs.h"
#include "sorted.h"
#include "byte.h"

GEN_ALLOC_readyplus(sorted,stralloc,p,len,a,100,sorted_readyplus)

int sorted_insert(sorted *sl, stralloc *sa)
{
	int             i;
	int             j;
	int             eq;
	int             len;
	stralloc        newsa;

	if (!sorted_readyplus(sl, 1))
		return 0;
	j = sl->len++;
	newsa = sl->p[j];
	while (j)
	{
		i = (j - 1);
		len = sa->len;
		if (sl->p[i].len < len)
			len = sl->p[i].len;
		eq = byte_diff(sl->p[i].s, len, sa->s);
		if (eq < 0)
			break;
		if (eq == 0)
			if (sl->p[i].len < sa->len)
				break;
		sl->p[j] = sl->p[i];
		j = i;
	}
	sl->p[j] = newsa;
	stralloc_copyb(&sl->p[j], sa->s, sa->len);
	return 1;
}

void
getversion_sorted_c()
{
	const char     *x = "$Id: sorted.c,v 1.5 2025-08-12 22:06:57+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
