/*
** Copyright 2000-2007 Double Precision, Inc.
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
#include	"maildircreate.h"
#include	"maildirsharedrc.h"


/* Prerequisited for shared folder support */

#if	HAVE_READLINK
#if	HAVE_SYMLINK
#if	HAVE_DBOBJ

#define	YES_WE_CAN_DO_SHARED	1

#endif
#endif
#endif

#if	YES_WE_CAN_DO_SHARED

#include	"dbobj.h"

static void list_sharable(const char *, const char *,
	void (*)(const char *, void *),
	void *);

extern FILE *maildir_shared_fopen(const char *, const char *);
extern void maildir_shared_fparse(char *, char **, char **);

void maildir_list_sharable(const char *maildir,
	void (*func)(const char *, void *),
	void *voidp)
{
char	buf[BUFSIZ];
FILE	*fp;
char	*p;
int	pass;

	if (!maildir)	maildir=".";

	for (pass=0; pass<2; pass++)
	{
		fp=pass ? maildir_shared_fopen(maildir, "r")
			: fopen (MAILDIRSHAREDRC, "r");

		if (!fp)	continue;

		while ((p=fgets(buf, sizeof(buf), fp)) != 0)
		{
		char	*name, *dir;

			maildir_shared_fparse(p, &name, &dir);
			if (name)
				list_sharable(name, dir, func, voidp);
		}
		fclose(fp);
	}
}

static void list_sharable(const char *pfix, const char *path,
	void (*func)(const char *, void *),
	void *voidp)
{
DIR	*dirp;
struct	dirent *de;
struct	stat	stat_buf;

	dirp=opendir(path);
	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*z;

		if (de->d_name[0] != '.')	continue;
		if (strcmp(de->d_name, ".") == 0 ||
			strcmp(de->d_name, "..") == 0)	continue;

		z=malloc(strlen(path)+strlen(de->d_name)+12);
		if (!z)	continue;

		strcat(strcat(strcat(strcpy(z, path),
			"/"), de->d_name), "/cur/.");

		if (stat(z, &stat_buf))
		{
			free(z);
			continue;
		}
		free(z);
		z=malloc(strlen(pfix)+strlen(de->d_name)+1);
		if (!z)	continue;
		strcat(strcpy(z, pfix), de->d_name);
		(*func)(z, voidp);
		free(z);
	}
	if (dirp)	closedir(dirp);
}

int maildir_shared_subscribe(const char *maildir, const char *folder)
{
char	linebuf[BUFSIZ];
FILE	*fp;
char	*p;
char	*name=strchr(folder, '.');
char	*s, *n, *dir;
char	*buf, *link;
unsigned l;
int	pass;

	if (!name)
	{
		errno=EINVAL;
		return (-1);
	}

	if (!maildir)	maildir=".";
	p=maildir_shareddir(maildir, folder);	/* valid folder name? */
	if (!p)
	{
		errno=EINVAL;
		return (-1);
	}
	free(p);

	p=0;

	for (pass=0; pass<2; pass++)
	{
		fp=pass ? maildir_shared_fopen(maildir, "r")
			: fopen (MAILDIRSHAREDRC, "r");

		if (!fp)	continue;

		while ((p=fgets(linebuf, sizeof(linebuf), fp)) != 0)
		{
			maildir_shared_fparse(p, &n, &dir);

			if (!n)	continue;

			if (strlen(n) == name - folder &&
				memcmp(n, folder, name-folder) == 0)	break;
		}
		fclose(fp);

		if (p)	break;
	}

	if (p)
	{
		/*
		** We will create:
		**
		**  maildir/shared-folders/ (name-folder) /(name)
		**
		**  there we'll have subdirs cur/new/tmp  and shared link
		*/

		l=sizeof("/" SHAREDSUBDIR "//shared") +
			strlen(maildir) + strlen(folder);
		buf=malloc(l);
		if (!buf)	return (-1);
		strcat(strcpy(buf, maildir), "/" SHAREDSUBDIR);
		mkdir(buf, 0700);
		strcat(buf, "/");
		strncat(buf, folder, name-folder);
		mkdir(buf, 0700);
		strcat(buf, "/");
		strcat(buf, name+1);
		if ( mkdir(buf, 0700))	return (-1);
		s=buf+strlen(buf);
		*s++='/';
		strcpy(s, "tmp");
		if ( mkdir(buf, 0700))
		{
			s[-1]=0;
			rmdir(buf);
			free(buf);
			return (-1);
		}
		strcpy(s, "cur");
		if ( mkdir(buf, 0700))
		{
			strcpy(s, "tmp");
			rmdir(buf);
			s[-1]=0;
			rmdir(buf);
			free(buf);
			return (-1);
		}
		strcpy(s, "new");
		if ( mkdir(buf, 0700))
		{
			strcpy(s, "cur");
			rmdir(buf);
			strcpy(s, "tmp");
			rmdir(buf);
			s[-1]=0;
			rmdir(buf);
			free(buf);
			return (-1);
		}

		strcpy(s, "shared");
		if ((link=malloc(strlen(dir)+strlen(name)+2)) == 0 ||
			symlink( strcat(strcat(strcpy(link, dir), "/"), name),
				buf))
		{
			if (link)	free(link);
			strcpy(s, "new");
			rmdir(buf);
			strcpy(s, "cur");
			rmdir(buf);
			strcpy(s, "tmp");
			rmdir(buf);
			s[-1]=0;
			rmdir(buf);
			free(buf);
			return (-1);
		}
		free(link);
		free(buf);
		return (0);
	}
	errno=ENOENT;
	return (-1);
}

