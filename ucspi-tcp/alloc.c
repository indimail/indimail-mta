/*
 * $Log: alloc.c,v $
 * Revision 1.3  2019-05-26 12:03:12+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2010-03-18 15:06:37+05:30  Cprogrammer
 * add unsigned integer overflow check to alloc.c
 * Matthew Dempsky - http://marc.info/?l=qmail&m=125213850310173&w=2
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "error.h"
#include <stdlib.h>

#define ALIGNMENT 16			/*- XXX: assuming that this alignment is enough */
#define SPACE 2048				/*- must be multiple of ALIGNMENT */

typedef union
{
	char            irrelevant[ALIGNMENT];
	double          d;
}
aligned;
static aligned  realspace[SPACE / ALIGNMENT];
#define space ((char *) realspace)
static unsigned int avail = SPACE;	/*- multiple of ALIGNMENT; 0<=avail<=SPACE */

/*@null@*//*@out@*/char *alloc(n)
	unsigned int    n;
{
	char           *x;
	unsigned int    m = n;

	if ((n = ALIGNMENT + n - (n & (ALIGNMENT - 1))) < m)	/*- handle overflow */
	{
		errno = error_nomem;
		return 0;
	}
	if (n <= avail)
	{
		avail -= n;
		return space + avail;
	}
	if (!(x = malloc(n)))
		errno = error_nomem;
	return x;
}

void
alloc_free(x)
	char           *x;
{
	if (x >= space && (x < space + SPACE))
		return;				/*- XXX: assuming that pointers are flat */
	free(x);
}
