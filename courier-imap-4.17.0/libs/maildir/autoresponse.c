/*
** Copyright 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"autoresponse.h"
#include	"autoresponsequota.h"
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
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


struct maildir_autoresponse_quota {
	unsigned files;
	unsigned long bytes;
} ;

struct temp_autoresponse_list {
	struct temp_autoresponse_list *next;
	char *filename;
} ;

char **maildir_autoresponse_list(const char *maildir)
{
	char *d, **a;
	struct temp_autoresponse_list *list=NULL;
	unsigned list_cnt;
	struct temp_autoresponse_list *p;

	DIR *dirp;

	if (!maildir)
		maildir=".";

	d=malloc(strlen(maildir)+sizeof("/autoresponses"));

	if (!d)
		return (NULL);

	strcat(strcpy(d, maildir), "/autoresponses");

	dirp=opendir(d);
	free(d);

	list_cnt=0;

	if (dirp)
	{
		struct dirent *de;

		while ((de=readdir(dirp)) != NULL)
		{
			if (strchr(de->d_name, '.'))
				continue;

			p=(struct temp_autoresponse_list *)
				malloc(sizeof(struct temp_autoresponse_list));
			if (p)
			{
				if ((p->filename=strdup(de->d_name)) == NULL)
				{
					free(p);
					p=NULL;
				}
			}

			if (!p)
			{
				closedir(dirp);

				while (list)
				{
					p=list;
					list=p->next;
					free(p->filename);
					free(p);
				}
				return (NULL);
			}
			p->next=list;
			list=p;
			++list_cnt;
		}
		closedir(dirp);
	}

	a=malloc( (list_cnt+1)*sizeof(char *));

	if (!a)
	{
		while (list)
		{
			p=list;
			list=p->next;
			free(p->filename);
			free(p);
		}
		return (NULL);
	}

	list_cnt=0;

	while (list)
	{
		p=list;
		list=p->next;
		a[list_cnt]=p->filename;
		free(p);
		++list_cnt;
	}
	a[list_cnt]=0;
	return (a);
}

void maildir_autoresponse_list_free(char **a)
{
	unsigned i;

	for (i=0; a[i]; i++)
		free(a[i]);
	free(a);
}

static char *afilename(const char *maildir, const char *filename)
{
	char *p;

	if (!maildir)
		maildir=".";

	if (strchr(filename, '.') || strchr(filename, '/')
	    || strchr(filename, '\'') || strchr(filename, '\"')
	    || strchr(filename, '*') || strchr(filename, '?')
	    || strchr(filename, '[') || strchr(filename, ']')
	    || strchr(filename, ' ') || strchr(filename, '\n')
	    || strchr(filename, '\t') || strchr(filename, '\r')
	    || strchr(filename, '~') || !*filename)
	{
		errno=EINVAL;
		return (NULL);
	}

	p=malloc(strlen(maildir)+strlen(filename)+
		 sizeof("/autoresponsesXXXXXXXXXXXXXXXXXXXXXXXX"));

	if (!p)
		return (NULL);
	return (strcat(strcat(strcpy(p, maildir), "/autoresponses/"),
		       filename));
}

int maildir_autoresponse_validate(const char *maildir, const char *filename)
{
	char *p=afilename(maildir, filename);

	if (!p)
		return (-1);
	free(p);
	return (0);
}

/* Delete autoreply scratch file (optionally the autoreply file itself) */

static void deletefiles(const char *dir, const char *filename, int deleteall)
{
	DIR *dirp=opendir(dir);
	struct dirent *de;
	int l=strlen(filename);

	if (!dirp)
		return;

	while ((de=readdir(dirp)) != 0)
	{
		char *q;

		if (strncmp(de->d_name, filename, l))
			continue;

		if (de->d_name[l] == 0)
		{
			if (!deleteall)
				continue;
		}
		else if (de->d_name[l] != '.')
			continue;

		q=malloc(strlen(dir)+strlen(de->d_name)+2);

		if (q)
		{
			unlink(strcat(strcat(strcpy(q, dir), "/"),de->d_name));
			free(q);
		}
	}
	closedir(dirp);
}

void maildir_autoresponse_delete(const char *maildir, const char *filename)
{
	char *p=afilename(maildir, filename);

	char *q;

	if (!p)
		return;

	q=strrchr(p, '/');
	*q++=0;

	deletefiles(p, q, 1);
	free(p);
}

