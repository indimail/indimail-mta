/*
 * $Log: vmake_maildir.c,v $
 * Revision 2.7  2009-02-18 21:36:01+05:30  Cprogrammer
 * check chown for error
 *
 * Revision 2.6  2007-12-22 00:33:49+05:30  Cprogrammer
 * moved BulkMail flag to Maildir
 *
 * Revision 2.5  2004-07-02 18:13:57+05:30  Cprogrammer
 * renamed .domain to domain, .BulkMail to BulkMail
 *
 * Revision 2.4  2004-05-17 14:02:30+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.3  2003-06-18 23:11:07+05:30  Cprogrammer
 * added chown of .BulkMail file
 *
 * Revision 2.2  2002-08-25 22:36:24+05:30  Cprogrammer
 * added chmod()
 *
 * Revision 2.1  2002-05-15 00:06:02+05:30  Cprogrammer
 * create .BulkMail to prevent current bulletins for getting delivered to new users
 *
 * Revision 1.5  2002-02-17 00:14:15+05:30  Cprogrammer
 * create migrateflag for new users if CREATE_FLAG_ON_NEWUSER defined
 *
 * Revision 1.4  2001-12-03 04:18:33+05:30  Cprogrammer
 * removed hardcoding of Qmail.txt
 *
 * Revision 1.3  2001-11-24 12:22:05+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:55+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef	lint
static char     sccsid[] = "$Id: vmake_maildir.c,v 2.7 2009-02-18 21:36:01+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vmake_maildir(char *dir, uid_t uid, gid_t gid, char *domain)
{
	FILE           *fp;
	int             i;
	char            tmpbuf[MAX_BUFF];
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};

	snprintf(tmpbuf, MAX_BUFF, "%s/Maildir", dir);
	if(access(tmpbuf, F_OK) && r_mkdir(tmpbuf, INDIMAIL_DIR_MODE, uid, gid))
		return(-1);
	for(i = 0;i < 3;i++)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/%s", dir, MailDirNames[i]);
		if(access(tmpbuf, F_OK) && (mkdir(tmpbuf, INDIMAIL_DIR_MODE) || chown(tmpbuf, uid, gid)))
			return(-1);
		chmod(tmpbuf, INDIMAIL_DIR_MODE);
	}
	if(use_etrn)
		return(0);
	snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/domain", dir);
	if(!(fp = fopen(tmpbuf, "w")))
		return(-1);
	if(domain && *domain)
		fprintf(fp, "%s\n", domain);
	else
		fprintf(fp, "localhost\n");
	fclose(fp);
	if(chown(tmpbuf, uid, gid))
		return(-1);
	/*- Prevent Current Bulletins to be delivered */
	snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/BulkMail", dir);
	close(open(tmpbuf, O_CREAT | O_TRUNC , 0644));
	if (chown(tmpbuf, uid, gid) == -1)
		return(-1);
#ifdef CREATE_FLAG_ON_NEWUSER
	snprintf(tmpbuf, MAX_BUFF, "%s/%s", dir, MIGRATEFLAG);
	close(open(tmpbuf, O_TRUNC | O_CREAT , 0600));
	if(chown(tmpbuf, uid, gid) == -1)
		return(-1);
#endif
	return(0);
}

void
getversion_vmake_maildir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
