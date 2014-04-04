/*
 * $Log: parse_email.c,v $
 * Revision 2.1  2008-08-24 17:41:17+05:30  Cprogrammer
 * added getEnvConfigStr
 *
 * Revision 1.3  2001-11-24 12:19:48+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:43+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: parse_email.c,v 2.1 2008-08-24 17:41:17+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * parse out user and domain from an email address utility function
 * 
 * email  = input email address
 * user   = parsed user
 * domain = parsed domain
 * return   0 success
 *         -1 if either user or domain was truncated due to buff_size being reached
 */
int
parse_email(email, user, domain, buff_size)
	char           *email;
	char           *user;
	char           *domain;
	int             buff_size;
{
	char           *ptr, *cptr;
	int             i, truncate_flag;

	for (truncate_flag = i = 0,ptr = email, cptr = user;*ptr && !strchr(ATCHARS, *ptr) && i < (buff_size - 1);
		*cptr++ = *ptr++, i++);
	if (i == (buff_size - 1))
	{
		for (;*ptr && !strchr(ATCHARS, *ptr);ptr++);
		truncate_flag = -1; /*- user will get truncated */
	}
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (i = 0, cptr = domain;*ptr && i < (buff_size - 1);*cptr++ = *ptr++, i++);
	if (i == (buff_size - 1))
		truncate_flag = -1; /*- domain will get truncated */
	*cptr = 0;
	return (truncate_flag);
}

void
getversion_parse_email_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