static void read_quota(struct maildir_autoresponse_quota *q, const char *f)
{
	char buf[BUFSIZ];
	FILE *fp;
	const char *p;

	if ((fp=fopen(f, "r")) == NULL)
		return;
	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
		fclose(fp);
		return;
	}
	fclose(fp);

	for (p=buf; *p; )
	{
		if (*p == 'C')
		{
			q->files=0;
			for ( ++p; *p; ++p)
			{
				if (!isdigit((int)(unsigned char)*p))
					break;
				q->files=q->files * 10 + (*p-'0');
			}
			continue;
		}

		if (*p == 'S')
		{
			q->bytes=0;
			for ( ++p; *p; ++p)
			{
				if (!isdigit((int)(unsigned char)*p))
					break;
				q->bytes=q->bytes * 10 + (*p-'0');
			}
			continue;
		}
		++p;
	}
}

static int get_quota(struct maildir_autoresponse_quota *q, const char *maildir)
{
	char *p;

	q->files=0;
	q->bytes=0;
	read_quota(q, AUTORESPONSEQUOTA);

	if (!maildir)
		maildir=".";

	p=malloc(strlen(maildir)+sizeof("/autoresponsesquota"));
	if (!p)
		return (-1);
	strcat(strcpy(p, maildir), "/autoresponsesquota");
	read_quota(q, p);
	free(p);
	return (0);
}

static void add_quota(struct maildir_autoresponse_quota *q, const char *file, int sign)
{
	struct stat stat_buf;

	if (stat(file, &stat_buf))
		return;
	q->files += sign;
	q->bytes += (long)stat_buf.st_size*sign;
}

static int calc_quota(struct maildir_autoresponse_quota *q, const char *maildir)
{
	char *p;
	DIR *dirp;
	struct dirent *de;

	q->files=0;
	q->bytes=0;

	if (!maildir)
		maildir=".";

	p=malloc(strlen(maildir)+sizeof("/autoresponses"));
	if (!p)
		return (-1);
	strcat(strcpy(p, maildir), "/autoresponses");
	dirp=opendir(p);
	free(p);
	if (!dirp)
		return (0);
	while ((de=readdir(dirp)) != 0)
	{
		if (strchr(de->d_name, '.'))
			continue;

		p=malloc(strlen(maildir)+strlen(de->d_name)
			 +sizeof("/autoresponses/"));
		if (!p)
		{
			closedir(dirp);
			return (-1);
		}

		strcat(strcat(strcpy(p, maildir), "/autoresponses/"),
		       de->d_name);
		add_quota(q, p, 1);
		free(p);
	}
	closedir(dirp);
	return (0);
}

static int check_quota(struct maildir_autoresponse_quota *setquota,
		       struct maildir_autoresponse_quota *newquota)
{
	if (setquota->files > 0 && newquota->files > setquota->files)
		return (-1);
	if (setquota->bytes > 0 && newquota->bytes > setquota->bytes)
		return (-1);
	return (0);
}

FILE *maildir_autoresponse_create(const char *maildir, const char *filename)
{
	char *p=afilename(maildir, filename);
	FILE *fp;
	char *q;

	if (!p)
		return (NULL);

	strcat(p, ".tmp");
	fp=fopen(p, "w");

	if (!fp)	/* Perhaps we need to create the autoresponse dir? */
	{
		q=strrchr(p, '/');
		*q=0;
		mkdir(p, 0700);
		*q='/';
		fp=fopen(p, "w");
	}
	free(p);
	return (fp);
}

int maildir_autoresponse_create_finish(const char *maildir, const char *filename,
			       FILE *fp)
{
	char *p, *q;
	struct maildir_autoresponse_quota set_quota, new_quota;

	fclose(fp);
	p=afilename(maildir, filename);

	if (!p)
		return (0);
	q=strdup(p);

	if (q)
	{
		if (get_quota(&set_quota, maildir)
		    || calc_quota(&new_quota, maildir))
		{
			strcat(p, ".tmp");
			unlink(p);
			free(q);
			free(p);
			return (-1);
		}

		add_quota(&new_quota, p, -1);
		strcat(p, ".tmp");
		add_quota(&new_quota, p, 1);
		if (check_quota(&set_quota, &new_quota))
		{
			unlink(p);
			free(p);
			free(q);
			errno=ENOSPC;
			return (-1);
		}

		rename(p, q);
		free(q);
	}
	free(p);
	return (0);
}

FILE *maildir_autoresponse_open(const char *maildir, const char *filename)
{
	char *p=afilename(maildir, filename);
	FILE *fp;

	if (!p)
		return (NULL);

	fp=fopen(p, "r");
	free(p);
	return (fp);
}
