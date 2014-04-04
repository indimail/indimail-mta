/*
 * $Log: isnum.c,v $
 * Revision 1.2  2001-12-11 11:31:50+05:30  Cprogrammer
 * typecast for isdigit to stop compiler complaints
 *
 * Revision 1.1  2001-12-07 22:29:17+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: isnum.c,v 1.2 2001-12-11 11:31:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include <ctype.h>

int
isnum(str)
	char           *str;
{
	char           *ptr;

	for (ptr = str; *ptr; ptr++)
		if (!isdigit((int) *ptr))
			return (0);
	return (1);
}

void
getversion_isnum_c()
{
	printf("%s\n", sccsid);
	return;
}
