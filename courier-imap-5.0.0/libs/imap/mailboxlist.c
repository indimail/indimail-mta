/*
** Copyright 1998 - 2007 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
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
#if	HAVE_UTIME_H
#include	<utime.h>
#endif
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"imapscanclient.h"

#include	"mysignal.h"
#include	"imapd.h"
#include	"fetchinfo.h"
#include	"searchinfo.h"
#include	"storeinfo.h"
#include	"mailboxlist.h"

#include	"maildir/config.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildiraclt.h"
#include	"maildir/maildirnewshared.h"
#include	"maildir/maildirinfo.h"
#include    "unicode/courier-unicode.h"
#include	"authlib/auth.h"

static const char rcsid[]="$Id: mailboxlist.c,v 1.21 2009/06/27 16:32:38 mrsam Exp $";

static const char hierchs[]={HIERCH, 0};

extern char *decode_valid_mailbox(const char *, int);
extern dev_t homedir_dev;
extern ino_t homedir_ino;
/*
		LIST MAILBOXES
*/

static int do_mailbox_list(int do_lsub, char *qq, int isnullname,
			   int (*callback_func)(const char *hiersep,
						const char *mailbox,
						int flags,
						void *void_arg),
			   void *void_arg);

static int shared_index_err_reported=0;

const char *maildir_shared_index_file()
{
	static char *filenamep=NULL;

	if (filenamep == NULL)
	{
		const char *p=getenv("IMAP_SHAREDINDEXFILE");

		if (p && *p)
		{
			const char *q=authgetoptionenv("sharedgroup");

			if (!q) q="";

			filenamep=malloc(strlen(p)+strlen(q)+1);

			if (!filenamep)
				write_error_exit(0);

			strcat(strcpy(filenamep, p), q);
		}
	}

	if (filenamep && !shared_index_err_reported) /* Bitch just once */
	{
		struct stat stat_buf;

		shared_index_err_reported=1;
		if (stat(filenamep, &stat_buf))
		{
			fprintf(stderr, "ERR: ");
			perror(filenamep);
		}
	}

	return filenamep;
}

/*
**	IMAP sucks.  Here's why.
*/


int mailbox_scan(const char *reference, const char *name,
		 int list_options,
		 int (*callback_func)(const char *hiersep,
				      const char *mailbox,
				      int flags,
				      void *void_arg),
		 void *void_arg)
{
	char	*pattern, *p;
	int	nullname= *name == 0;
	int	rc;

	pattern=malloc(strlen(reference)+strlen(name)+2);

	strcpy(pattern, reference);

	p=strrchr(pattern, HIERCH);
	if (p && p[1] == 0)	*p=0; /* Strip trailing . for now */
	if (*pattern)
	{
		struct maildir_info mi;

		if (maildir_info_imap_find(&mi, pattern,
					   getenv("AUTHENTICATED")))
		{
			free(pattern);
			return (0); /* Invalid reference */
		}
		maildir_info_destroy(&mi);
	}

	/* Combine reference and name. */
	if (*pattern && *name)
		strcat(pattern, hierchs);
	strcat(pattern, name);

	if (name && *name)
	{
	char *s=strrchr(pattern, HIERCH);

		if (s && s[1] == 0)	*s=0;	/* strip trailing . */

	}

	/* Now, do the list */

	rc=do_mailbox_list(list_options, pattern, nullname,
			   callback_func, void_arg);
	free(pattern);
	return (rc);
}

static int match_mailbox(char *, char *, int flags);
static void match_mailbox_prep(char *);

/* Check if a folder has any new messages */

static int hasnewmsgs2(const char *dir)
{
DIR	*dirp=opendir(dir);
struct	dirent *de;

	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*p;

		if (de->d_name[0] == '.')	continue;
		p=strrchr(de->d_name, MDIRSEP[0]);
		if (p == 0 || strncmp(p, MDIRSEP "2,", 3) ||
			strchr(p, 'S') == 0)
		{
			closedir(dirp);
			return (1);
		}
	}
	if (dirp)	closedir(dirp);
	return (0);
}

