/*
 * $Log: getDbLock.c,v $
 * Revision 2.3  2008-05-28 16:35:34+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-10-29 13:17:26+05:30  Cprogrammer
 * conditional compilation of USE_LOCKING
 *
 * Revision 2.1  2003-10-23 13:15:25+05:30  Cprogrammer
 * conditional compilation of locking code
 *
 * Revision 1.4  2002-04-06 22:30:54+05:30  Cprogrammer
 * Remove lock first prior to releasing the lock
 *
 * Revision 1.3  2002-04-04 16:37:08+05:30  Cprogrammer
 * delete lock if get_write_lock fails
 * wrong filename was used to read the pid
 *
 * Revision 1.2  2002-04-01 04:04:59+05:30  Cprogrammer
 * added function readPidLock()
 *
 * Revision 1.1  2002-03-31 21:39:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getDbLock.c,v 2.3 2008-05-28 16:35:34+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
int
getDbLock(char *filename, char proj)
{
#ifdef FILE_LOCKING
	int             lockfd;

	if ((lockfd = lockcreate(filename, proj)) == -1)
	{
		fprintf(stderr, "lockcreate: %s.%d: %s\n", filename, proj, strerror(errno));
		return(-1);
	} else
	if (get_write_lock(lockfd) == -1)
	{
		fprintf(stderr, "get_write_lock: %s.%d: %s\n", filename, proj, strerror(errno));
		delDbLock(lockfd, filename, proj);
		return(-1);
	}
	return(lockfd);
#else
	return(0);
#endif
}

int
delDbLock(int lockfd, char *filename, char proj)
{
#ifdef FILE_LOCKING
	if (RemoveLock(filename, proj) == -1)
	{
		ReleaseLock(lockfd);
		return(-1);
	}
	if (ReleaseLock(lockfd) == -1)
		return(-1);
#endif
	return(0);
}

int
readPidLock(char *filename, char proj)
{
	char            TmpFil[MAX_BUFF];
	int             fd;
	pid_t           pid;

	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if ((fd = open(TmpFil, O_RDONLY, 0)) == -1)
		return(-1);
	if (read(fd, (char *) &pid, sizeof(pid_t)) == -1)
	{
		close(fd);
		return(-1);
	}
	close(fd);
	return(pid);
}

void
getversion_getDbLock_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
