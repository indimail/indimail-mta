/*
 * $Log: scopy.c,v $
 * Revision 1.2  2001-11-20 10:55:56+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:09+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: scopy.c,v 1.2 2001-11-20 10:55:56+05:30 Cprogrammer Stab mbhangui $";
#endif

int
scopy(char *dest, const char *src, const int bound)
{
	register int    i;

	if (!src)
	{
		for(i = 0;i < bound;i++)
			dest[i] = 0;
		return 0;
	}
	for (i = 0; src[i] && (i < (bound - 1)); i++)
		dest[i] = src[i];
	dest[i] = 0;
	if(i == (bound - 1) && src[i])
		return(-1);
	return 0;
}

#include <stdio.h>
void
getversion_scopy_c()
{
	printf("%s\n", sccsid);
}
