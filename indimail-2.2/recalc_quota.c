/*
 * $Log: recalc_quota.c,v $
 * Revision 2.8  2009-11-17 20:15:41+05:30  Cprogrammer
 * struct flock members have different order on Mac OS X
 *
 * Revision 2.7  2009-09-25 23:50:16+05:30  Cprogrammer
 * return quota as zero of maildirsize is absent
 *
 * Revision 2.6  2009-06-03 09:31:01+05:30  Cprogrammer
 * use fcntl for file locking
 *
 * Revision 2.5  2008-06-24 21:53:30+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.4  2005-12-21 09:47:57+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.3  2003-01-12 02:00:02+05:30  Cprogrammer
 * prevent interupting quota updation on tty signals
 *
 * Revision 2.2  2002-10-12 02:37:36+05:30  Cprogrammer
 * code to pick up maildirsize or .current_size from the base Maildir
 *
 * Revision 2.1  2002-05-31 15:42:08+05:30  Cprogrammer
 * recalculate quota if size of maildirsize > 512
 *
 * Revision 1.7  2002-04-03 01:42:24+05:30  Cprogrammer
 * get the uid/gid from assign file
 *
 * Revision 1.6  2002-03-25 00:35:47+05:30  Cprogrammer
 * added locking before quota update
 *
 * Revision 1.5  2002-02-24 03:25:44+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.4  2002-02-17 00:17:40+05:30  Cprogrammer
 * removed hardcoding in snprintf
 * reduced penalty for overquota to 1 day
 *
 * Revision 1.3  2001-11-24 12:19:58+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:52+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: recalc_quota.c,v 2.8 2009-11-17 20:15:41+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef USE_MAILDIRQUOTA
mdir_t recalc_quota(char *Maildir, mdir_t *mailcount, mdir_t size_limit, mdir_t count_limit, int force_flag)
#else
mdir_t recalc_quota(char *Maildir, int force_flag)
#endif
{
	static mdir_t   mail_size, mail_count;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif
	int             Fd;
#ifdef USE_MAILDIRQUOTA
	FILE           *fp;
#endif
	static char     prevmaildir[MAX_BUFF];
	char            tmpbuf[MAX_BUFF], maildir[MAX_BUFF];
	char           *ptr;
	struct stat     statbuf;
	time_t          tm;
	uid_t           uid;
	gid_t           gid;
	void            (*pstat[3]) ();

	scopy(maildir, Maildir, MAX_BUFF);
	if ((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
#ifdef USE_MAILDIRQUOTA
	if (mailcount)
		*mailcount = 0;
	snprintf(tmpbuf, MAX_BUFF, "%s/maildirsize", maildir);
#else
	snprintf(tmpbuf, MAX_BUFF, "%s/.current_size", maildir);
#endif
	if (!force_flag)
	{
		if (!stat(tmpbuf, &statbuf))
		{
			tm = time(0);
			if ((tm - statbuf.st_mtime) < 43200 && statbuf.st_size <= 512)
			{
#ifdef USE_MAILDIRQUOTA
				if (!mail_size || !mail_count)
					mail_size = check_quota(maildir, &mail_count);
				if (mailcount)
					*mailcount = mail_count;
#else
				if (!mail_size)
					mail_size = check_quota(maildir);
#endif
				return(mail_size);
			}
		}
	}
	if (force_flag != 2) /*- Cache it */
	{
		if (!strncmp(maildir, prevmaildir, MAX_BUFF))
			return(mail_size);
	}
	if (!(ptr = maildir_to_domain(maildir)) || !vget_assign(ptr, 0, 0, &uid, &gid))
	{
		if (indimailuid == -1 || indimailgid == -1)
			GetIndiId(&indimailuid, &indimailgid);
		uid = indimailuid;
		gid = indimailgid;
	} 
	/*- recursive function, can take time for a large Maildir */
	mail_size = count_dir(maildir, &mail_count); 
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[2] = signal(SIGTSTP, SIG_IGN)) == SIG_ERR)
		return (-1);
	if ((Fd = open(tmpbuf, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1)
	{
		if (errno == 2)
		{
			mail_size = 0;
			(void) signal(SIGINT, pstat[0]);
			(void) signal(SIGQUIT, pstat[1]);
			(void) signal(SIGTSTP, pstat[2]);
			return (0);
		}
		fprintf(stderr, "recalc_quota: %s: %s\n", tmpbuf, strerror(errno));
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
		return (-1);
	}
	if (fchown(Fd, uid, gid) == -1)
	{
		fprintf(stderr, "recalc_quota: chown: %s: %s\n", tmpbuf, strerror(errno));
		close(Fd);
		return (-1);
	}
	if (!(fp = fdopen(Fd, "w")))
	{
		fprintf(stderr, "recalc_quota: fdopen: %s\n", strerror(errno));
		close(Fd);
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
		return (-1);
	}
	fl.l_pid = getpid();
	/*- lock the file */
	for (;;)
	{
		if (fcntl(Fd, F_SETLKW, &fl) == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "recalc_quota: fcntl: %s\n", strerror(errno));
			close(Fd);
			(void) signal(SIGINT, pstat[0]);
			(void) signal(SIGQUIT, pstat[1]);
			(void) signal(SIGTSTP, pstat[2]);
			return(-1);
		}
		break;
	}
#ifdef USE_MAILDIRQUOTA
	if (count_limit)
		fprintf(fp, "%"PRIu64"S,%"PRIu64"C\n", size_limit, count_limit);
	else
		fprintf(fp, "%"PRIu64"S\n", size_limit);
	if (mail_size || mail_count)
		fprintf(fp, "%"PRIu64" %"PRIu64"\n", mail_size, mail_count);
	if (mailcount)
		*mailcount = mail_count;
#else
	fprintf(fp, "%"PRIu64"\n", mail_size);
#endif
	fl.l_type = F_UNLCK;
	if (fcntl(Fd, F_SETLK, &fl) == -1)
		fprintf(stderr, "recalc_quota: fcntl: %s\n", strerror(errno));
	fclose(fp);
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	(void) signal(SIGTSTP, pstat[2]);
	CurCount = mail_count;
	scopy(prevmaildir, maildir, MAX_BUFF);
	return (mail_size);
}

void
getversion_recalc_quota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