void maildir_list_shared(const char *maildir,
	void (*func)(const char *, void *),
	void *voidp)
{
char	*sh;
DIR	*dirp;
struct	dirent *de;

	if (!maildir)	maildir=".";
	sh=malloc(strlen(maildir)+sizeof("/" SHAREDSUBDIR));
	if (!sh)	return;

	strcat(strcpy(sh, maildir), "/" SHAREDSUBDIR);

	dirp=opendir(sh);
	while (dirp && (de=readdir(dirp)) != 0)
	{
	DIR	*dirp2;
	struct	dirent *de2;
	char	*z;

		if (de->d_name[0] == '.')	continue;

		z=malloc(strlen(sh)+strlen(de->d_name)+2);
		if (!z)	continue;
		strcat(strcat(strcpy(z, sh), "/"), de->d_name);
		dirp2=opendir(z);
		free(z);

		while (dirp2 && (de2=readdir(dirp2)) != 0)
		{
		char	*s;

			if (de2->d_name[0] == '.')	continue;
			s=malloc(strlen(de->d_name)+strlen(de2->d_name)+2);
			if (!s)	continue;
			strcat(strcat(strcpy(s, de->d_name), "."), de2->d_name);
			(*func)(s, voidp);
			free(s);
		}
		if (dirp2)	closedir(dirp2);
	}
	free(sh);
	if (dirp)	closedir(dirp);
}

int maildir_shared_unsubscribe(const char *maildir, const char *folder)
{
char	*s;

	s=maildir_shareddir(maildir, folder);
	if (!s)	return (-1);

	if (maildir_del(s))
	{
		free(s);
		return (-1);
	}
	*strrchr(s, '/')=0;	/* Try to remove the whole folder dir */
	rmdir(s);
	free(s);
	return (0);
}

/*                    LET'S SYNC IT                  */

static void do_maildir_shared_sync(const char *, const char *);

void maildir_shared_sync(const char *dir)
{
char	*shareddir;
char	*buf;

	shareddir=malloc(strlen(dir)+sizeof("/shared"));
	if (!shareddir)
	{
		perror("malloc");
		return;
	}
	strcat(strcpy(shareddir, dir),"/shared");

	buf=maildir_getlink(shareddir);
	free(shareddir);
	if (buf)
	{
		do_maildir_shared_sync(dir, buf);
		free(buf);
	}
}

/* Step 1 - safely create a temporary database */

static int create_db(struct dbobj *obj,
	const char *dir,
	char **dbname)
{
	struct maildir_tmpcreate_info createInfo;

	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=dir;
	createInfo.uniq="sync";
	createInfo.doordie=1;

	{
		int	fd;

		fd=maildir_tmpcreate_fd(&createInfo);

		if (fd < 0)
		{
			perror(dir);
			return -1;
		}
		close(fd);

		dbobj_init(obj);
		if (dbobj_open(obj, createInfo.tmpname, "N") < 0)
		{
			perror(createInfo.tmpname);
			unlink(createInfo.tmpname);
			maildir_tmpcreate_free(&createInfo);
			return (-1);
		}
	}

	*dbname=createInfo.tmpname;
	createInfo.tmpname=NULL;
	maildir_tmpcreate_free(&createInfo);
	return (0);
}

