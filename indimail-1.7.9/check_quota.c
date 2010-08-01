/*
 * $Log: check_quota.c,v $
 * Revision 2.11  2009-06-03 09:28:20+05:30  Cprogrammer
 * use fcntl() interface for file locking
 *
 * Revision 2.10  2009-06-02 15:55:28+05:30  Cprogrammer
 * replaced lockf() with fcntl()
 *
 * Revision 2.9  2009-02-18 21:24:15+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.8  2009-02-06 11:36:47+05:30  Cprogrammer
 * ignore return value of few statements
 *
 * Revision 2.7  2008-06-24 21:45:07+05:30  Cprogrammer
 * porting for 64bit
 *
 * Revision 2.6  2005-12-21 09:45:36+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.5  2003-01-17 01:16:36+05:30  Cprogrammer
 * added missing unlock() and close()
 *
 * Revision 2.4  2002-10-12 02:36:43+05:30  Cprogrammer
 * code to pick up maildirsize or .current_size from the base Maildir
 *
 * Revision 2.3  2002-09-04 12:56:35+05:30  Cprogrammer
 * removed lock before fclose()
 *
 * Revision 2.2  2002-08-11 00:24:44+05:30  Cprogrammer
 * removed unecessary printing of error
 *
 * Revision 2.1  2002-07-11 18:22:20+05:30  Cprogrammer
 * recalculate if current bytes or size is negative
 *
 * Revision 1.7  2002-03-25 00:33:07+05:30  Cprogrammer
 * added locking for quota updation and reading
 *
 * Revision 1.6  2002-02-24 04:11:29+05:30  Cprogrammer
 * code reorg
 *
 * Revision 1.5  2002-02-24 03:24:11+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.4  2001-12-08 00:32:44+05:30  Cprogrammer
 * removed hardcoding
 *
 * Revision 1.3  2001-11-24 12:17:51+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:38+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: check_quota.c,v 2.11 2009-06-03 09:28:20+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Assumes the current working directory is user/Maildir
 *
 * We go off to look at cur and tmp dirs
 * return size of files
 *
 */
#ifdef USE_MAILDIRQUOTA
mdir_t check_quota(char *Maildir, mdir_t *total)
#else
mdir_t check_quota(char *Maildir)
#endif
{
	mdir_t          mail_size;
	char            tmpbuf[MAX_BUFF], maildir[MAX_BUFF];
	char           *ptr;
	int             Fd;
	struct flock    fl;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size, count;
	FILE           *fp;
#endif

	scopy(maildir, Maildir, MAX_BUFF);
	if ((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
	fl.l_type   = F_RDLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
#ifdef USE_MAILDIRQUOTA
	(void) snprintf(tmpbuf, MAX_BUFF, "%s/maildirsize", maildir);
#else
	(void) snprintf(tmpbuf, MAX_BUFF, "%s/.current_size", maildir);
#endif
	if ((Fd = open(tmpbuf, O_RDONLY)) == -1)
	{
		if (errno == ENOENT)
			return (0);
		fprintf(stderr, "check_quota: %s: %s\n", tmpbuf, strerror(errno));
		return (-1);
	}
	if (!(fp = fdopen(Fd, "r")))
	{
		fprintf(stderr, "check_quota: fdopen: %s\n", strerror(errno));
		close(Fd);
		return (-1);
	}
	for (;;)
	{
		if (fcntl(Fd, F_SETLKW, &fl) == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "check_quota: fcntl: %s\n", strerror(errno));
			(void) close(Fd);
			return (-1);
		}
		break;
	}
#ifdef USE_MAILDIRQUOTA
	if (total)
		*total = 0;
	fl.l_type   = F_UNLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	for (mail_size = 0;;)
	{
		if (!fgets(tmpbuf, MAX_BUFF - 2, fp))
			break;
		if (sscanf(tmpbuf, "%"SCNd64" %"SCNd64, &size, &count) == 2)
		{
			mail_size += size;
			if (total)
				*total += count;
		}
	}
	if (fcntl(Fd, F_SETLK, &fl) == -1) ; /*- Make compiler happy */
	(void) close(Fd);
	if (mail_size < 0 || (total && (*total < 0)))
		mail_size = count_dir(maildir, total ? (mdir_t *) total : (mdir_t *) 0);
#else
	fl.l_type   = F_UNLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	if ((mail_size = read(Fd, tmpbuf, MAX_BUFF)) == -1)
	{
		if (fcntl(Fd, F_SETLK, &fl) == -1) ; /*- Make compiler happy */
		(void) close(Fd);
		return(-1);
	}
	if (fcntl(Fd, F_SETLK, &fl) == -1) ; /*- Make compiler happy */
	(void) close(Fd);
	tmpbuf[mail_size] = 0;
	mail_size = atol(tmpbuf);
	if (mail_size < 0)
		mail_size = count_dir(maildir, 0);
#endif
	return (mail_size);
}

void
getversion_check_quota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
