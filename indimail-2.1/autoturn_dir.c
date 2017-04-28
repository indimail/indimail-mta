/*
 * $Log: autoturn_dir.c,v $
 * Revision 2.5  2017-03-13 13:36:15+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.4  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.3  2005-12-29 22:39:39+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-09-20 19:52:47+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.1  2002-09-01 18:34:30+05:30  Cprogrammer
 * function to get the Maildir path for AUTOTURN domains
 *
 */
#include "indimail.h"
#include <string.h>
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: autoturn_dir.c,v 2.5 2017-03-13 13:36:15+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *
autoturn_dir(char *domain)
{
	char            filename[MAX_BUFF], template[MAX_BUFF];
	static char     tmpbuf[MAX_BUFF];
	char           *ptr, *sysconfdir, *controldir;
	int             len;
	FILE           *fp;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(filename, MAX_BUFF, "%s/virtualdomains", controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/%s/virtualdomains", sysconfdir, controldir);
	if (!(fp = fopen(filename, "r")))
	{
		perror(filename);
		return ((char *) 0);
	}
	snprintf(template, sizeof(template), "%s:", domain);
	for (;;)
	{
		if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
		if ((ptr = strchr(tmpbuf, '\n')) || (ptr = strchr(tmpbuf, '#')))
			*ptr = 0;
		for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!strncmp(tmpbuf, template, len = slen(template)))
		{
			fclose(fp);
			return (tmpbuf + len + 9);
		}
	}
	fclose(fp);
	return ((char *) 0);
}

void
getversion_autoturn_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
