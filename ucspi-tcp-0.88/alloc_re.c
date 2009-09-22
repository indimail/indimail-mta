/*
 * $Log: alloc_re.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
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
