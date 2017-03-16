/*
 * $Log: isvirtualdomain.c,v $
 * Revision 2.6  2017-03-13 14:03:35+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.5  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.4  2009-02-18 09:07:29+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.3  2005-12-29 22:45:11+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-09-20 19:53:45+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.1  2002-08-25 22:33:10+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.3  2001-11-24 12:19:20+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:16+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: isvirtualdomain.c,v 2.6 2017-03-13 14:03:35+05:30 Cprogrammer Exp mbhangui $";
#endif

int
isvirtualdomain(char *domain)
{
	FILE           *fp;
	char           *sysconfdir, *controldir, *ptr;
	char            tmpbuf[MAX_BUFF];

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(tmpbuf, MAX_BUFF, "%s/virtualdomains", controldir);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(tmpbuf, MAX_BUFF, "%s/%s/virtualdomains", sysconfdir, controldir);
	}
	if(!(fp = fopen(tmpbuf, "r")))
		return(0);
	for(;;)
	{
		if (!fgets(tmpbuf, MAX_BUFF, fp))
		{
			if(feof(fp))
				break;
			fprintf(stderr, "isvirtualdomain: fgets: %s\n", strerror(errno));
			return (-1);
		}
		if((ptr = strchr(tmpbuf, ':')))
		{
			*ptr = 0;
			if(!strcmp(tmpbuf, domain))
			{
				fclose(fp);
				return(1);
			}
		}
	}
	fclose(fp);
	return(0);
}

void
getversion_isvirtualdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
