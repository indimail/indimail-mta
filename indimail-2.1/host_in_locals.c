/*
 * $Log: host_in_locals.c,v $
 * Revision 2.5  2017-03-13 13:44:51+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.4  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.3  2005-12-29 22:45:04+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-09-20 19:53:35+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.1  2002-08-25 22:32:49+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.3  2001-11-24 12:19:09+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:58+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: host_in_locals.c,v 2.5 2017-03-13 13:44:51+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
int
host_in_locals(domain)
	char           *domain;
{
	char            tmpbuf[MAX_BUFF];
	char           *ptr, *sysconfdir, *controldir;
	FILE           *fs;

	if(!domain || !*domain)
		return(0);
	else
	if (!strcmp(domain, "localhost"))
		return (1);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(tmpbuf, MAX_BUFF, "%s/locals", controldir);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(tmpbuf, MAX_BUFF, "%s/%s/locals", sysconfdir, controldir);
	}
	if(!(fs = fopen(tmpbuf, "r")))
		return (0);
	for(;;)
	{
		if(!fgets(tmpbuf, MAX_BUFF, fs))
			break;
		if((ptr = strrchr(tmpbuf, '\n')) || (ptr = strchr(tmpbuf, '#')))
			*ptr = 0;
		for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!strncmp(domain, tmpbuf, MAX_BUFF))
		{
			fclose(fs);
			return (1);
		}
	}
	fclose(fs);
	return (0);
}
#endif

void
getversion_host_in_locals_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
