/*
 * $Log: stralloc_cats.c,v $
 * Revision 1.3  2004-10-22 20:30:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:00+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_cats(sa, s)
	stralloc       *sa;
	char           *s;
{
	return stralloc_catb(sa, s, str_len(s));
}

void
getversion_stralloc_cats_c()
{
	static char    *x = "$Id: stralloc_cats.c,v 1.3 2004-10-22 20:30:47+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