static int hasnewmsgs(const char *folder)
{
char *dir=decode_valid_mailbox(folder, 0);
char *subdir;

	if (!dir)	return (0);

	if (is_sharedsubdir(dir))
		maildir_shared_sync(dir);

	subdir=malloc(strlen(dir)+sizeof("/cur"));
	if (!subdir)	write_error_exit(0);
	strcat(strcpy(subdir, dir), "/new");
	if (hasnewmsgs2(subdir))
	{
		free(subdir);
		free(dir);
		return (1);
	}

	strcat(strcpy(subdir, dir), "/cur");
	if (hasnewmsgs2(subdir))
	{
		free(subdir);
		free(dir);
		return (1);
	}

	free(subdir);
	free(dir);
	return (0);
}

/* Each folder is listed with the \Noinferiors tag.  Then, for every subfolder
** we've seen, we need to output a listing for all the higher-level hierarchies
** with a \Noselect tag.  Therefore, we need to keep track of all the
** hierarchies we've seen so far.
*/

struct hierlist {
	struct hierlist *next;
	int flag;
	char *hier;
	} ;

static int add_hier(struct hierlist **h, const char *s)
{
struct hierlist *p;

	for (p= *h; p; p=p->next)
		if (strcmp(p->hier, s) == 0)	return (1);
			/* Seen this one already */

	if ((p=(struct hierlist *)
		malloc( sizeof(struct hierlist)+1+strlen(s))) == 0)
			/* HACK!!!! */
		write_error_exit(0);
	p->flag=0;
	p->hier=(char *)(p+1);
	strcpy(p->hier, s);
	p->next= *h;
	*h=p;
	return (0);
}

static struct hierlist *search_hier(struct hierlist *h, const char *s)
{
struct hierlist *p;

	for (p= h; p; p=p->next)
		if (strcmp(p->hier, s) == 0)	return (p);
	return (0);
}

static void hier_entry(char *folder,
		       struct hierlist **hierarchies);

static int has_hier_entry(char *folder,
			  struct hierlist **hierarchies);

static void folder_entry(char *folder, char *pattern,
			 int list_options,
			 struct hierlist **folders,
			 struct hierlist **hierarchies)
{
	size_t i;
	size_t folder_l=strlen(folder);

	int need_add_hier;
	int need_add_folders;

	match_mailbox_prep(folder);

	/* Optimize away folders we don't care about */

	for (i=0; pattern[i]; i++)
	{
		if ((!(list_options & LIST_CHECK1FOLDER)) &&
		    (pattern[i] == '%' || pattern[i] == '*'))
		{
			while (i)
			{
				if (pattern[i] == HIERCH)
					break;
				--i;
			}
			break;
		}
	}

	if (folder_l <= i)
	{
		if (memcmp(folder, pattern, folder_l))
			return;

		if (folder_l != i && pattern[folder_l] != HIERCH)
			return;
	}
	else if (i)
	{
		if (memcmp(folder, pattern, i))
			return;
		if (folder[i] != HIERCH)
			return;
	}

	need_add_folders=0;

	if (match_mailbox(folder, pattern, list_options) == 0)
		need_add_folders=1;

	need_add_hier=0;
	if (!has_hier_entry(folder, hierarchies))
		need_add_hier=1;

	if (!need_add_folders && !need_add_hier)
		return; /* Nothing to do */

	{
		CHECK_RIGHTSM(folder, have_rights, ACL_LOOKUP);

		if (!have_rights[0])
			return;
	}

	if (need_add_folders)
		(void) add_hier(folders, folder);

	if (need_add_hier)
		hier_entry(folder, hierarchies);
}

static void hier_entry(char *folder,
		       struct hierlist **hierarchies)
{
	unsigned i;

	for (i=0; folder[i]; i++)
	{
		if (folder[i] != HIERCH)	continue;
		folder[i]=0;
		(void)add_hier(hierarchies, folder);
		folder[i]=HIERCH;
	}
}

static int has_hier_entry(char *folder,
			  struct hierlist **hierarchies)
{
	unsigned i;

	for (i=0; folder[i]; i++)
	{
		if (folder[i] != HIERCH)	continue;
		folder[i]=0;
		if (!search_hier(*hierarchies, folder))
		{
			folder[i]=HIERCH;
			return (0);
		}
		folder[i]=HIERCH;
	}
	return (1);
}

struct list_sharable_info {
	char *pattern;
	struct hierlist **folders, **hierarchies;
	int flags;
	int (*callback_func)(const char *hiersep,
			     const char *mailbox,
			     int flags,
			     void *void_arg);
	void *cb_arg;
	} ;

