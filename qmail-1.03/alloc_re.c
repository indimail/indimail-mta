/*
 * $Log: alloc_re.c,v $
 * Revision 1.5  2004-10-22 20:18:06+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-11 13:48:32+05:30  Cprogrammer
 * prevent inclusing of alloc.h with prototypes
 *
 * Revision 1.3  2004-07-17 21:14:39+05:30  Cprogrammer
 * added RCS log
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

	y = alloc(new_size);
	if (!y)
		return 0;
	byte_copy(y, old_size, *x);
	alloc_free(*x);
	*x = y;
	return 1;
}

void
getversion_alloc_re_c()
{
	static char    *x = "$Id: alloc_re.c,v 1.5 2004-10-22 20:18:06+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
