/*
 * $Log: getFreeFS.c,v $
 * Revision 2.7  2016-05-25 09:01:07+05:30  Cprogrammer
 * use SYSCONFDIR for lastfstab
 *
 * Revision 2.6  2010-08-09 18:28:38+05:30  Cprogrammer
 * use hostid instead of ip address
 *
 * Revision 2.5  2008-05-28 16:35:38+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2005-12-29 22:44:34+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-06-20 16:34:14+05:30  Cprogrammer
 * rename ISOCOR_BASE_PATH to BASE_PATH
 *
 * Revision 2.2  2003-10-23 13:16:54+05:30  Cprogrammer
 * variable LockFile not declared if code is not compiled with locking
 *
 * Revision 2.1  2002-08-11 12:08:52+05:30  Cprogrammer
 * function to balance file system
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getFreeFS.c,v 2.7 2016-05-25 09:01:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char    *getLastFstab();
static int      putLastFstab(char *);

char           *
getFreeFS()
{
	char           *lastSelectedFS, *tmpfstab, *local_hostid, *ptr;
	int             status;
	float           load, prev_load;
	long            cur_user, cur_size, max_user, max_size;
	char            HostID[MAX_BUFF];
	static char     FileSystem[MAX_BUFF];

	lastSelectedFS = getLastFstab();
	if (!(local_hostid = get_local_hostid()))
	{
		fprintf(stderr, "getFreeFilesystem: Could not get local ip: %s\n", strerror(errno));
		return ((char *) 0);
	} else
		scopy(HostID, local_hostid, MAX_BUFF);
	for (*FileSystem = 0, prev_load = -1;;)
	{
		if (!(tmpfstab = vfstab_select(HostID, &status, &max_user, &cur_user, &max_size, &cur_size)))
			break;
		if (status == FS_OFFLINE)
			continue;
		if (lastSelectedFS && !strncmp(tmpfstab, lastSelectedFS, MAX_BUFF))
			continue;
		load = cur_size ? ((float) (cur_user * 1024 * 1024)/ (float) cur_size) : 0.0;
		if (load < prev_load || prev_load == -1.0)
			scopy(FileSystem, tmpfstab, MAX_BUFF);
		prev_load = load;
	}
	if (*FileSystem && !putLastFstab(FileSystem))
		return (FileSystem);
	getEnvConfigStr(&tmpfstab, "BASE_PATH", BASE_PATH);
	if(!(ptr = pathToFilesystem(tmpfstab)))
		return (tmpfstab);
	return(ptr);
}

static char    *
getLastFstab()
{
#ifdef FILE_LOCKING
	int             fd;
#endif
	FILE           *fp;
	char           *ptr, *LockFile = SYSCONFDIR"/lastfstab";
	static char     buffer[MAX_BUFF];

#ifdef FILE_LOCKING
	if ((fd = getDbLock(LockFile, 1)) == -1)
		return ((char *) 0);
#endif
	if (!(fp = fopen(LockFile, "r")))
	{
#ifdef FILE_LOCKING
		delDbLock(fd, LockFile, 1);
#endif
		return ((char *) 0);
	}
	if (!fgets(buffer, sizeof(buffer) - 2, fp))
	{
#ifdef FILE_LOCKING
		delDbLock(fd, LockFile, 1);
#endif
		fclose(fp);
		return ((char *) 0);
	}
	fclose(fp);
#ifdef FILE_LOCKING
	delDbLock(fd, LockFile, 1);
#endif
	if ((ptr = strchr(buffer, '\n')))
		*ptr = 0;
	if (*buffer)
		return (buffer);
	return ((char *) 0);
}

static int
putLastFstab(char *filesystem)
{
	char           *LockFile = SYSCONFDIR"/lastfstab";
#ifdef FILE_LOCKING
	int             fd;
#endif
	FILE           *fp;

#ifdef FILE_LOCKING
	if ((fd = getDbLock(LockFile, 1)) == -1)
		return (-1);
#endif
	if (!(fp = fopen(LockFile, "w")))
	{
#ifdef FILE_LOCKING
		delDbLock(fd, LockFile, 1);
#endif
		return (-1);
	}
	fprintf(fp, "%s", filesystem);
	fclose(fp);
#ifdef FILE_LOCKING
	delDbLock(fd, LockFile, 1);
#endif
	return (0);
}

void
getversion_getFreeFS_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
