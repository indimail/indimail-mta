/*
 * $Log: vset_default_domain.c,v $
 * Revision 2.3  2005-12-29 22:53:46+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-05-17 14:02:35+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.1  2002-12-31 11:54:51+05:30  Cprogrammer
 * modification for IPv6
 *
 * Revision 1.3  2001-11-24 12:22:28+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:02:13+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: vset_default_domain.c,v 2.3 2005-12-29 22:53:46+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Set the default domain from either the DEFAULT_DOMAIN
 */
void
vset_default_domain(char *domain)
{
	int             i;
	char           *tmpstr, *default_domain;
#ifdef IP_ALIAS_DOMAINS
	char            host[MAX_BUFF];
#endif

	if (!domain || *domain)
		return;
	if((tmpstr = (char *) getenv("INDIMAIL_DOMAIN")) != NULL)
	{
		scopy(domain, tmpstr, MAX_BUFF);
		return;
	}
#ifdef IP_ALIAS_DOMAINS
	tmpstr = (char *) getenv("TCPLOCALIP");
	/* courier-imap uses IPv6 */
	if (tmpstr &&  *tmpstr == ':')
	{
		for(tmpstr += 2;*tmpstr != ':';tmpstr++)
		++tmpstr;
	}
	memset(host, 0, MAX_BUFF);
	if (vget_ip_map(tmpstr, host, MAX_BUFF) == 0 && !host_in_locals(host))
	{
		if (*host)
			scopy(domain, host, MAX_BUFF);
		return;
	}
#endif
	getEnvConfigStr(&default_domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	if((i = slen(default_domain)))
		scopy(domain, default_domain, i + 1);
}

void
getversion_vset_default_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
