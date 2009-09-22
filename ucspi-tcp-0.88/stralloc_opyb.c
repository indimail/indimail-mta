/*
 * $Log: stralloc_opyb.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "byte.h"

int
stralloc_copyb(stralloc * sa, char *s, unsigned int n)
{
	if (!stralloc_ready(sa, n + 1))
		return 0;
	byte_copy(sa->s, n, s);
	sa->len = n;
	sa->s[n] = 'Z';				/*- ``offensive programming'' */
	return 1;
}