/*
** Populate the DB by building the db with the messages in the sharable
** folder's cur.  The key is the stripped message filename, the value is
** the complete message filename.
*/

static int build_db(const char *shared, struct dbobj *obj)
{
char	*dummy=malloc(strlen(shared)+sizeof("/cur"));
DIR	*dirp;
struct	dirent *de;

	if (!dummy)
	{
		perror("malloc");
		return (-1);
	}

	strcat(strcpy(dummy, shared), "/cur");

	dirp=opendir(dummy);
	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*a, *b;
	char	*c;

		if (de->d_name[0] == '.')
			continue;
		if ((a=malloc(strlen(de->d_name)+1)) == 0)
		{
			perror("malloc");
			closedir(dirp);
			free(dummy);
			return (-1);
		}
		if ((b=malloc(strlen(de->d_name)+1)) == 0)
		{
			perror("malloc");
			closedir(dirp);
			free(dummy);
			free(a);
			return (-1);
		}
		strcpy(a, de->d_name);
		strcpy(b, de->d_name);
		c=strrchr(a, MDIRSEP[0]);
		if (c)	*c=0;

		if (dbobj_store(obj, a, strlen(a), b, strlen(b), "R"))
		{
			perror("dbobj_store");
			free(a);
			free(b);
			closedir(dirp);
			free(dummy);
			return (-1);
		}
		free(a);
		free(b);
	}
	if (dirp)	closedir(dirp);
	free(dummy);
	return (0);
}

static int update_link(const char *,
	const char *, const char *,
	const char *,
	const char *,
	size_t);

/*
**	Now, read our synced cur directory, and make sure that the soft
**	links are up to date.  Remove messages that have been deleted from
**	the sharable maildir, and make sure that the remaining links are
**	valid.
*/

static int update_cur(const char *cur, const char *shared, struct dbobj *obj)
{
DIR	*dirp;
struct	dirent *de;
char	*p;

	dirp=opendir(cur);
	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*cur_base;

	char	*cur_name_ptr;
	size_t	cur_name_len;

	char	*linked_name_buf;
	size_t	linked_name_len;
	int	n;

		if (de->d_name[0] == '.')	continue;

		/*
		** Strip the maildir flags, and look up the message in the
		** db.
		*/

		cur_base=malloc(strlen(de->d_name)+1);
		if (!cur_base)
		{
			perror("malloc");
			closedir(dirp);
			return (-1);
		}
		strcpy(cur_base, de->d_name);
		p=strrchr(cur_base, MDIRSEP[0]);
		if (p)	*p=0;

		cur_name_ptr=dbobj_fetch(obj, cur_base, strlen(cur_base),
			&cur_name_len, "");

		/* If it's there, delete the db entry. */

		if (cur_name_ptr)
			dbobj_delete(obj, cur_base, strlen(cur_base));

		/*
		** We'll either delete this soft link, or check its
		** contents, so we better build its complete pathname in
		** any case.
		*/

		free(cur_base);
		cur_base=malloc(strlen(de->d_name)+strlen(cur)+2);
		if (!cur_base)
		{
			perror("malloc");
			if (cur_name_ptr)	free(cur_name_ptr);
			closedir(dirp);
			return (-1);
		}
		strcat(strcat(strcpy(cur_base, cur), "/"), de->d_name);

		if (!cur_name_ptr)	/* Removed from sharable dir */
		{
			unlink(cur_base);
			free(cur_base);
			continue;
		}

		linked_name_len=strlen(shared)+strlen(de->d_name)+100;
			/* should be enough */

		if ((linked_name_buf=malloc(linked_name_len)) == 0)
		{
			perror("malloc");
			free(cur_base);
			free(cur_name_ptr);
			closedir(dirp);
			return (-1);
		}

		if ((n=readlink(cur_base, linked_name_buf, linked_name_len))< 0)
		{
			/* This is stupid, let's just unlink this nonsense */

			n=0;
		}

		if (n == 0 || n >= linked_name_len ||
			(linked_name_buf[n]=0,
			update_link(cur,
				cur_base, linked_name_buf, shared, cur_name_ptr,
				cur_name_len)))
		{
			unlink(cur_base);
			free(linked_name_buf);
			free(cur_base);
			free(cur_name_ptr);
			closedir(dirp);
			return (-1);
		}
		free(cur_base);
		free(linked_name_buf);
		free(cur_name_ptr);
	}
	if (dirp)	closedir(dirp);
	return (0);
}

