/*
 * $Log: stralloc_cat.c,v $
 * Revision 1.3  2004-10-22 20:30:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:58+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "byte.h"
#include "stralloc.h"

int
stralloc_cat(sato, safrom)
	stralloc       *sato;
	stralloc       *safrom;
{
	return stralloc_catb(sato, safrom->s, safrom->len);
}

void
getversion_stralloc_cat_c()
{
	static char    *x = "$Id: stralloc_cat.c,v 1.3 2004-10-22 20:30:47+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
