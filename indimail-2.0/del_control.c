/*
 * $Log: del_control.c,v $
 * Revision 2.7  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.6  2009-09-28 20:44:47+05:30  Cprogrammer
 * added chkrcptdomains to the list of control files from which a deleted domain should
 * be removed
 *
 * Revision 2.5  2009-01-15 08:55:11+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.4  2005-12-29 22:42:36+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-05-17 14:00:46+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.2  2002-09-29 00:58:14+05:30  Cprogrammer
 * if etrnhosts is absent, do not attempt removing domain from etrnhosts.
 *
 * Revision 2.1  2002-08-25 22:31:49+05:30  Cprogrammer
 * made control dir configurable
 * code for autoturn, etrn
 *
 * Revision 1.3  2001-11-24 12:18:46+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:10+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: del_control.c,v 2.7 2016-05-17 17:09:39+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * delete a domain from the control files
 */
int
del_control(domain)
	char           *domain;
{
	char            filename[MAX_BUFF], tmpbuf[MAX_BUFF];
	char           *qmaildir, *controldir;
	int             i, status = 0, relative;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative)
		snprintf(filename, MAX_BUFF, "%s/%s/rcpthosts", qmaildir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/rcpthosts", controldir);
	status = remove_line(domain, filename, 0, INDIMAIL_QMAIL_MODE);
	if (status < 1) /*- if no lines found or if remove_line returned error */
	{
		snprintf(filename, MAX_BUFF, "%s/%s/morercpthosts", qmaildir, controldir);
		/* at least one matching line found */
		if ((i = remove_line(domain, filename, 0, INDIMAIL_QMAIL_MODE)) > 0)
		{
			struct stat     statbuf;
			if (!stat(filename, &statbuf))
			{
				if (statbuf.st_size == 0)
				{
					unlink(filename);
					scat(filename, ".cdb", MAX_BUFF);
					unlink(filename);
				} else
					compile_morercpthosts();
			}
		}
		if (i == -1)
			status = i;
	} 
	if (relative)
		snprintf(filename, MAX_BUFF, "%s/%s/etrnhosts", qmaildir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/etrnhosts", controldir);
	if(!access(filename, F_OK) && remove_line(domain, filename, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	if (relative)
		snprintf(filename, MAX_BUFF, "%s/%s/chkrcptdomains", qmaildir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/chkrcptdomains", controldir);
	if(!access(filename, F_OK) && remove_line(domain, filename, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	if (relative)
		snprintf(filename, MAX_BUFF, "%s/%s/virtualdomains", qmaildir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/virtualdomains", controldir);
	if(use_etrn == 2)
		snprintf(tmpbuf, MAX_BUFF, "%s:autoturn-", domain);
	else
		snprintf(tmpbuf, MAX_BUFF, "%s:%s", domain, domain);
	if (remove_line(tmpbuf, filename, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	return (status);
}

void
getversion_del_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
