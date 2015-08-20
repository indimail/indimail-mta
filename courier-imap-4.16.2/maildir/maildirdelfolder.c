/*
** Copyright 2000-2003 Double Precision, Inc.
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

#include	"maildirmisc.h"

int maildir_deletefolder(const char *maildir, const char *folder)
{
char	*s;
int	rc;

	if (*folder == '.')
	{
		errno=EINVAL;
		return (-1);
	}

	s=malloc(strlen(maildir)+strlen(folder)+3);
	if (!s)	return (-1);
	strcat(strcat(strcpy(s, maildir), "/."), folder);

	rc=maildir_del(s);
	free(s);
	return (rc);
}
