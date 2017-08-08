/*
** Copyright 2000-2004 Double Precision, Inc.
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
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<time.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"maildirmisc.h"


static void do_maildir_getnew(const char *, const char *,
			      void (*)(const char *, void *),
			      void *);

void maildir_getnew(const char *maildir, const char *folder,
		    void (*callback_func)(const char *, void *),
		    void *callback_arg)
{
char	*dir=maildir_folderdir(maildir, folder);
char	*newd, *curd;

	if (!dir)	return;

	newd=malloc(strlen(dir)+sizeof("/new"));
	curd=malloc(strlen(dir)+sizeof("/cur"));

	if (newd && curd)
	{
		strcat(strcpy(newd, dir), "/new");
		strcat(strcpy(curd, dir), "/cur");
		do_maildir_getnew(newd, curd, callback_func, callback_arg);
	}

	if (newd)	free(newd);
	if (curd)	free(curd);
	free(dir);
}

static void do_maildir_getnew(const char *newd, const char *curd,
			      void (*callback_func)(const char *, void *),
			      void *callback_arg)
{
	DIR	*dirp;
	struct dirent *de;
	int keepgoing;
	int n;
	char *new_buf[20];
	char *cur_buf[20];

	do
	{
		keepgoing=0;
		n=0;

		dirp=opendir(newd);
		while (dirp && (de=readdir(dirp)) != 0)
		{
			char	*np, *cp;

			if (de->d_name[0] == '.')	continue;

			if ((np=malloc(strlen(newd)+strlen(de->d_name)+2))
			    != 0)
			{
				if ((cp=malloc(strlen(curd)+strlen(de->d_name)
					       + sizeof("/" MDIRSEP "2,")))
				    != 0)
				{
					char *a;

					strcat(strcat(strcpy(np, newd), "/"),
					       de->d_name);
					strcat(strcat(strcpy(cp, curd), "/"),
					       de->d_name);
					a=strchr(cp+strlen(curd), MDIRSEP[0]);
					if (a && strncmp(a, MDIRSEP "2,", 3))
					{
						*a=0;
						a=0;
					}
					if (!a)	strcat(cp, MDIRSEP "2,");
					new_buf[n]=np;
					cur_buf[n]=cp;

					if (++n >= sizeof(cur_buf)/
					    sizeof(cur_buf[0]))
					{
						keepgoing=1;
						break;
					}
				}
				else
					free(np);
			}
		}
		if (dirp)	closedir(dirp);

		while (n)
		{
			char *np, *cp;

			--n;

			np=new_buf[n];
			cp=cur_buf[n];

			if (rename(np, cp))
			{
				fprintf(stderr,
					"ERR: rename(%s,%s) failed:"
					" %s\n", np, cp, strerror(errno));
				keepgoing=0;
				/* otherwise we could have infinite loop */
			}

			if (callback_func)
				(*callback_func)(strrchr(cp, '/')+1,
						 callback_arg);
			free(np);
			free(cp);
		}
	} while (keepgoing);
}
