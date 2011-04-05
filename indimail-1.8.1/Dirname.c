/*
 * $Log: Dirname.c,v $
 * Revision 1.1  2001-12-19 20:56:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: Dirname.c,v 1.1 2001-12-19 20:56:49+05:30 Cprogrammer Stab mbhangui $";
#endif

char    *
Dirname(char *path)
{
	static char     tmpbuf[MAX_BUFF];
	char           *ptr;

	if (!path || !*path)
		return ((char *) 0);
	scopy(tmpbuf, path, MAX_BUFF);
	if ((ptr = strrchr(tmpbuf, '/')) != (char *) 0)
	{
		if (ptr == tmpbuf)
			return ("/");
		*ptr = 0;
		return (tmpbuf);
	} 
	return ((char *) 0);
}

void
getversion_Dirname_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
