/*
 * $Log: stralloc_cats.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_cats(stralloc * sa, char *s)
{
	return stralloc_catb(sa, s, str_len(s));
}