static void list_sharable(const char *n,
	void *voidp)
{
struct list_sharable_info *ip=(struct list_sharable_info *)voidp;
char	*p=malloc(strlen(n)+sizeof("shared."));

	if (!p)	write_error_exit(0);

	strcat(strcpy(p, "shared."), n);

	folder_entry(p, ip->pattern, ip->flags,
		     ip->folders, ip->hierarchies);

	free(p);
}

static void list_subscribed(char *hier,
			    int flags,
			    struct hierlist **folders,
			    struct hierlist **hierarchies)
{
char	buf[BUFSIZ];
FILE	*fp;

	fp=fopen(SUBSCRIBEFILE, "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) != 0)
		{
			char *q=strchr(buf, '\n');

			if (q)	*q=0;

			if (*hier == '#')
			{
				if (*buf != '#')
					continue;
			}
			else
			{
				if (*buf == '#')
					continue;
			}

			folder_entry(buf, hier, flags,
				     folders, hierarchies);
		}
		fclose(fp);
	}
}

static void maildir_scan(const char *inbox_dir,
			 const char *inbox_name,
			 struct list_sharable_info *shared_info)
{
	DIR	*dirp;
	struct	dirent *de;
	struct stat statbuf;

	/* Scan maildir, looking for .subdirectories */

	dirp=opendir(inbox_dir && inbox_dir ? inbox_dir:".");
	while (dirp && (de=readdir(dirp)) != 0)
	{
	char	*p;

		if (de->d_name[0] != '.' ||
		    strcmp(de->d_name, "..") == 0)
			continue;
		/*
		 * Skip regular files - mbhangui
		 */
		if (stat(de->d_name, &statbuf))
			continue;
		if (S_ISREG(statbuf.st_mode))
			continue;

		if ((p=malloc(strlen(de->d_name)+strlen(inbox_name)+10)) == 0)
					/* A bit too much, that's OK */
			write_error_exit(0);

		strcpy(p, inbox_name);

		if (strcmp(de->d_name, "."))
			strcat(p, de->d_name);

		folder_entry(p, shared_info->pattern, shared_info->flags,
			     shared_info->folders,
			     shared_info->hierarchies);
		free(p);
	}

	if (dirp)	closedir(dirp);
}

/* List the #shared hierarchy */

struct list_newshared_info {
	const char *acc_pfix;
	const char *skipped_pattern;
	struct list_sharable_info *shared_info;
	struct maildir_shindex_cache *parentCache;
	int dorecurse;
};

static int list_newshared_cb(struct maildir_newshared_enum_cb *cb);
static int list_newshared_skipcb(struct maildir_newshared_enum_cb *cb);
static int list_newshared_skiplevel(struct maildir_newshared_enum_cb *cb);

static int list_newshared_shortcut(const char *skipped_pattern,
				   struct list_sharable_info *shared_info,
				   const char *current_namespace,
				   struct maildir_shindex_cache *parentCache,
				   const char *indexfile,
				   const char *subhierarchy);

static int list_newshared(const char *skipped_pattern,
			  struct list_sharable_info *shared_info)
{
	return list_newshared_shortcut(skipped_pattern, shared_info,
				       NEWSHARED,
				       NULL, NULL, NULL);
}

