/*
** Copyright 2002-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<stdlib.h>
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<ctype.h>
#include	<errno.h>
#include	<stdio.h>
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

#include	"maildirmisc.h"
#include	"maildiraclt.h"

/* Hierarchical rename */

static int validrename(const char *oldname, const char *newname)
{
	size_t l=strlen(oldname);

	if (strncmp(oldname, newname, l) == 0 &&
	    (newname[l] == 0 || newname[l] == '.'))
		return (-1); /* Can't rename to its subdir */

	if (strchr(oldname, '/') ||
	    strchr(newname, '/') ||
	    oldname[0] != '.' ||
	    newname[0] != '.' ||
	    strcmp(newname, ".") == 0)
		return (-1);

	while (*newname)
	{
		if (*newname == '.')
		{
			if (newname[1] == '.' || newname[1] == 0)
				return -1;
		}
		++newname;
	}
	return 0;
}

struct rename_list {
	struct rename_list *next;
	char *o, *n;
};

static int cmp_fn(const void *a, const void *b)
{
	return strcmp( (*(struct rename_list **)a)->o,
		       (*(struct rename_list **)b)->o);
}

static int scan_maildir_rename(const char *, const char *, const char *,
			       int,
			       struct rename_list **);

static int scan_aclhier_rename(const char *, const char *, const char *,
			       int,
			       struct rename_list **);

int maildir_rename(const char *maildir,
		   const char *oldname, const char *newname, int flags,
		   void (*rename_func)(const char *old_path,
				       const char *new_path))
{
	struct rename_list *rl;
	int rc= -1;

	if (validrename(oldname, newname))
	{
		errno=EINVAL;
		return (-1);
	}

	rl=NULL;

	if (scan_maildir_rename(maildir, oldname, newname, flags, &rl) == 0 &&
	    scan_aclhier_rename(maildir, oldname+1, newname+1, flags, &rl)
	    == 0)
	{
		size_t n=0;
		struct rename_list *p, **a;

		for (p=rl; p; p=p->next)
			++n;

		if ((a=malloc(sizeof(struct rename_list *)*(n+1))) != NULL)
		{
			n=0;
			for (p=rl; p; p=p->next)
				a[n++]=p;
			a[n]=NULL;
			if (n)
				qsort(a, n, sizeof(*a), cmp_fn);


			rc=0;
			for (n=0; a[n]; n++)
			{
				if (rename(a[n]->o, a[n]->n))
				{
					rc= -1;
					/* Try to undo the damage */

					while (n)
					{
						--n;
						rename(a[n]->n,
						       a[n]->o);
					}
					break;
				}
				if (rename_func)
					(*rename_func)(a[n]->o,
						       a[n]->n);
			}
			free(a);
		}
	}
	while (rl)
	{
		struct rename_list *p=rl;

		rl=rl->next;
		free(p->o);
		free(p->n);
		free(p);
	}
	return (rc);
}

static int add_rename_nochk(const char *on, const char *nn,
			    struct rename_list **rn)
{
	struct rename_list *p;

	if ((p=malloc(sizeof(struct rename_list))) == NULL)
		return -1;

	if ((p->o=strdup(on)) == NULL)
	{
		free(p);
		return -1;
	}

	if ((p->n=strdup(nn)) == NULL)
	{
		free(p);
		return -1;
	}

	p->next= *rn;
	*rn=p;
	return 0;

}

static int add_rename(const char *on, const char *nn,
		      struct rename_list **rn)
{
	if (access(nn, 0) == 0)
	{
		errno=EEXIST;
		return (-1);	/* Destination folder exists */
	}

	return add_rename_nochk(on, nn, rn);
}

static int scan_maildir_rename(const char *maildir,
			       const char *oldname,
			       const char *newname,
			       int flags,
			       struct rename_list **rename_list_head)
{
	char *new_p;

	int oldname_l=strlen(oldname);
	DIR *dirp;
	struct dirent *de;

