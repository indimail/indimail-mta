/*
 * $Log: update_file.c,v $
 * Revision 2.4  2009-01-13 14:41:50+05:30  Cprogrammer
 * added documentation
 *
 * Revision 2.3  2008-08-02 09:08:48+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2006-06-28 14:21:38+05:30  Cprogrammer
 * attempt to set ownership of files updated/created to original/indimail id
 *
 * Revision 2.1  2004-07-03 22:54:42+05:30  Cprogrammer
 * added missing release lock
 *
 * Revision 1.11  2002-04-04 16:40:32+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.10  2002-04-01 02:11:05+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.9  2002-03-31 21:50:57+05:30  Cprogrammer
 * RemoveLock() after releasing lock
 *
 * Revision 1.8  2002-03-27 01:52:41+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.7  2002-03-25 00:36:09+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.6  2002-03-24 19:17:59+05:30  Cprogrammer
 * addtional argument to get_write_lock()
 *
 * Revision 1.5  2002-03-03 15:40:59+05:30  Cprogrammer
 * removed unecessary variable lockfile
 * Change in ReleaseLock() function
 *
 * Revision 1.4  2001-12-01 11:15:45+05:30  Cprogrammer
 * bug when file is not existing. Create a 0 bytes file
 *
 * Revision 1.3  2001-11-24 12:20:10+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:08+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: update_file.c,v 2.4 2009-01-13 14:41:50+05:30 Cprogrammer Stab mbhangui $";
#endif

int
update_file(filename, update_line, mode)
	char           *filename;
	char           *update_line;
	mode_t          mode;
{
	FILE           *fs1 = NULL, *fs2 = NULL;
	char           *ptr;
	static char     fname[MAX_BUFF], tmpbuf[MAX_BUFF];
	int             found, user_assign = 0;
	struct stat     statbuf;
#ifdef FILE_LOCKING
	int             fd;
#endif

#ifdef FILE_LOCKING
	if ((fd = getDbLock(filename, 1)) == -1)
	{
		error_stack(stderr, "get_write_lock: %s: %s\n", filename, strerror(errno));
		return (-1);
	}
#endif
	snprintf(fname, MAX_BUFF, "%s.%d", filename, (int) getpid());
	if (!(fs2 = fopen(fname, "w+")))
	{
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		error_stack(stderr, "fopen: %s: %s\n", fname, strerror(errno));
		return (-1);
	}
	if (fchmod(fileno(fs2), mode))
	{
		error_stack(stderr, "fchmod: %s - %o: %s\n", fname, mode, strerror(errno));
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		return (-1);
	}
	if (stat(filename, &statbuf))
	{
		if (errno != ENOENT)
		{
			error_stack(stderr, "stat: %s : %s\n", filename, strerror(errno));
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			return (-1);
		}
		if (GetIndiId(&statbuf.st_uid, &statbuf.st_gid))
		{
			error_stack(stderr, "GetIndiId: failed\n");
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			return (-1);
		}
	}
	if (fchown(fileno(fs2), statbuf.st_uid, statbuf.st_gid))
	{
		error_stack(stderr, "fchown: %s - %o: %s\n", fname, mode, strerror(errno));
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		return (-1);
	}
	if (access(filename, F_OK) && !(fs1 = fopen(filename, "a")))
	{
		perror(filename);
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		return(-1);
	} else
	if (fs1)
		fclose(fs1);
	if ((fs1 = fopen(filename, "r+")) != (FILE *) 0)
	{
		for (found = 0;;) 
		{
			if (!fgets(tmpbuf, MAX_BUFF, fs1))
				break;
			if ((ptr = strrchr(tmpbuf, '\n')) != NULL)
				*ptr = 0;
			if (!strcmp(tmpbuf, "."))
			{
				user_assign = 1;
				continue;
			}
			if (!strcmp(tmpbuf, update_line))
			{
				found = 1;
				fprintf(fs2, "%s\n", update_line);
			} else
				fprintf(fs2, "%s\n", tmpbuf);
		}
		fclose(fs1);
		if (!found) /*- Append */
			fprintf(fs2, "%s\n", update_line);
	} else
		perror(filename);
	if (user_assign)
		fprintf(fs2, ".\n");
	fclose(fs2);
	if (rename(fname, filename))
	{
		error_stack(stderr, "rename: %s->%s: %s\n", fname, filename, strerror(errno));
#ifdef FILE_LOCKING
		delDbLock(fd, filename, 1);
#endif
		return(-1);
	}
#ifdef FILE_LOCKING
	delDbLock(fd, filename, 1);
#endif
	return (0);
}

void
getversion_update_file_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
