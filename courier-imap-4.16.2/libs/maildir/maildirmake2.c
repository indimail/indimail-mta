#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<ctype.h>
#include	<errno.h>

#include	"maildirmisc.h"

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

int maildir_make(const char *maildir, int perm, int subdirperm, int folder)
{
	char *q=malloc(strlen(maildir)+sizeof("/maildirfolder"));
	int fd= -1;

	if (!q)
		return -1;

	if (mkdir(maildir, perm) < 0 ||
	    chmod(maildir, perm) < 0 ||
	    mkdir(strcat(strcpy(q, maildir), "/tmp"), subdirperm) < 0 ||
	    chmod(q, subdirperm) < 0 ||
	    mkdir(strcat(strcpy(q, maildir), "/new"), subdirperm) < 0 ||
	    chmod(q, subdirperm) < 0 ||
	    mkdir(strcat(strcpy(q, maildir), "/cur"), subdirperm) < 0 ||
	    chmod(q, subdirperm) < 0 ||
	    (folder && (fd=open(strcat(strcpy(q, maildir), "/maildirfolder"),
				O_CREAT|O_WRONLY, 0600)) < 0))
	{
		free(q);
		return -1;
	}
	if (fd >= 0)
		close(fd);
	free(q);
	return 0;
}

int maildir_del_content(const char *maildir)
{
	char *filenamebuf[100];
	int n, i;
	DIR *dirp;
	struct dirent *de;

	do
	{
		dirp=opendir(maildir);
		n=0;
		while (dirp && (de=readdir(dirp)) != 0)
		{
			if (strcmp(de->d_name, ".") == 0 ||
			    strcmp(de->d_name, "..") == 0)
				continue;

			if ((filenamebuf[n]=malloc(strlen(maildir)+
						   strlen(de->d_name)+2))
			    == NULL)
			{
				closedir(dirp);
				while (n)
					free(filenamebuf[--n]);
				return -1;
			}
			strcat(strcat(strcpy(filenamebuf[n], maildir),
				      "/"), de->d_name);
			if (++n >= sizeof(filenamebuf)/sizeof(filenamebuf[0]))
				break;
		}
		if (dirp)
			closedir(dirp);

		for (i=0; i<n; i++)
		{
			struct stat s_buf;

			if (lstat(filenamebuf[i], &s_buf) < 0)
				continue;

			if (S_ISDIR(s_buf.st_mode))
			{
				if (maildir_del(filenamebuf[i]) < 0)
				{
					while (n)
						free(filenamebuf[--n]);
					return -1;
				}
			}
			else if (unlink(filenamebuf[i]) < 0)
			{
				if (errno != ENOENT)
				{
					while (n)
						free(filenamebuf[--n]);
					return -1;

				}
			}
		}

		for (i=0; i<n; i++)
			free(filenamebuf[i]);
	} while (n);
	return 0; 
}

int maildir_del(const char *maildir)
{
	int rc;
	if ((rc=maildir_del_content(maildir)) == -1)
	    return rc;
	return rmdir(maildir) < 0 && errno != ENOENT ? -1:0;
}
