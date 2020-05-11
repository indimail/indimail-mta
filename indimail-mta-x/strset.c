/*
 * $Log: strset.c,v $
 * Revision 1.3  2020-05-11 10:58:31+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.2  2004-10-22 20:31:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-10-10 12:08:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "strset.h"
#include "str.h"
#include "byte.h"
#include "alloc.h"

uint32
strset_hash(s)
	char           *s;
{
	unsigned char   ch;
	uint32          h;

	h = 5381;
	while ((ch = *s)) {
		h = ((h << 5) + h) ^ ch;
		++s;
	}
	return h;
}

int
strset_init(set)
	strset         *set;
{
	int             h;
	set->mask = 15;
	set->n = 0;
	set->a = 10;

	set->first = (int *) alloc(sizeof(int) * (set->mask + 1));
	if (!set->first)
		return 0;
	set->p = (strset_list *) alloc(sizeof(strset_list) * set->a);
	if (!set->p) {
		alloc_free((char *) set->first);
		return 0;
	}
	set->x = (char **) alloc(sizeof(char *) * set->a);
	if (!set->x) {
		alloc_free((char *) set->p);
		alloc_free((char *) set->first);
		return 0;
	}

	for (h = 0; h <= set->mask; ++h)
		set->first[h] = -1;

	return 1;
}

char           *
strset_in(set, s)
	strset         *set;
	char           *s;
{
	uint32          h;
	strset_list    *sl;
	int             i;
	char           *xi;

	h = strset_hash(s);
	i = set->first[h & set->mask];
	while (i >= 0) {
		sl = set->p + i;
		if (sl->h == h) {
			xi = set->x[i];
			if (!str_diff(xi, s))
				return xi;
		}
		i = sl->next;
	}
	return 0;
}

int
strset_add(set, s)
	strset         *set;
	char           *s;
{
	uint32          h;
	int             n;
	strset_list    *sl;

	n = set->n;

	if (n == set->a) {
		int             newa;
		strset_list    *newp;
		char          **newx;

		newa = n + 10 + (n >> 3);
		newp = (strset_list *) alloc(sizeof(strset_list) * newa);
		if (!newp)
			return 0;
		newx = (char **) alloc(sizeof(char *) * newa);
		if (!newx) {
			alloc_free((char *) newp);
			return 0;
		}

		byte_copy((char *) newp, sizeof(strset_list) * n, (char *) set->p);
		byte_copy((char *) newx, sizeof(char *) * n, (char *) set->x);
		alloc_free((char *) set->p);
		alloc_free((char *) set->x);
		set->p = newp;
		set->x = newx;
		set->a = newa;

		if (n + n + n > set->mask) {
			int             newmask;
			int            *newfirst;
			int             i;
			uint32          t;

			newmask = set->mask + set->mask + 1;
			newfirst = (int *) alloc(sizeof(int) * (newmask + 1));
			if (!newfirst)
				return 0;

			for (t = 0; t <= newmask; ++t)
				newfirst[t] = -1;

			for (i = 0; i < n; ++i) {
				sl = set->p + i;
				t = sl->h & newmask;
				sl->next = newfirst[t];
				newfirst[t] = i;
			}

			alloc_free((char *) set->first);
			set->first = newfirst;
			set->mask = newmask;
		}
	}

	h = strset_hash(s);

	sl = set->p + n;
	sl->h = h;
	h &= set->mask;
	sl->next = set->first[h];
	set->first[h] = n;
	set->x[n] = s;
	set->n = n + 1;
	return 1;
}

void
getversion_strset_c()
{
	static char    *x = "$Id: strset.c,v 1.3 2020-05-11 10:58:31+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
