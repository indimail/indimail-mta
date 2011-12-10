/*
 * $Log: vauth_munch_domain.c,v $
 * Revision 2.1  2008-05-28 16:39:39+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 1.3  2001-11-24 12:21:00+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:57:58+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_munch_domain.c,v 2.1 2008-05-28 16:39:39+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *
vauth_munch_domain(char *domain)
{
	int             i;
	static char     tmpbuf[MAX_BUFF];

	if (!domain || !*domain)
		return (domain);
	for (i = 0; domain[i] != 0; ++i)
	{
		tmpbuf[i] = domain[i];
		if (domain[i] == '.' || domain[i] == '-')
			tmpbuf[i] = MYSQL_DOT_CHAR;
	}
	tmpbuf[i] = 0;
	return (tmpbuf);
}

void
getversion_vauth_munch_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
