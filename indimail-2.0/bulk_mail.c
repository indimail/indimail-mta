/*
 * $Log: bulk_mail.c,v $
 * Revision 2.4  2016-05-17 14:41:16+05:30  Cprogrammer
 * replace control directory with CONTROLDIR
 *
 * Revision 2.3  2010-05-06 22:30:33+05:30  Cprogrammer
 * corrected BulkMail flag path
 *
 * Revision 2.2  2004-07-02 18:04:20+05:30  Cprogrammer
 * rename .BulkMail to BulkMail
 *
 * Revision 2.1  2003-02-01 14:05:53+05:30  Cprogrammer
 * CopyEmailFile() changed
 *
 * Revision 1.8  2002-03-03 11:50:00+05:30  Cprogrammer
 * removed function RemoteBulkMail() to Login_Tasks() to prevent multiple
 * invocations
 *
 * Revision 1.7  2002-03-03 11:17:57+05:30  Cprogrammer
 * corrected problem with stat failing with errno = 2
 * code shortened by changing if() tests for st_mtime > last_mtime
 *
 * Revision 1.6  2002-03-02 00:53:33+05:30  Cprogrammer
 * code restructured
 * bulk mail files in BULK_MAILDIR to now have ,all suffix
 * Added Function RemoteBulkMail to copy bulk mails after lookup into mysql bulmail table
 *
 * Revision 1.5  2002-02-25 13:54:02+05:30  Cprogrammer
 * use mtime instead of ctime for SNAP OS Bug
 *
 * Revision 1.4  2001-11-28 23:44:13+05:30  Cprogrammer
 * removed updation of .BulkMail flag and added it to Login_Tasks.c
 *
 * Revision 1.3  2001-11-24 12:17:49+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:37+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: bulk_mail.c,v 2.4 2016-05-17 14:41:16+05:30 Cprogrammer Exp mbhangui $";
#endif

int
bulk_mail(email, domain, homedir)
	const char     *email, *domain, *homedir;
{
	DIR            *Dir;
	struct dirent  *Dirent;
	struct stat     statbuf;
	char            TmpBuf[MAX_BUFF], bulkdir[MAX_BUFF];
	char           *ptr;
	time_t          last_mtime;
	int             status;

	snprintf(bulkdir, MAX_BUFF, "%s/%s/%s", 
		CONTROLDIR, domain, (ptr = getenv("BULK_MAILDIR")) ? ptr : BULK_MAILDIR);
	if(access(bulkdir, F_OK))
		return(0);
	if (!(Dir = opendir(bulkdir)))
	{
		fprintf(stderr, "opendir:%s: %s\n", bulkdir, strerror(errno));
		return (1);
	}
	snprintf(TmpBuf, MAX_BUFF, "%s/Maildir/BulkMail", homedir);
	if (stat(TmpBuf, &statbuf))
		last_mtime = 0;
	else
		last_mtime = statbuf.st_mtime;
	for (status = 1;;)
	{
		if (!(Dirent = readdir(Dir)))
			break;
		else
		if (!strncmp(Dirent->d_name, ".", 1))
			continue;
		else
		if(!strstr(Dirent->d_name, ",all"))
			continue;
		snprintf(TmpBuf, MAX_BUFF, "%s/%s", bulkdir, Dirent->d_name);
		if (stat(TmpBuf, &statbuf))
		{
			perror(TmpBuf);
			continue;
		} else
		if (!S_ISREG(statbuf.st_mode) || !(statbuf.st_mtime > last_mtime))
			continue;
		if(!CopyEmailFile(homedir, TmpBuf, email, 0, 0, 0, 0, 1, statbuf.st_size))
			status = 0;
	}
	closedir(Dir);
	return(status);
}

void
getversion_bulk_mail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
