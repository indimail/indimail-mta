/*
 * $Log: stralloc_arts.c,v $
 * Revision 1.3  2004-10-22 20:30:45+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_starts(sa, s)
	stralloc       *sa;
	char           *s;
{
	int             len;
	len = str_len(s);
	return (sa->len >= len) && byte_equal(s, len, sa->s);
}

void
getversion_stralloc_arts_c()
{
	static char    *x = "$Id: stralloc_arts.c,v 1.3 2004-10-22 20:30:45+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
