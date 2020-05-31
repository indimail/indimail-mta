/*
 * $Log: stralloc_copy.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "stralloc.h"

int
stralloc_copy(stralloc * sato, stralloc * safrom)
{
	return stralloc_copyb(sato, safrom->s, safrom->len);
}
