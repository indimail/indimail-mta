/*
 * $Log: vadduserl.c,v $
 * Revision 2.2  2016-05-25 09:07:17+05:30  Cprogrammer
 * use CONTROLDIR for lock file
 *
 * Revision 2.1  2003-10-23 13:21:53+05:30  Cprogrammer
 * conditional compilation for locking code
 *
 * Revision 1.9  2002-04-04 16:41:11+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.8  2002-04-01 02:11:12+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.7  2002-03-31 21:51:24+05:30  Cprogrammer
 * RemoveLock after releasing lock
 *
 * Revision 1.6  2002-03-27 01:53:20+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.5  2002-03-25 00:36:37+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.4  2002-03-24 19:13:56+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.3  2001-11-24 12:20:29+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:23+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: vadduserl.c,v 2.2 2016-05-25 09:07:17+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef FILE_LOCKING
int
main(int argc, char **argv)
{
	char            path[MAX_BUFF];
	char           *lockfile = CONTROLDIR"/adduser";
	int             len, fd;

	scopy(path, argv[0], MAX_BUFF);
	len = slen(path);
	if (path[len - 1] != 'l')
		return (1);
	if ((fd = getDbLock(lockfile, 1)) == -1)
		return (1);
	path[len - 1] = 0;
	execv(path, argv);
	delDbLock(fd, lockfile, 1);
	return(1);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-file-locking=y\n");
	return(0);
}
#endif

void
getversion_vadduserl_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
