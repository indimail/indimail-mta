/*
 * $Log: alloc.c,v $
 * Revision 1.5  2008-08-03 18:23:51+05:30  Cprogrammer
 * use stdlib.h
 *
 * Revision 1.4  2004-12-20 22:55:46+05:30  Cprogrammer
 * change return type of malloc to void * (ansic)
 *
 * Revision 1.3  2004-10-22 20:17:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:14:26+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "alloc.h"
#include "error.h"
#include <stdlib.h>

#define ALIGNMENT 16			/*- XXX: assuming that this alignment is enough */
#define SPACE 4096				/*- must be multiple of ALIGNMENT */

typedef union
{
	char            irrelevant[ALIGNMENT];
	double          d;
} aligned;

static aligned  realspace[SPACE / ALIGNMENT];
#define space ((char *) realspace)
static unsigned int avail = SPACE;	/*- multiple of ALIGNMENT; 0<=avail<=SPACE */

/* @null@ */
/* @out@ */
char       *
alloc(n)
	unsigned int    n;
{
	char           *x;
	n = ALIGNMENT + n - (n & (ALIGNMENT - 1));	/*- XXX: could overflow */
	if (n <= avail)
	{
		avail -= n;
		return space + avail;
	}
	x = malloc(n);
	if (!x)
		errno = error_nomem;
	return x;
}

void
alloc_free(x)
	char           *x;
{
	if (x >= space)
		if (x < space + SPACE)
			return;				/*- XXX: assuming that pointers are flat */
	free(x);
}

void
getversion_alloc_c()
{
	static char    *x = "$Id: alloc.c,v 1.5 2008-08-03 18:23:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
