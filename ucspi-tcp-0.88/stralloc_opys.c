/*
 * $Log: stralloc_opys.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_copys(stralloc * sa, char *s)
{
	return stralloc_copyb(sa, s, str_len(s));
}