static int list_newshared_shortcut(const char *skipped_pattern,
				   struct list_sharable_info *shared_info,
				   const char *acc_pfix,
				   struct maildir_shindex_cache *parentCache,
				   const char *indexfile,
				   const char *subhierarchy)
{
	struct list_newshared_info lni;
	int rc;
	struct maildir_shindex_cache *curcache=NULL;

	lni.acc_pfix=acc_pfix;
	lni.skipped_pattern=skipped_pattern;
	lni.shared_info=shared_info;
	lni.dorecurse=1;

	/* Try for some common optimization, to avoid expanding the
	** entire #shared hierarchy, taking advantage of the cache list.
	*/

	for (;;)
	{
		const char *p;
		size_t i;
		char *q;
		int eof;

		if (strcmp(skipped_pattern, "%") == 0)
		{
			lni.dorecurse=0;
			break;
		}

		if (strncmp(skipped_pattern, "%" HIERCHS,
			    sizeof("%" HIERCHS)-1) == 0)
		{
			curcache=maildir_shared_cache_read(parentCache,
							   indexfile,
							   subhierarchy);
			if (!curcache)
				return 0;

			lni.acc_pfix=acc_pfix;
			lni.skipped_pattern=skipped_pattern
				+ sizeof("%" HIERCHS)-1;
			lni.parentCache=curcache;

			for (i=0; i<curcache->nrecords; i++)
			{
				if (i == 0)
				{
					curcache->indexfile.startingpos=0;
					rc=maildir_newshared_nextAt(&curcache->indexfile,
								    &eof,
								    list_newshared_skiplevel,
								    &lni);
				}
				else
					rc=maildir_newshared_next(&curcache->indexfile,
								  &eof,
								  list_newshared_skiplevel,
								  &lni);

				if (rc || eof)
				{
					fprintf(stderr, "ERR:maildir_newshared_next failed: %s\n",
						strerror(errno));
					break;
				}
			}
			return 0;
		}

		for (p=skipped_pattern; *p; p++)
			if (*p == HIERCH ||
			    ((lni.shared_info->flags & LIST_CHECK1FOLDER) == 0
			     && (*p == '*' || *p == '%')))
				break;

		if (*p && *p != HIERCH)
			break;

		curcache=maildir_shared_cache_read(parentCache, indexfile,
						   subhierarchy);
		if (!curcache)
			return 0;

		for (i=0; i < curcache->nrecords; i++)
		{
			char *n=maildir_info_imapmunge(curcache->records[i]
						       .name);

			if (!n)
				write_error_exit(0);

			if (strlen(n) == p-skipped_pattern &&
			    strncmp(n, skipped_pattern, p-skipped_pattern) == 0)
			{
				free(n);
				break;
			}
			free(n);
		}

		if (i >= curcache->nrecords) /* not found */
			return 0;

		if (*p)
			++p;


		q=malloc(strlen(acc_pfix)+(p-skipped_pattern)+1);
		if (!q)
		{
			write_error_exit(0);
		}
		strcpy(q, acc_pfix);
		strncat(q, skipped_pattern, p-skipped_pattern);

		lni.acc_pfix=q;
		lni.skipped_pattern=p;
		lni.parentCache=curcache;

		curcache->indexfile.startingpos=curcache->records[i].offset;

		rc=maildir_newshared_nextAt(&curcache->indexfile, &eof,
					    list_newshared_skipcb, &lni);
		free(q);
		return rc;

	}

	if (!indexfile)
		indexfile=maildir_shared_index_file();

	rc=maildir_newshared_enum(indexfile, list_newshared_cb, &lni);

	return rc;
}

static int list_newshared_cb(struct maildir_newshared_enum_cb *cb)
{
	const char *name=cb->name;
	const char *homedir=cb->homedir;
	const char *maildir=cb->maildir;
	struct list_newshared_info *lni=
		(struct list_newshared_info *)cb->cb_arg;
	char *n=maildir_info_imapmunge(name);
	int rc;

	if (!n)
		write_error_exit(0);

	if (homedir == NULL)
	{
		struct list_newshared_info new_lni= *lni;
		char *new_pfix=malloc(strlen(lni->acc_pfix)+
				      strlen(n)+2);
		if (!new_pfix)
			write_error_exit(0);

		strcat(strcpy(new_pfix, lni->acc_pfix), n);

		free(n);
		n=new_pfix;
		new_lni.acc_pfix=n;
		add_hier(lni->shared_info->hierarchies, n);
		hier_entry(n, lni->shared_info->hierarchies);
		strcat(n, hierchs);
		rc=lni->dorecurse ?
			maildir_newshared_enum(maildir, list_newshared_cb,
					       &new_lni):0;
	}
	else
	{
		char *new_pfix;
		struct stat stat_buf;

		new_pfix=maildir_location(homedir, maildir);

		if (stat(new_pfix, &stat_buf) < 0 ||
		    /* maildir inaccessible, perhaps another server? */

		    (stat_buf.st_dev == homedir_dev &&
		     stat_buf.st_ino == homedir_ino))
		    /* Exclude ourselves from the shared list */
		{
			free(new_pfix);
			free(n);
			return 0;
		}
		free(new_pfix);

		new_pfix=malloc(strlen(lni->acc_pfix)+
				      strlen(n)+1);
		if (!new_pfix)
			write_error_exit(0);

		strcat(strcpy(new_pfix, lni->acc_pfix), n);

		free(n);
		n=new_pfix;

		new_pfix=malloc(strlen(homedir)+strlen(maildir)+2);

		if (!new_pfix)
			write_error_exit(0);

		if (*maildir == '/')
			strcpy(new_pfix, maildir);
		else
			strcat(strcat(strcpy(new_pfix, homedir), "/"),
			       maildir);

		/*		if (lni->dorecurse) */

		maildir_scan(new_pfix, n, lni->shared_info);
#if 0
		else
		{
			folder_entry(n, lni->shared_info->pattern,
				     lni->shared_info->flags,
				     lni->shared_info->folders,
				     lni->shared_info->hierarchies);
		}
#endif

		free(new_pfix);
		rc=0;
	}
	free(n);
	return rc;
}

