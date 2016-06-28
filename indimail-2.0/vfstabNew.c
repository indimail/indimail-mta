/*
 * $Log: vfstabNew.c,v $
 * Revision 2.8  2016-01-12 14:27:18+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.7  2009-10-14 20:47:24+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.6  2008-07-13 19:50:02+05:30  Cprogrammer
 * port for Darwin (Mac OS X)
 *
 * Revision 2.5  2008-05-28 17:41:18+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2005-12-29 22:53:16+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2002-09-30 18:20:31+05:30  Cprogrammer
 * corrected typo for statvfs()
 *
 * Revision 2.2  2002-08-11 14:13:44+05:30  Cprogrammer
 * added AVG_USER_QUOTA to be specified as an environment variable
 *
 * Revision 2.1  2002-08-11 11:30:01+05:30  Cprogrammer
 * function to add local filesystem to fstab
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstabNew.c,v 2.8 2016-01-12 14:27:18+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <mysqld_error.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>

#if defined(sun)
#include <sys/types.h>
#include <sys/statvfs.h>
#elif defined(DARWIN)
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif

#include <sys/socket.h>
#include <string.h>
#include <errno.h>

int
vfstabNew(char *filesystem, long max_user, long max_size)
{
	long            quota_user, quota_size;
	char           *local_ip, *ptr, *avg_user_quota;
#ifdef sun
	struct statvfs  statbuf;
#else
	struct statfs   statbuf;
#endif

	if(max_user == -1 || max_size == -1)
	{
#ifdef sun
		if (statvfs(filesystem, &statbuf))
#else
		if (statfs(filesystem, &statbuf))
#endif
		{
			fprintf(stderr, "statfs: %s: %s\n", filesystem, strerror(errno));
			return (-1);
		}
		if(max_size == -1)
			quota_size = (statbuf.f_bavail * statbuf.f_bsize);
		else
			quota_size = max_size;
		if(max_user == -1)
		{
			getEnvConfigStr(&avg_user_quota, "AVG_USER_QUOTA", AVG_USER_QUOTA);
			quota_user = quota_size / strtoll(avg_user_quota, 0, 0);
		}
		else
			quota_user = max_user;
	} else
	{
		quota_size = max_size;
		quota_user = max_user;
	}
	if(!(local_ip = get_local_ip(PF_INET)))
	{
		fprintf(stderr, "vfstabNew: get_local_ip: %s\n", strerror(errno));
		return(-1);
	}
	if(!(ptr = pathToFilesystem(filesystem)))
	{
		fprintf(stderr, "vfstabNew: %s: Not a filesystem\n", filesystem);
		return(-1);
	}
	return(vfstab_insert(ptr, local_ip, FS_ONLINE, quota_user, quota_size));
}

void
getversion_vfstabNew_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