/* Update the link pointer */

static int update_link(const char *curdir,
	const char *linkname, const char *linkvalue,
	const char *shareddir,
	const char *msgfilename,
	size_t msgfilenamelen)
{
	char	*p=malloc(strlen(shareddir)+sizeof("/cur/")+msgfilenamelen);
	char	*q;
	int	fd;
	struct maildir_tmpcreate_info createInfo;

	if (!p)
	{
		perror("malloc");
		return (-1);
	}

	strcat(strcpy(p, shareddir), "/cur/");
	q=p+strlen(p);
	memcpy(q, msgfilename, msgfilenamelen);
	q[msgfilenamelen]=0;

	if (linkvalue && strcmp(p, linkvalue) == 0)
	{
		/* the link is good */

		free(p);
		return (0);
	}

	/* Ok, we want this to be an atomic operation. */

	maildir_tmpcreate_init(&createInfo);
	createInfo.maildir=curdir;
	createInfo.uniq="relink";
	createInfo.doordie=1;

	if ((fd=maildir_tmpcreate_fd(&createInfo)) < 0)
		return -1;

	close(fd);
	unlink(createInfo.tmpname);

	if (symlink(p, createInfo.tmpname) < 0 ||
	    rename(createInfo.tmpname, linkname) < 0)
	{
		perror(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return (-1);
	}

	maildir_tmpcreate_free(&createInfo);
	return (0);
}

/* and now, anything that's left in the temporary db must be new messages */

static int newmsgs(const char *cur, const char *shared, struct dbobj *obj)
{
	char	*key, *val;
	size_t	keylen, vallen;
	int fd;
	struct maildir_tmpcreate_info createInfo;

	maildir_tmpcreate_init(&createInfo);
	createInfo.maildir=cur;
	createInfo.uniq="newlink";
	createInfo.doordie=1;

	if ((fd=maildir_tmpcreate_fd(&createInfo)) < 0)
		return -1;
	close(fd);

	unlink(createInfo.tmpname);

	for (key=dbobj_firstkey(obj, &keylen, &val, &vallen); key;
		key=dbobj_nextkey(obj, &keylen, &val, &vallen))
	{
	char	*slink=malloc(strlen(shared)+sizeof("/cur/")+vallen);
	char	*q;

		if (!slink)
		{
			free(val);
			maildir_tmpcreate_free(&createInfo);
			return (-1);
		}

		strcat(strcpy(slink, shared), "/cur/");
		q=slink+strlen(slink);
		memcpy(q, val, vallen);
		q[vallen]=0;
		free(val);

		if (symlink(slink, createInfo.tmpname))
		{
			perror(createInfo.tmpname);

			free(slink);
			maildir_tmpcreate_free(&createInfo);
			return (-1);
		}

		free(slink);
		slink=malloc(strlen(cur)+sizeof("/new/" MDIRSEP "2,")+keylen);
		if (!slink)
		{
			perror("malloc");
			maildir_tmpcreate_free(&createInfo);
			return (-1);
		}

		strcat(strcpy(slink, cur), "/new/");
		q=slink+strlen(slink);
		memcpy(q, key, keylen);
		strcpy(q+keylen, MDIRSEP "2,");

		if (rename(createInfo.tmpname, slink))
		{
			free(slink);
			maildir_tmpcreate_free(&createInfo);
			return (-1);
		}
		free(slink);
	}
	maildir_tmpcreate_free(&createInfo);
	return (0);
}

static void do_maildir_shared_sync(const char *dir, const char *shared)
{
struct	dbobj obj;
char	*dbname;
char	*cur;
char	*shared_update_name;

struct	stat	stat1, stat2;
int	fd;

	maildir_purgetmp(dir);	/* clean up after myself */
	maildir_getnew(dir, 0, NULL, NULL);

	maildir_purgetmp(shared);
	maildir_getnew(shared, 0, NULL, NULL);

	/* Figure out if we REALLY need to sync something */

	shared_update_name=malloc(strlen(dir)+sizeof("/shared-timestamp"));
	if (!shared_update_name)	return;
	strcat(strcpy(shared_update_name, dir), "/shared-timestamp");
	cur=malloc(strlen(shared)+sizeof("/new"));
	if (!cur)
	{
		free(shared_update_name);
		return;
	}

	if (stat(shared_update_name, &stat1) == 0)
	{
		if ( stat( strcat(strcpy(cur, shared), "/new"), &stat2) == 0 &&
			stat2.st_mtime < stat1.st_mtime &&
			stat( strcat(strcpy(cur, shared), "/cur"), &stat2)
			== 0 && stat2.st_mtime < stat1.st_mtime)
		{
			free(shared_update_name);
			free(cur);
			return;
		}
	}
	if ((fd=maildir_safeopen(shared_update_name, O_RDWR|O_CREAT, 0600))>= 0)
	{
		if (write(fd, "", 1) < 0)
			perror("write");
		close(fd);
	}

	free(cur);
	free(shared_update_name);

	if (create_db(&obj, dir, &dbname))	return;

	if (build_db(shared, &obj))
	{
		dbobj_close(&obj);
		unlink(dbname);
		free(dbname);
		return;
	}

	if ((cur=malloc(strlen(dir)+sizeof("/cur"))) == 0)
	{
		perror("malloc");
		dbobj_close(&obj);
		unlink(dbname);
		free(dbname);
		return;
	}
	strcat(strcpy(cur, dir), "/cur");
	if (update_cur(cur, shared, &obj) == 0)
	{
		strcat(strcpy(cur, dir), "/new");
		if (update_cur(cur, shared, &obj) == 0)
		{
			*strrchr(cur, '/')=0;	/* Chop off the /new */
			newmsgs(cur, shared, &obj);
		}
	}

	free(cur);
	dbobj_close(&obj);
	unlink(dbname);
	free(dbname);
}

int maildir_sharedisro(const char *maildir)
{
char	*p=malloc(strlen(maildir)+sizeof("/shared/cur"));

	if (!p)
	{
		perror("malloc");
		return (-1);
	}
	strcat(strcpy(p, maildir), "/shared/cur");

	if (access(p, W_OK) == 0)
	{
		free(p);
		return (0);
	}
	free(p);
	return (1);
}

int maildir_unlinksharedmsg(const char *filename)
{
char	*buf=maildir_getlink(filename);

	if (buf)
	{
		struct stat stat_buf;
		int rc=unlink(buf);

		/*
		** If we FAILED to unlink the real message in the real
		** sharable folder, but the message still exists, it means
		** that we do not have the permission to do so, so do not
		** purge this folder.  Instead, remove the T flag from
		** this message.
		*/

		if (rc && stat(buf, &stat_buf) == 0)
		{
			char *cpy=strdup(filename);

			if (cpy)
			{
				char *p=strrchr(cpy, MDIRSEP[0]);

				if (p && strchr(p, '/') == 0 &&
				    strncmp(p, MDIRSEP "2,", 3) == 0 &&
				    (p=strchr(p, 'T')) != 0)
				{
					while ((*p=p[1]) != 0)
						++p;
					rename(filename, cpy);
				}
				free(cpy);
			}
						
			free(buf);
			return (0);
		}
		free(buf);
	}
	unlink(filename);
	return (0);
}


#else

/* We cannot implement sharing */

void maildir_list_sharable(const char *maildir,
	void (*func)(const char *, void *),
	void *voidp)
{
}

int maildir_shared_subscribe(const char *maildir, const char *folder)
{
	errno=EINVAL;
	return (-1);
}

void maildir_list_shared(const char *maildir,
	void (*func)(const char *, void *),
	void *voidp)
{
}

int maildir_shared_unsubscribe(const char *maildir, const char *folder)
{
	errno=EINVAL;
	return (-1);
}

#if 0
char *maildir_shareddir(const char *maildir, const char *sharedname)
{
	errno=EINVAL;
	return (0);
}
#endif

void maildir_shared_sync(const char *maildir)
{
}

int maildir_sharedisro(const char *maildir)
{
	return (-1);
}

int maildir_unlinksharedmsg(const char *filename)
{
	return (-1);
}
#endif
