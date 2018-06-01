/*
 * $Log: stralloc_opys.c,v $
 * Revision 1.3  2004-10-22 20:30:52+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:08+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_copys(sa, s)
	stralloc       *sa;
	char           *s;
{
	return stralloc_copyb(sa, s, str_len(s));
}

void
getversion_stralloc_opys_c()
{
	static char    *x = "$Id: stralloc_opys.c,v 1.3 2004-10-22 20:30:52+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