static int list_newshared_skiplevel(struct maildir_newshared_enum_cb *cb)
{
	struct list_newshared_info *lni=
		(struct list_newshared_info *)cb->cb_arg;
	char *n=maildir_info_imapmunge(cb->name);

	char *p=malloc(strlen(lni->acc_pfix)+strlen(n)+sizeof(HIERCHS));
	int rc;
	const char *save_skip;

	if (!n || !p)
		write_error_exit(0);

	strcat(strcat(strcpy(p, lni->acc_pfix), n), HIERCHS);
	free(n);

	save_skip=lni->acc_pfix;
	lni->acc_pfix=p;

	rc=list_newshared_skipcb(cb);
	lni->acc_pfix=save_skip;
	free(p);
	return rc;
}

static int list_newshared_skipcb(struct maildir_newshared_enum_cb *cb)
{
	struct list_newshared_info *lni=
		(struct list_newshared_info *)cb->cb_arg;
	char *dir;
	char *inbox_name;

	if (cb->homedir == NULL)
		return list_newshared_shortcut(lni->skipped_pattern,
					       lni->shared_info,
					       lni->acc_pfix,
					       lni->parentCache,
					       cb->maildir,
					       cb->name);

	inbox_name=my_strdup(lni->acc_pfix);

	dir=strrchr(inbox_name, HIERCH);
	if (dir && dir[1] == 0)
		*dir=0; /* Strip trailing hier separator */

	dir=malloc(strlen(cb->homedir)+strlen(cb->maildir)+2);

	if (!dir)
	{
		free(inbox_name);
		write_error_exit(0);
	}

	if (*cb->maildir == '/')
		strcpy(dir, cb->maildir);
	else
		strcat(strcat(strcpy(dir, cb->homedir), "/"), cb->maildir);

	maildir_scan(dir, inbox_name, lni->shared_info);
	free(dir);
	free(inbox_name);
	return 0;
}

