/*
 * $Log: MoveFile.c,v $
 * Revision 1.3  2001-11-24 12:17:21+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:21+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: MoveFile.c,v 1.3 2001-11-24 12:17:21+05:30 Cprogrammer Stab mbhangui $";
#endif

int
MoveFile(const char *src_dir, const char *dest_dir)
{
	DIR            *entry;
	struct dirent  *dp;
	struct utimbuf  ubuf1, ubuf2;
	struct stat     statbuf;
	char            TmpBuf[MAX_BUFF];
	char           *nsrc_dir, *ndest_dir;
	int             len1, len2, status;

	if(stat(src_dir, &statbuf))
	{
		fprintf(stderr, "stat: %s: %s\n", src_dir, strerror(errno));
		return(-1);
	} else
	if(!(entry = opendir(src_dir)))
	{
		fprintf(stderr, "opendir: %s: %s\n", src_dir, strerror(errno));
		return(-1);
	} 
	ubuf1.actime = statbuf.st_atime;
	ubuf1.modtime = statbuf.st_mtime;
	if(access(dest_dir, F_OK) && r_mkdir((char *) dest_dir, statbuf.st_mode, 
		statbuf.st_uid, statbuf.st_gid))
	{
		fprintf(stderr, "r_mkdir: %s: %s\n", dest_dir, strerror(errno));
		closedir(entry);
		return(-1);
	}
	if(!rename(src_dir, dest_dir))
	{
		closedir(entry);
		return(0);
	}
	else
	if(errno != EXDEV)
	{
		closedir(entry);
		return(-1);
	}
	for(nsrc_dir = ndest_dir = 0, status = 0;;)
	{
		if(!(dp = readdir(entry)))
			break;
		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		len1 = slen(dest_dir) + slen(dp->d_name) + 2;
		if(!(ndest_dir = (char *) realloc(ndest_dir, (len1 * sizeof(char)))))
		{
			perror("malloc");
			continue;
		}
		snprintf(ndest_dir, len1, "%s/%s", dest_dir, dp->d_name);
		len2 = slen(src_dir) + slen(dp->d_name) + 2;
		if(!(nsrc_dir = (char *) realloc(nsrc_dir, (len2 * sizeof(char)))))
		{
			perror("malloc");
			continue;
		}
		snprintf(nsrc_dir, len2, "%s/%s", src_dir, dp->d_name);
		if(lstat(nsrc_dir, &statbuf))
		{
			fprintf(stderr, "lstat: %s: %s\n", nsrc_dir, strerror(errno));
			continue;
		}
		ubuf2.actime = statbuf.st_atime;
		ubuf2.modtime = statbuf.st_mtime;
		if (S_ISDIR(statbuf.st_mode))
			status = MoveFile(nsrc_dir, ndest_dir);
		else
		if(S_ISREG(statbuf.st_mode))
			status = fappend(nsrc_dir, ndest_dir, "w", statbuf.st_mode, 
				statbuf.st_uid, statbuf.st_gid);
		if(S_ISCHR(statbuf.st_mode))
			status = mknod(ndest_dir, statbuf.st_mode|S_IFCHR, statbuf.st_dev);
		else
		if(S_ISBLK(statbuf.st_mode))
			status = mknod(ndest_dir, statbuf.st_mode|S_IFBLK, statbuf.st_dev);
		else
		if(S_ISFIFO(statbuf.st_mode))
			status = mknod(ndest_dir, statbuf.st_mode|S_IFIFO, statbuf.st_dev);
		else
		if(S_ISLNK(statbuf.st_mode))
		{
			if((len1 = readlink(nsrc_dir, TmpBuf, MAX_BUFF - 1)) == -1)
			{
				fprintf(stderr, "symlink %s->%s: %s\n", 
					ndest_dir, TmpBuf, strerror(errno));
				continue;
			}
			if(len1 == MAX_BUFF - 1)
			{
				fprintf(stderr, "symlink %s->%s: %s\n", 
					ndest_dir, TmpBuf, strerror(ENAMETOOLONG));
			}
			TmpBuf[len1] = 0;
			if((status = symlink(TmpBuf, ndest_dir)) == -1)
				fprintf(stderr, "symlink: %s->%s: %s\n", 
					ndest_dir, TmpBuf, strerror(errno));
		}
		if(status)
			fprintf(stderr, "copy: %s->%s: %s\n", nsrc_dir, ndest_dir, strerror(errno));
		else
		{
			if (!S_ISDIR(statbuf.st_mode))
				status = unlink(nsrc_dir);
		}
		if(!S_ISLNK(statbuf.st_mode) && utime(ndest_dir, &ubuf2))
			fprintf(stderr, "utime: %s: %s\n", ndest_dir, strerror(errno));
	}
	if(utime(dest_dir, &ubuf1))
		fprintf(stderr, "utime: %s: %s\n", dest_dir, strerror(errno));
	free(ndest_dir);
	if((status = rmdir(src_dir) == -1))
		fprintf(stderr, "rmdir: %s: %s\n", src_dir, strerror(errno));
	free(nsrc_dir);
	closedir(entry);
	return(status);
}

void
getversion_MoveFile_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
