/*
 * $Log: scat.c,v $
 * Revision 1.2  2001-11-20 10:55:54+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:09+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: scat.c,v 1.2 2001-11-20 10:55:54+05:30 Cprogrammer Stab mbhangui $";
#endif

int
scat(char *dest, const char *src, const int bound)
{
	register int    i, j;
	for (i = 0; dest[i]; i++)	/*- nowt */ ;
	for (j = 0; src[j] && ((i + j) < (bound - 1)); j++)
		dest[i + j] = src[j];
	dest[i + j] = 0;
	if((i + j) == (bound - 1) && src[j])
	{
		return -1;
	}
	return 0;
}

#include <stdio.h>
void
getversion_scat_c()
{
	printf("%s\n", sccsid);
}
