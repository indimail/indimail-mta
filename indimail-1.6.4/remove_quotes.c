/*
 * $Log: remove_quotes.c,v $
 * Revision 2.1  2008-10-24 20:37:06+05:30  Cprogrammer
 * BUG - fixed problem where the last quote was getting copied resulting in malloc problem
 *
 * Revision 1.3  2002-08-04 17:30:30+05:30  Cprogrammer
 * changed default behaviour not to touch the address
 * remove quotes only if the quotes are on the boundary
 *
 * Revision 1.2  2002-02-25 13:54:45+05:30  Cprogrammer
 * corrected bug where one char was missing on scopy
 *
 * Revision 1.1  2002-02-24 22:06:19+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: remove_quotes.c,v 2.1 2008-10-24 20:37:06+05:30 Cprogrammer Stab mbhangui $";
#endif
#include "indimail.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

int
remove_quotes(char *address)
{
	char           *tmpstr, *ptr, *cptr;
	size_t          len;

	if (!address || !*address)
		return (1);
	for (ptr = address;*ptr && isspace((int) *ptr);ptr++);
	if (!*ptr)
		return(1);
	if (*ptr == '\"')
	{
		ptr++;
		len = slen(ptr);
		if (ptr[len - 1] == '\"')
		{
			if (!(tmpstr = (char *) malloc(len)))
			{
				perror("malloc");
				return(-1);
			}
			*(ptr + len - 1) = 0;
			for (cptr = tmpstr;*ptr;*cptr++ = *ptr++);
			*cptr = 0;
			scopy(address, tmpstr, len);
			free(tmpstr);
			return(0);
		} else
			return(0);
	}
	return (0);
}

void
getversion_remove_quotes_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
