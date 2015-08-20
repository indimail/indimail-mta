/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

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
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<fcntl.h>

#include	"maildirmisc.h"

void maildir_list(const char *maildir,
		  void (*func)(const char *, void *),
		  void *voidp)
{
	DIR *dirp=opendir(maildir);
	struct dirent *de;

	while (dirp && (de=readdir(dirp)) != NULL)
	{
		char *p;

		if (strcmp(de->d_name, "..") == 0)
			continue;

		if (de->d_name[0] != '.')
			continue;

		if ((p=malloc(strlen(maildir) + strlen(de->d_name)+20))
		    == NULL)
			continue;

		strcat(strcat(strcat(strcpy(p, maildir), "/"), de->d_name),
		       "/cur/.");

		if (access(p, X_OK))
		{
			free(p);
			continue;
		}

		strcpy(p, INBOX);

		if (strcmp(de->d_name, "."))
			strcat(p, de->d_name);

		(*func)(p, voidp);
		free(p);
	}
	if (dirp)
		closedir(dirp);
}