	dirp=opendir(maildir);
	while (dirp && (de=readdir(dirp)) != 0)
	{
		char *tst_cur;

		if (de->d_name[0] != '.')
			continue;
		if (strcmp(de->d_name, "..") == 0)
			continue;

		if ((tst_cur=malloc(strlen(maildir) + strlen(de->d_name)
				    + sizeof("//cur")))
		    == NULL)
		{
			closedir(dirp);
			return (-1);
		}
	 	strcat(strcat(strcat(strcpy(tst_cur, maildir), "/"),
			      de->d_name), "/cur");
		if (access(tst_cur, 0))
		{
			free(tst_cur);
			continue;
		}
		if (strncmp(de->d_name, oldname, oldname_l))
		{
			free(tst_cur);
			continue;
		}

		if (de->d_name[oldname_l] == 0)
		{
			if (!(flags & MAILDIR_RENAME_FOLDER))
			{
				free(tst_cur);
				continue;	/* Only the hierarchy */
			}

			strcat(strcat(strcpy(tst_cur, maildir), "/"),
			       de->d_name);

			new_p=malloc(strlen(maildir)+sizeof("/")
				     +strlen(newname));

			if (!new_p)
			{
				free(tst_cur);
				closedir(dirp);
				return (-1);
			}

			strcat(strcat(strcpy(new_p, maildir), "/"), newname);

			if ( add_rename(tst_cur, new_p, rename_list_head))
			{
				free(new_p);
				free(tst_cur);
				closedir(dirp);
				return (-1);
			}
			free(new_p);
			free(tst_cur);
			continue;
		}
		free(tst_cur);

	        if (de->d_name[oldname_l] != '.')
			continue;

		if (!(flags & MAILDIR_RENAME_SUBFOLDERS))
			continue;

		tst_cur=malloc(strlen(maildir) + 
			       strlen(newname) + strlen(de->d_name+oldname_l)
			       + sizeof("/"));

		if (!tst_cur)
		{
			closedir(dirp);
			return (-1);
		}

		strcat(strcat(strcat(strcpy(tst_cur, maildir),
				     "/"), newname), de->d_name+oldname_l);

		new_p=malloc(strlen(maildir) + strlen(de->d_name)
			     + sizeof("/"));

		if (!new_p)
		{
			free(tst_cur);
			closedir(dirp);
			return (-1);
		}

		strcat(strcat(strcpy(new_p, maildir), "/"),
		       de->d_name);

		if ( add_rename(new_p, tst_cur, rename_list_head))
		{
			free(new_p);
			free(tst_cur);
			closedir(dirp);
			return (-1);
		}
		free(new_p);
		free(tst_cur);
	}
	if (dirp) closedir(dirp);
	return (0);
}

static int scan_aclhier2_rename(const char *aclhierdir,
				const char *oldname,
				const char *newname,
				int flags,
				struct rename_list **rename_list_head);

static int scan_aclhier_rename(const char *maildir,
			       const char *oldname,
			       const char *newname,
			       int flags,
			       struct rename_list **rename_list_head)
{

	char *aclhier=malloc(strlen(maildir)+sizeof("/" ACLHIERDIR));
	int rc;

	if (!aclhier)
		return -1;

	rc=scan_aclhier2_rename(strcat(strcpy(aclhier, maildir),
				       "/" ACLHIERDIR), oldname, newname,
				flags, rename_list_head);

	free(aclhier);
	return rc;
}

static int scan_aclhier2_rename(const char *aclhierdir,
				const char *oldname,
				const char *newname,
				int flags,
				struct rename_list **rename_list_head)
{
	char *new_p;

	int oldname_l=strlen(oldname);
	DIR *dirp;
	struct dirent *de;

	dirp=opendir(aclhierdir);
	while (dirp && (de=readdir(dirp)) != 0)
	{
		char *tst_cur;

		if (de->d_name[0] == '.')
			continue;

		if (strncmp(de->d_name, oldname, oldname_l))
		{
			continue;
		}

		if ((tst_cur=malloc(strlen(aclhierdir) + strlen(de->d_name)
				    + sizeof("/")))
		    == NULL)
		{
			closedir(dirp);
			return (-1);
		}

		if (de->d_name[oldname_l] == 0)
		{
			if (!(flags & MAILDIR_RENAME_FOLDER))
			{
				free(tst_cur);
				continue;	/* Only the hierarchy */
			}

			strcat(strcat(strcpy(tst_cur, aclhierdir), "/"),
			       de->d_name);

			new_p=malloc(strlen(aclhierdir)+sizeof("/")
				     +strlen(newname));

			if (!new_p)
			{
				free(tst_cur);
				closedir(dirp);
				return (-1);
			}

			strcat(strcat(strcpy(new_p, aclhierdir), "/"), newname);

			if ( add_rename_nochk(tst_cur, new_p,
					      rename_list_head))
			{
				free(new_p);
				free(tst_cur);
				closedir(dirp);
				return (-1);
			}
			free(new_p);
			free(tst_cur);
			continue;
		}
		free(tst_cur);

	        if (de->d_name[oldname_l] != '.')
			continue;

		if (!(flags & MAILDIR_RENAME_SUBFOLDERS))
			continue;

		tst_cur=malloc(strlen(aclhierdir) + 
			       strlen(newname) + strlen(de->d_name+oldname_l)
			       + sizeof("/"));

		if (!tst_cur)
		{
			closedir(dirp);
			return (-1);
		}

		strcat(strcat(strcat(strcpy(tst_cur, aclhierdir),
				     "/"), newname), de->d_name+oldname_l);

		new_p=malloc(strlen(aclhierdir) + strlen(de->d_name)
			     + sizeof("/"));

		if (!new_p)
		{
			free(tst_cur);
			closedir(dirp);
			return (-1);
		}

		strcat(strcat(strcpy(new_p, aclhierdir), "/"),
		       de->d_name);

		if ( add_rename_nochk(new_p, tst_cur, rename_list_head))
		{
			free(new_p);
			free(tst_cur);
			closedir(dirp);
			return (-1);
		}
		free(new_p);
		free(tst_cur);
	}
	if (dirp) closedir(dirp);
	return (0);
}
