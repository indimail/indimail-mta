/*
 * $Log: GetSmtproute.c,v $
 * Revision 2.7  2016-05-17 17:09:39+05:30  mbhangui
 * use control directory set by configure
 *
 * Revision 2.6  2010-05-28 14:11:07+05:30  Cprogrammer
 * use QMTP as default
 *
 * Revision 2.5  2010-04-24 15:27:38+05:30  Cprogrammer
 * new function get_smtp_qmtp_port to parse both smtproutes, qmtproutes file
 * return QMTP or SMTP port.
 *
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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: GetSmtproute.c,v 2.7 2016-05-17 17:09:39+05:30 mbhangui Exp $";
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
	char           *qmaildir, *controldir, *routes;
	int             default_port, relative;
	char            smtproute[1024];

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative)
		snprintf(smtproute, sizeof(smtproute) - 1, "%s/%s/qmtproutes", qmaildir, controldir);
	else
		snprintf(smtproute, sizeof(smtproute) - 1, "%s/qmtproutes", controldir);
	default_port = access(smtproute, F_OK) ? PORT_SMTP : PORT_QMTP;
	if ((routes = getenv("ROUTES")) && *routes)
	{
		if (!memcmp(routes, "qmtp", 4))
			default_port = PORT_QMTP;
		else
		if (!memcmp(routes, "smtp", 4))
			default_port = PORT_SMTP;
	}
	if (relative)
		snprintf(smtproute, sizeof(smtproute) - 1, "%s/%s/%s",
			qmaildir, controldir, default_port == PORT_SMTP ? "smtproutes" : "qmtproutes");
	else
		snprintf(smtproute, sizeof(smtproute) - 1, "%s/%s",
			controldir, default_port == PORT_SMTP ? "smtproutes" : "qmtproutes");
	return (get_smtp_qmtp_port(smtproute, domain, default_port));
}

int
get_smtp_qmtp_port(char *file, char *domain, int default_port)
{
	char            buffer[2048];
	char           *ptr;
	int             len;
	FILE           *fp;

	if(!(fp = fopen(file, "r")))
	{
		if(errno == ENOENT)
			return(default_port);
		else
		{
			fprintf(stderr, "GetSmtproute: %s %s\n", file, strerror(errno));
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
					return(default_port);
			} else
				return(default_port);
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
						return(default_port);
				} else
					return(default_port);
			}
		}
	}
	fclose(fp);
	return(default_port);
}

void
getversion_GetSmtproute_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
