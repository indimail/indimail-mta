/*
 * $Log: addressToken.c,v $
 * Revision 2.3  2002-11-22 01:14:00+05:30  Cprogrammer
 * added missing #ifdef VFILTER
 *
 * Revision 2.2  2002-10-11 21:38:07+05:30  Cprogrammer
 * added function to reinitialize token
 *
 * Revision 2.1  2002-10-11 01:05:00+05:30  Cprogrammer
 * function to return address tokens from a comma separated list of email addresses
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: addressToken.c,v 2.3 2002-11-22 01:14:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <ctype.h>
#include <string.h>

static int      idx;
static char     email[MAX_PW_NAME + MAX_DOMAINNAME + 2];
char           *
addressToken(char *email_list)
{
	int             skip, len;
	char           *ptr, *cptr;

	ptr = email_list + idx;
	if (!*ptr)
		return ((char *) 0);
	skip = 0;
	for (cptr = email, len = 0; *ptr && *ptr != ','; ptr++, idx++)
	{
		if (isspace((int) *ptr) || *ptr == '<' || *ptr == '>')
			continue;
		if(!skip && *ptr == '\"')
		{
			skip = 1;
			continue;
		}
		if(!skip)
		{
			*cptr++ = *ptr;
			len++;
			if(len == (MAX_PW_NAME + MAX_DOMAINNAME + 1))
			{
				for(;*ptr && *ptr != ',';ptr++, idx++);
				break;
			}
		}
		if(skip && *ptr == '\"')
			skip = 0;
	}
	*cptr = 0;
	if (*ptr == ',')
		idx++;
	return (email);
}

void
rewindAddrToken()
{
	idx = 0;
	*email = 0;
}
#endif

void
getversion_addressToken_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
