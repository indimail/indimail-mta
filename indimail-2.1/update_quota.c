/*
 * $Log: update_quota.c,v $
 * Revision 2.6  2009-11-17 20:15:49+05:30  Cprogrammer
 * struct flock members have different order on Mac OS X
 *
 * Revision 2.5  2009-06-03 09:31:17+05:30  Cprogrammer
 * replaced lockf with fcntl for file locking
 *
 * Revision 2.4  2008-06-24 21:56:44+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.3  2003-01-17 00:01:02+05:30  Cprogrammer
 * for MAILDIRQUOTA return zero if maildirsize does not exist
 *
 * Revision 2.2  2003-01-12 02:00:29+05:30  Cprogrammer
 * prevent interuption of quota updation on tty signals
 *
 * Revision 2.1  2002-10-12 02:37:46+05:30  Cprogrammer
 * code to pick up maildirsize or .current_size from the base Maildir
 *
 * Revision 1.7  2002-04-03 01:42:53+05:30  Cprogrammer
 * get uid/gid from assign file
 *
 * Revision 1.6  2002-03-25 00:36:15+05:30  Cprogrammer
 * added locking for quota updation and reading
 *
 * Revision 1.5  2002-02-24 03:26:03+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.4  2001-12-19 16:29:20+05:30  Cprogrammer
 * used #define to define size of tmpbuf
 *
 * Revision 1.3  2001-11-24 12:20:13+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:10+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: update_quota.c,v 2.6 2009-11-17 20:15:49+05:30 Cprogrammer Stab mbhangui $";
#endif

int
update_quota(char *Maildir, mdir_t new_size)
{
	char            tmpbuf[MAX_BUFF], maildir[MAX_BUFF];
	char           *ptr;
	uid_t           uid;
	gid_t           gid;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif
	void            (*pstat[3]) ();
	int             Fd;
	FILE           *fp;

	scopy(maildir, Maildir, MAX_BUFF);
#ifdef USE_MAILDIRQUOTA
	snprintf(tmpbuf, MAX_BUFF, "%s/maildirsize", maildir);
	if (access(tmpbuf, F_OK))
		return(0);
#else
	snprintf(tmpbuf, MAX_BUFF, "%s/.current_size", maildir);
#endif
	if((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
	/*- attempt to obtain uid, gid for the maildir */
	if(!(ptr = maildir_to_domain(maildir)) || !vget_assign(ptr, 0, 0, &uid, &gid))
	{
		if(indimailuid == -1 || indimailgid == -1)
			GetIndiId(&indimailuid, &indimailgid);
		uid = indimailuid;
		gid = indimailgid;
	} 
	if ((Fd = open(tmpbuf, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR)) == -1)
	{
		fprintf(stderr, "update_quota: %s: %s\n", tmpbuf, strerror(errno));
		return(-1);
	}
	if (fchown(Fd, uid, gid) == -1)
	{
		fprintf(stderr, "update_quota: fchown: %s\n", strerror(errno));
		close(Fd);
		return(-1);
	}
	if (!(fp = fdopen(Fd, "a")))
	{
		fprintf(stderr, "update_quota: %s: %s\n", tmpbuf, strerror(errno));
		close(Fd);
		return(-1);
	}
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[2] = signal(SIGTSTP, SIG_IGN)) == SIG_ERR)
		return (-1);
	for (;;)
	{
		if (fcntl(Fd, F_SETLKW, &fl) == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "update_quota: fcntl: %s\n", strerror(errno));
			close(Fd);
			(void) signal(SIGINT, pstat[0]);
			(void) signal(SIGQUIT, pstat[1]);
			(void) signal(SIGTSTP, pstat[2]);
			return(-1);
		}
		break;
	}
#ifdef USE_MAILDIRQUOTA
	fprintf(fp, "%"PRIu64" 1\n", new_size);
#else
	fprintf(fp, "%"PRIu64"\n", new_size);
#endif
	fl.l_type = F_UNLCK;
	if (fcntl(Fd, F_SETLK, &fl) == -1)
		fprintf(stderr, "update_quota: fcntl: %s\n", strerror(errno));
	fclose(fp);
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	(void) signal(SIGTSTP, pstat[2]);
	return(1);
}

void
getversion_update_quota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
