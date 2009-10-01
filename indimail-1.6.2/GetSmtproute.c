/*
 * $Log: GetSmtproute.c,v $
 * Revision 2.4  2005-12-29 22:44:55+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-09-20 19:53:22+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.2  2003-01-01 19:21:52+05:30  Cprogrammer
 * added code to handle all cases of smtproute format - from qmail-remote(8)
 *
 * Revision 2.1  2002-12-29 19:12:52+05:30  Cprogrammer
 * function to get smtp port from qmail control file 'smtproutes'
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: GetSmtproute.c,v 2.4 2005-12-29 22:44:55+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * qmail control  file SMTPROUTE format
 * host:relay
 * host:relay:port
 * If host is blank, all host match the line
 * If relay is blank, it means MX lookup should be done
 */
int
GetSmtproute(char *domain)
{
	char           *ptr, *qmaildir, *controldir;
	FILE           *fp;
	int             len;
	char            buffer[2048], smtproute[1024];

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(smtproute, sizeof(smtproute) - 1, "%s/%s/smtproutes", qmaildir, controldir) == -1)
	{
		errno = ENAMETOOLONG;
		return(-1);
	}
	if(!(fp = fopen(smtproute, "r")))
	{
		if(errno == ENOENT)
			return(25);
		else
		{
			fprintf(stderr, "GetSmtproute: %s %s\n", smtproute, strerror(errno));
			return(-1);
		}
	}
	for(;;)
	{
		if(!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if((ptr = strchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if(*buffer == ':') /*- wildcard */
		{
			fclose(fp);
			if((ptr = strrchr(buffer + 1, ':')))
			{
				if(*(ptr + 1))
					return(atoi(ptr + 1));
				else
					return(25);
			} else
				return(25);
		}
		if((ptr = strchr(buffer, ':')))
		{
			*ptr = 0;
			if(!strncmp(buffer, domain, sizeof(buffer) - 1))
			{
				fclose(fp);
				len = slen(buffer) + 1;
				*ptr = ':';
				if((ptr = strrchr(buffer + len, ':')))
				{
					if(*(ptr + 1))
						return(atoi(ptr + 1));
					else
						return(25);
				} else
					return(25);
			}
		}
	}
	fclose(fp);
	return(25);
}

void
getversion_GetSmtproute_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
