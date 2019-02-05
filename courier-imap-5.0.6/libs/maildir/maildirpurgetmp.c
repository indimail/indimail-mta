/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "maildirquota.h"

#include <sys/types.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<stdlib.h>
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"maildirmisc.h"

static void dopurge(const char *m, unsigned nage,
		    int *nfiles,
		    long *nbytes)
{
time_t	current_time;
DIR	*dirp;
struct dirent *de;

	time (&current_time);
	dirp=opendir(m);
	*nfiles=0;
	*nbytes=0;

	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*z;
	struct	stat	stat_buf;

		if (de->d_name[0] == '.')	continue;
		z=malloc(strlen(m)+strlen(de->d_name)+2);
		if (!z)	continue;

		strcat(strcat(strcpy(z, m), "/"), de->d_name);

		if (stat(z, &stat_buf) == 0
			&& stat_buf.st_ctime < current_time - nage)
		{
			if (maildirquota_countfile(de->d_name))
			{
				++ *nfiles;
				*nbytes += stat_buf.st_size;
			}
			unlink(z);
		}
		free(z);
	}
	if (dirp)	closedir(dirp);
}

void maildir_purgetmp(const char *maildir)
{
	char	*m=malloc(strlen(maildir)+sizeof("/tmp"));
	int nfiles;
	long nbytes;

	if (!m)	return;
	strcat(strcpy(m, maildir), "/tmp");
	dopurge(m, 60 * 60 * 36, &nfiles, &nbytes);
	free(m);
}

void maildir_purge(const char *maildir, unsigned nage)
{
	char	*m=malloc(strlen(maildir)+sizeof("/cur"));
	char	*p;
	int adjustquota;
	int nfiles;
	long nbytes;

	int nfiles2;
	long nbytes2;

	p=strrchr(maildir, '/');
	if (p)
		++p;
	else
		p=".";

	adjustquota=maildirquota_countfolder(p);

	if (!m)	return;
	strcat(strcpy(m, maildir), "/cur");
	dopurge(m, nage, &nfiles, &nbytes);
	strcat(strcpy(m, maildir), "/new");
	dopurge(m, nage, &nfiles2, &nbytes2);
	free(m);

	nfiles += nfiles2;
	nbytes += nbytes2;

	if (adjustquota && nfiles > 0)
		maildir_quota_deleted(maildir, -nbytes, -nfiles);
}