static int do_mailbox_list(int list_options, char *pattern, int isnullname,
			   int (*callback_func)(const char *hiersep,
						const char *mailbox,
						int flags,
						void *void_arg),
			   void *void_arg)
{
int	found_hier=MAILBOX_NOSELECT;
int	is_interesting;
int	i,j,bad_pattern;
struct	hierlist *hierarchies, *folders, *hp;
struct list_sharable_info shared_info;

const char *obsolete;
int check_all_folders=0;
char hiersepbuf[8];
int callback_rc=0;

	obsolete=getenv("IMAP_CHECK_ALL_FOLDERS");
	if (obsolete && atoi(obsolete))
		check_all_folders=1;

	obsolete=getenv("IMAP_OBSOLETE_CLIENT");

	if (obsolete && atoi(obsolete) == 0)
		obsolete=0;

	/* Allow up to ten wildcards */

	for (i=j=0; pattern[i]; i++)
		if (pattern[i] == '*' || pattern[i] == '%')	++j;
	bad_pattern= j > 10;

	if (list_options & LIST_CHECK1FOLDER)
		bad_pattern=0;

	if (bad_pattern)
	{
		errno=EINVAL;
		return -1;
	}

	hierarchies=0;
	folders=0;

	match_mailbox_prep(pattern);

	shared_info.pattern=pattern;
	shared_info.folders= &folders;
	shared_info.hierarchies= &hierarchies;
	shared_info.flags=list_options;
	shared_info.callback_func=callback_func;
	shared_info.cb_arg=void_arg;

	if (!(list_options & LIST_SUBSCRIBED))
	{
		if (strncmp(pattern, NEWSHARED,
			    sizeof(NEWSHARED)-1) == 0)
		{
			list_newshared(pattern +
				       sizeof(NEWSHARED)-1,
				       &shared_info);
		}
		else
		{
			maildir_scan(".", INBOX, &shared_info);

			/* List sharable maildirs */

			maildir_list_sharable( ".", &list_sharable,
					       &shared_info );
		}
	}
	else
	{
		list_subscribed(pattern, list_options, &folders, &hierarchies);

		/* List shared folders */

		maildir_list_shared( ".", &list_sharable,
				     &shared_info );
	}

	while ((hp=folders) != 0)
	{
		struct hierlist *d;
		int mb_flags;

		folders=hp->next;

		is_interesting= -1;

		if (strcmp(hp->hier, INBOX) == 0 || check_all_folders)
			is_interesting=hasnewmsgs(hp->hier);

		strcat(strcat(strcpy(hiersepbuf, "\""), hierchs), "\"");

		mb_flags=0;

		if (is_interesting == 0)
		{
			mb_flags|=MAILBOX_UNMARKED;
		}
		if (is_interesting > 0)
		{
			mb_flags|=MAILBOX_MARKED;
		}

		if ((d=search_hier(hierarchies, hp->hier)) == 0)
		{
			mb_flags |=
				obsolete ? MAILBOX_NOINFERIORS:MAILBOX_NOCHILDREN;
		}
		else
		{
			d->flag=1;
			if (!obsolete)
				mb_flags |= MAILBOX_CHILDREN;
		}

		if (isnullname)
			found_hier=mb_flags;
		else
			if (callback_rc == 0)
				callback_rc=(*callback_func)
					(hiersepbuf, hp->hier,
					 mb_flags | list_options, void_arg);
		free(hp);
	}

	while ((hp=hierarchies) != 0)
	{
		hierarchies=hp->next;

		match_mailbox_prep(hp->hier);

		if (match_mailbox(hp->hier, pattern, list_options) == 0
		    && hp->flag == 0)
		{
			int mb_flags=MAILBOX_NOSELECT;

			if (!obsolete)
				mb_flags |= MAILBOX_CHILDREN;

			if (isnullname)
				found_hier=mb_flags;
			else 
			{
				strcat(strcat(strcpy(hiersepbuf, "\""),
					      hierchs), "\"");

				if (callback_rc == 0)
					callback_rc=(*callback_func)
						(hiersepbuf,
						 hp->hier,
						 mb_flags | list_options,
						 void_arg);
			}
		}
		free(hp);
	}

	if (isnullname)
	{
		const char *namesp="";

		if (strncmp(pattern, NEWSHARED, sizeof(NEWSHARED)-1) == 0)
			namesp=NEWSHARED;

		strcat(strcat(strcpy(hiersepbuf, "\""), hierchs), "\"");

		if (callback_rc == 0)
			callback_rc=(*callback_func)
				(hiersepbuf, namesp, found_hier | list_options,
				 void_arg);
	}
	return callback_rc;
}

static int match_recursive(char *, char *, int);

static void match_mailbox_prep(char *name)
{
	size_t	i;

	/* First component, INBOX, is case insensitive */

	if (
#if	HAVE_STRNCASECMP
	    strncasecmp(name, INBOX, sizeof(INBOX)-1) == 0
#else
	    strnicmp(name, INBOX, sizeof(INBOX)-1) == 0
#endif
	    )
		for (i=0; name[i] && name[i] != HIERCH; i++)
			name[i]=toupper( (int)(unsigned char)name[i] );

	/* ... except that "shared" should be lowercase ... */

	if (memcmp(name, "SHARED", 6) == 0)
		memcpy(name, "shared", 6);
}

static int match_mailbox(char *name, char *pattern, int list_options)
{
        if (list_options & LIST_CHECK1FOLDER)
                return strcmp(name, pattern);

	return (match_recursive(name, pattern, HIERCH));
}

static int match_recursive(char *name, char *pattern, int hierch)
{
	for (;;)
	{
		if (*pattern == '*')
		{
			do
			{
				if (match_recursive(name, pattern+1,
					hierch) == 0)
					return (0);
			} while (*name++);
			return (-1);
		}
		if (*pattern == '%')
		{
			do
			{
				if (match_recursive(name, pattern+1, hierch)
					== 0) return (0);
				if (*name == hierch)	break;
			} while (*name++);
			return (-1);
		}
		if (*name == 0 && *pattern == 0)	break;
		if (*name == 0 || *pattern == 0)	return (-1);
		if (*name != *pattern)	return (-1);
		++name;
		++pattern;
	}
	return (0);
}
