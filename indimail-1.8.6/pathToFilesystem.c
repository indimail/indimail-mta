/*
 * $Log: pathToFilesystem.c,v $
 * Revision 2.4  2008-07-18 19:10:06+05:30  Cprogrammer
 * removed leftover debug statement
 *
 * Revision 2.3  2008-07-14 19:49:15+05:30  Cprogrammer
 * added port for Mac OS X
 *
 * Revision 2.2  2002-08-11 00:27:22+05:30  Cprogrammer
 * enabled caching of result
 *
 * Revision 2.1  2002-08-10 18:39:56+05:30  Cprogrammer
 * function to get the mounted filesystem for a path
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: pathToFilesystem.c,v 2.4 2008-07-18 19:10:06+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <unistd.h>
#ifdef linux
#include <mntent.h>
#elif defined(sun)
#include <sys/types.h>
#include <sys/mnttab.h>
#elif defined(DARWIN)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

char           *
pathToFilesystem(char *path)
{
	char           *ptr;
	int             pathlen, len;
	static char     tmpbuf[MAX_BUFF], _path[MAX_BUFF];
#ifdef linux
	FILE           *fp;
	struct mntent  *mntptr;
#elif defined(sun)
#define mnt_dir    mnt_mountp
	FILE           *fp;
	struct mnttab   _MntTab;
	struct mnttab  *mntptr = &_MntTab;
#elif defined(DARWIN)
	int             num;
	struct statfs  *mntinf;
#endif

	if(!path || !*path)
		return((char *) 0);
	if(!strncmp(path, _path, MAX_BUFF))
		return(tmpbuf);
	/* 
	 * if directory does not exists, find parent
	 * directory recursively
	 */
	for (ptr = path; access(ptr, F_OK);)
	{
		if (!(ptr = Dirname(ptr)))
			break;
		if (!access(ptr, F_OK))
			break;
	}
	/*- Resolve links and Find the actual path */
	if (!(ptr = getactualpath(ptr)))
		return ((char *) 0);
#ifdef DARWIN
	if (!(num = getmntinfo(&mntinf, MNT_WAIT)))
		return((char *) 0);
	for (*tmpbuf = 0, pathlen = 0;num--;)
	{
		if (strstr(ptr, mntinf->f_mntonname))
		{
			if ((len = strlen(mntinf->f_mntonname)) > pathlen)
			{
				scopy(tmpbuf, mntinf->f_mntonname, MAX_BUFF);
				pathlen = len;
			}
		}
		mntinf++;
	}
#else
	fp = (FILE *) 0;
#ifdef linux
	if (!access("/etc/mtab", F_OK))
		fp = setmntent("/etc/mtab", "r");
#elif defined(sun)
	if (!access("/etc/mnttab", F_OK))
		fp = fopen("/etc/mnttab", "r");
#endif
	if (!fp)
		return ((char *) 0);
	for (*tmpbuf = 0, pathlen = 0;;)
	{
#ifdef linux
		if (!(mntptr = getmntent(fp)))
#elif defined(sun)
		if(getmntent(fp, mntptr))
#endif
			break;
		if (strstr(ptr, mntptr->mnt_dir))
		{
			if ((len = strlen(mntptr->mnt_dir)) > pathlen)
			{
				scopy(tmpbuf, mntptr->mnt_dir, MAX_BUFF);
				pathlen = len;
			}
		}
	}
	fclose(fp);
#endif /*- #ifdef DARWIN */
	scopy(_path, path, MAX_BUFF);
	return (tmpbuf);
}

void
getversion_pathToFilesystem_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
