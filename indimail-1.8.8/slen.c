/*
 * $Log: slen.c,v $
 * Revision 1.2  2001-11-20 10:56:04+05:30  Cprogrammer
 * Added getversion_slen_c()
 *
 * Revision 1.1  2001-10-24 18:15:11+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: slen.c,v 1.2 2001-11-20 10:56:04+05:30 Cprogrammer Stab mbhangui $";
#endif

int
slen(const char *dest)
{
	register int    i;

	if (!dest)
		return (0);
	for (i = 0; dest[i]; i++);
	return (i);

}

#include <stdio.h>
void
getversion_slen_c()
{
	printf("%s\n", sccsid);
}
