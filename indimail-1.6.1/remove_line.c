/*
 * $Log: remove_line.c,v $
 * Revision 2.3  2009-01-15 08:55:31+05:30  Cprogrammer
 * BUG - fixed all matched lines getting reduced to a single line
 *
 * Revision 2.2  2008-08-02 09:08:31+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.1  2002-08-25 22:34:08+05:30  Cprogrammer
 * made remove_line work on pattern
 *
 * Revision 1.10  2002-04-04 16:40:10+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock().
 *
 * Revision 1.9  2002-04-01 02:11:02+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.8  2002-03-31 21:50:37+05:30  Cprogrammer
 * RemoveLock() after releasing lock
 *
 * Revision 1.7  2002-03-27 01:52:19+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.6  2002-03-25 00:36:00+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.5  2002-03-24 19:17:49+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.4  2002-03-03 15:40:20+05:30  Cprogrammer
 * removed uneccessary variable lockfile
 * Change in ReleaseLock() function
 *
 * Revision 1.3  2001-11-24 12:19:59+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:53+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: remove_line.c,v 2.3 2009-01-15 08:55:31+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Generic remove a line from a file utility
 * input: template to search for
 *        file to search inside
 *
 * output: less than zero on failure
 *         0 if successful
 *         1 if match found
 */
int
remove_line(char *template, char *filename, int once_only, mode_t mode)
{
	int             found;
	char            bak_file[MAX_BUFF], tmpbuf[MAX_BUFF];
	struct stat     statbuf;
	char           *ptr;
	FILE           *fs1, *fs2;
	int             fd;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	if (stat(filename, &statbuf))
	{
		error_stack(stderr, "%s: %s\n", filename, strerror(errno));
		return (-1);
	}
#ifdef FILE_LOCKING
	if((lockfd = getDbLock(filename, 1)) == -1)
	{
		error_stack(stderr, "get_write_lock(%s): %s\n", filename, strerror(errno));
		return(-1);
	} 
#endif
	/*- format a new string */
	snprintf(bak_file, MAX_BUFF, "%s.bak", filename);
	if(rename(filename, bak_file))
	{
		error_stack(stderr, "rename %s->%s: %s\n", filename, bak_file, strerror(errno));
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		return(-1);
	}
	/*- open the file and check for error */
	if (!(fs1 = fopen(filename, "w+")))
	{
		rename(bak_file, filename);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		error_stack(stderr, "fopen(%s, w+: %s\n", filename, strerror(errno));
		return (-1);
	}
	fd = fileno(fs1);
	if(fchmod(fd, mode) || fchown(fd, statbuf.st_uid, statbuf.st_gid))
	{
		rename(bak_file, filename);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		error_stack(stderr, "chmod(%s, %d, %d, %o): %s\n",
			filename, statbuf.st_uid, statbuf.st_gid, mode, 
				strerror(errno));
		return (-1);
	}
	/*- open in read mode and check for error */
	if (!(fs2 = fopen(bak_file, "r+")))
	{
		rename(bak_file, filename);
#ifdef FILE_LOCKING
		delDbLock(lockfd, filename, 1);
#endif
		error_stack(stderr, "fopen(%s, r+): %s\n", filename, strerror(errno));
		fclose(fs1);
		return (-1);
	}
	/*- pound away on the files run the search algorythm */
	for (found = 0;;)
	{
		if(!fgets(tmpbuf, MAX_BUFF, fs2))
			break;
		if((ptr = strchr(tmpbuf, '\n')) != NULL)
			*ptr = 0;
		if (found && once_only)
		{
			fprintf(fs1, "%s\n", tmpbuf);
			continue;
		}
		if (strncmp(template, tmpbuf, slen(template)))
			fprintf(fs1, "%s\n", tmpbuf);
		else
			found++;
	}
	fclose(fs1);
	fclose(fs2);
	unlink(bak_file);
#ifdef FILE_LOCKING
	delDbLock(lockfd, filename, 1);
#endif
	/*
	 * return 0 = everything went okay, but we didn't find it
	 *        1 = everything went okay and we found a match
	 */
	return (found);
}

void
getversion_remove_line_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
