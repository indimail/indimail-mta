/*
 * $Log: stralloc_cat.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "stralloc.h"

int
stralloc_cat(stralloc * sato, stralloc * safrom)
{
	return stralloc_catb(sato, safrom->s, safrom->len);
}
