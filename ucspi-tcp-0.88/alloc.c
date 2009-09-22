/*
 * $Log: alloc.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "error.h"
extern void    *malloc();
extern void     free();

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
	n = ALIGNMENT + n - (n & (ALIGNMENT - 1));	/*- XXX: could overflow */
	if (n <= avail)
	{
		avail -= n;
		return space + avail;
	}
	if(!(x = malloc(n)))
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
