/*
** Copyright 2000-2002 Double Precision, Inc.
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


/*
** char *maildir_filename(const char *maildir, const char *folder,
**   const char *filename)
**	- find a message in a maildir
**
** Return the full path to the indicated message.  If the message flags
** in filename have changed, we search the maildir for this message.
*/

char *maildir_filename(const char *maildir,
	const char *folder, const char *filename)
{
struct stat stat_buf;
char	*p, *q;
DIR *dirp;
struct dirent *de;
char	*dir;

	if (strchr(filename, '/') || *filename == '.')
	{
		errno=ENOENT;
		return (0);
	}

	dir=maildir_folderdir(maildir, folder);

	if (!dir)	return (0);

	p=malloc(strlen(dir)+strlen(filename)+sizeof("/cur/"));

	if (!p)
	{
		free(dir);
		return (0);
	}

	strcat(strcat(strcpy(p, dir), "/cur/"), filename);

	if (stat(p, &stat_buf) == 0)
	{
		free(dir);
		return (p);
	}

	/* Oh, a wise guy... */

	q=strrchr(p, '/');
	*q=0;
	dirp=opendir(p);
	*q='/';

	if ( dirp == NULL)
	{
		free(dir);
		return p;
	}

	/* Compare filenames, ignore filename size if set by maildirquota */

	while ((de=readdir(dirp)) != NULL)
	{
	const char *a=filename;
	const char *b=de->d_name;

		for (;;)
		{
			if ( a[0] == ',' && a[1] == 'S' && a[2] == '=')
			{
				/* File size - quota shortcut - skip */
				a += 3;
				while (*a && isdigit((int)(unsigned char)*a))
					++a;
			}

			if ( b[0] == ',' && b[1] == 'S' && b[2] == '=')
			{
				/* File size - quota shortcut - skip */
				b += 3;
				while (*b && isdigit((int)(unsigned char)*b))
					++b;
			}

			if ( (*a == 0 || *a == MDIRSEP[0]) && (*b == 0 || *b == MDIRSEP[0]))
			{
				free(p);
				p=malloc(strlen(dir)+strlen(de->d_name)+
					sizeof("/cur/"));
				if (!p)
				{
					closedir(dirp);
					free(dir);
					return (0);
				}

				strcat(strcat(strcpy(p, dir), "/cur/"),
					de->d_name);
				closedir(dirp);
				free(dir);
				return (p);
			}
			if ( *a == 0 || *a == MDIRSEP[0] || *b == 0 || *b == MDIRSEP[0] ||
				*a != *b)
				break;

			++a;
			++b;
		}
	}
	closedir(dirp);
	free(dir);
	return (p);
}
