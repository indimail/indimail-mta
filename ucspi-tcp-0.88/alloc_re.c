/*
 * $Log: alloc_re.c,v $
 * Revision 1.2  2019-05-26 12:03:28+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "byte.h"

int
alloc_re(x, old_size, new_size)
	char          **x;
	unsigned int    old_size;
	unsigned int    new_size;
{
	char           *y;

	if (!(y = alloc(new_size)))
		return 0;
	byte_copy(y, old_size, *x);
	alloc_free(*x);
	*x = y;
	return 1;
}
