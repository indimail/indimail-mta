/*
 * $Log: isvalid_domain.c,v $
 * Revision 1.2  2002-03-04 12:50:47+05:30  Cprogrammer
 * typecast to int to avoid gcc complaint on solaris
 *
 * Revision 1.1  2002-02-23 20:41:24+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: isvalid_domain.c,v 1.2 2002-03-04 12:50:47+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <ctype.h>

int
isvalid_domain(char *domain)
{
	char           *ptr;
	register int    i;

	for (i = 0, ptr = domain; ptr && *ptr; ptr++, i++)
	{
		if (*ptr != '-' && *ptr != '.' && !isalnum((int) *ptr))
			return (0);
	}
	if(i > MAX_PW_DOMAIN)
		return(0);
	return (1);
}

void
getversion_isvalid_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
