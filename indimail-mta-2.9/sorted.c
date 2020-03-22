/*
 * $Log: sorted.c,v $
 * Revision 1.1  2008-06-03 23:21:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "gen_allocdefs.h"
#include "sorted.h"
#include "byte.h"

GEN_ALLOC_readyplus(sorted,stralloc,p,len,a,i,n,x,100,sorted_readyplus)

int
sorted_insert(sl, sa)
	sorted         *sl;
	stralloc       *sa;
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
	static char    *x = "$Id: sorted.c,v 1.1 2008-06-03 23:21:57+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
