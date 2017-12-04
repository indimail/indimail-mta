/*
 * $Log: SendWelcomeMail.c,v $
 * Revision 2.6  2016-05-17 14:56:55+05:30  Cprogrammer
 * use control directory defined by configure
 *
 * Revision 2.5  2003-12-19 20:46:40+05:30  Cprogrammer
 * made activation and welcome mail configurable via ACTIVATEMAIL, WELCOMEMAIL
 * environment variables
 *
 * Revision 2.4  2003-03-27 20:38:16+05:30  Cprogrammer
 * added argument subject to CopyEmailFile
 *
 * Revision 2.3  2003-02-01 14:12:17+05:30  Cprogrammer
 * change for CopyEmailFile() change
 *
 * Revision 2.2  2002-08-01 16:09:52+05:30  Cprogrammer
 * send activation mail if inactFlag is non zero else send welcome mail
 *
 * Revision 2.1  2002-05-20 15:17:36+05:30  Cprogrammer
 * routine to send welcome mail
 *
 */
#include "indimail.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: SendWelcomeMail.c,v 2.6 2016-05-17 14:56:55+05:30 Cprogrammer Stab mbhangui $";
#endif

void
SendWelcomeMail(char *homedir, char *username, char *domain, int inactFlag, char *subject)
{
	char            email[MAX_BUFF], TmpBuf[MAX_BUFF], bulkdir[MAX_BUFF];
	char           *ptr;
	struct stat     statbuf;

	snprintf(bulkdir, MAX_BUFF, "%s/%s/%s", 
		CONTROLDIR, domain, (ptr = getenv("BULK_MAILDIR")) ? ptr : BULK_MAILDIR);
	if (!access(bulkdir, F_OK))
	{
		snprintf(TmpBuf, MAX_BUFF, "%s/%s", 
			bulkdir, inactFlag ? ((ptr = getenv("ACTIVATEMAIL")) ? ptr : ACTIVATEMAIL) : ((ptr = getenv("WELCOMEMAIL")) ? ptr : WELCOMEMAIL));
		if (!stat(TmpBuf, &statbuf))
		{
			snprintf(email, MAX_BUFF, "%s@%s", username, domain);
			CopyEmailFile(homedir, TmpBuf, email, 0, 0, subject, 1, 0, statbuf.st_size);
		}
	}
	return;
}

void
getversion_SendWelcomeMail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
