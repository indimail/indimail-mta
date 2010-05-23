/*
 * $Log: update_newu.c,v $
 * Revision 2.2  2008-06-25 10:15:44+05:30  Cprogrammer
 * corrected permission of cdb file during creation
 *
 * Revision 2.1  2005-12-29 22:50:31+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 1.3  2001-11-24 12:20:11+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:09+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: update_newu.c,v 2.2 2008-06-25 10:15:44+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Compile the users/assign file using qmail-newu program
 */
int
update_newu()
{
	int             pid;

	pid = vfork();
	if (pid == 0)
	{
		char            tmpbuf[MAX_BUFF];
		char           *qmaildir;

		umask(022);
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		snprintf(tmpbuf, MAX_BUFF, "%s/bin/qmail-newu", qmaildir);
		execl(tmpbuf, "qmail-newu", NULL);
		exit(127);
	} else
		wait(&pid);
	return (0);
}

void
getversion_update_newu_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
