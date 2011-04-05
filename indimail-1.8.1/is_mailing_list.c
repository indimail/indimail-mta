/*
 * $Log: is_mailing_list.c,v $
 * Revision 2.2  2002-11-22 01:50:48+05:30  Cprogrammer
 * added missing #ifdef VFILTER
 *
 * Revision 2.1  2002-10-11 01:07:24+05:30  Cprogrammer
 * function to return if header name signifies a mailing list
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: is_mailing_list.c,v 2.2 2002-11-22 01:50:48+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
int
is_mailing_list(char *h_name, char *h_data)
{
	int             i;

	if (!strncasecmp(h_mailinglist[0], h_name, MAX_LINE_LENGTH))
	{
		if (!strncasecmp(h_data, "bulk", 5) || !strncasecmp(h_data, "list", 5))
			return (1);
	}
	for (i = 1; h_mailinglist[i]; i++)
	{
		if (!strncasecmp(h_mailinglist[i], h_name, MAX_LINE_LENGTH))
			return (1);
	}
	return (0);
}
#endif

void
getversion_is_mailing_list_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
