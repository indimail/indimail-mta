/*
 * $Log: maildir_to_domain.c,v $
 * Revision 2.1  2004-07-02 18:08:08+05:30  Cprogrammer
 * renamed .domain to domain
 *
 * Revision 1.1  2002-04-02 22:35:55+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: maildir_to_domain.c,v 2.1 2004-07-02 18:08:08+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include "indimail.h"

char           *
maildir_to_domain(char *maildir)
{
	static char     domain[MAX_BUFF];
	char            tmpbuf[MAX_BUFF];
	FILE           *fp;

	snprintf(tmpbuf, MAX_BUFF, "%s/domain", maildir);
	if((fp = fopen(tmpbuf, "r")) != NULL)
	{
		if(!fgets(tmpbuf, MAX_BUFF - 1, fp))
		{
			fclose(fp);
			return((char *) 0);
		}
		fclose(fp);
		if(sscanf(tmpbuf, "%s", domain) != 1)
			return((char *) 0);
		return(domain);
	} else
		return((char *) 0);
}

void
getversion_maildir_to_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
