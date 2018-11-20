/*
** Copyright 2003-2011 Double Precision, Inc.
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
#include	<signal.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
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
#if HAVE_LOCALE_H
#include	<locale.h>
#endif

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"mysignal.h"
#include	"imapd.h"
#include	"fetchinfo.h"
#include	"searchinfo.h"
#include	"storeinfo.h"
#include	"mailboxlist.h"
#include	"thread.h"
#include	"outbox.h"

#include	"imapwrite.h"
#include	"imaptoken.h"
#include	"imapscanclient.h"
#include	"searchinfo.h"
#include	"maildir/config.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildirrequota.h"
#include	"maildir/maildirgetquota.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirwatch.h"
#include	"maildir/maildiraclt.h"
#include	"maildir/maildirnewshared.h"
#include	"maildir/maildirinfo.h"
#include	"unicode/courier-unicode.h"

#include	"rfc2045/rfc2045.h"
#include	"rfc822/rfc822.h"

#define SMAP_BUFSIZ 8192

#define SHARED "shared"

#define LIST_FOLDER 1
#define LIST_DIRECTORY 2

#define FETCH_UID 1
#define FETCH_SIZE 2
#define FETCH_FLAGS 4
#define FETCH_KEYWORDS 8
#define FETCH_INTERNALDATE 16

extern dev_t homedir_dev;
extern ino_t homedir_ino;

int mdcreate(const char *mailbox);
int mddelete(const char *s);

extern int folder_rename(struct maildir_info *mi1,
			 struct maildir_info *mi2,
			 const char **errmsg);
extern int current_temp_fd;
extern const char *current_temp_fn;

extern int snapshot_init(const char *, const char *);
extern int keywords();

extern unsigned long header_count, body_count;

extern char *compute_myrights(maildir_aclt_list *l,
			      const char *l_owner);

extern int addRemoveKeywords(int (*callback_func)(void *, void *),
			     void *callback_func_arg,
			     struct storeinfo *storeinfo_s);
extern int doAddRemoveKeywords(unsigned long n, int uid, void *vp);


extern void snapshot_select(int);
extern void doflags(FILE *fp, struct fetchinfo *fi,
		    struct imapscaninfo *i, unsigned long msgnum,
		    struct rfc2045 *mimep);
extern void set_time(const char *tmpname, time_t timestamp);
extern int imapenhancedidle(void);
extern void imapidle(void);

extern void expunge();
extern void doNoop(int);
extern int do_store(unsigned long, int, void *);
extern int reflag_filename(struct imapscanmessageinfo *mi,
			   struct imapflags *flags, int fd);

extern void do_expunge(unsigned long expunge_start,
		       unsigned long expunge_end,
		       int force);

extern char *current_mailbox, *current_mailbox_acl;
static int current_mailbox_shared;

extern struct imapscaninfo current_maildir_info;
extern void get_message_flags(struct imapscanmessageinfo *,
			      char *, struct imapflags *);
extern void fetchflags(unsigned long);
extern int acl_lock(const char *homedir,
		    int (*func)(void *),
		    void *void_arg);
extern void aclminimum(const char *);

struct rfc2045 *fetch_alloc_rfc2045(unsigned long, FILE *);
FILE *open_cached_fp(unsigned long);
void fetch_free_cache();

extern FILE *maildir_mkfilename(const char *mailbox, struct imapflags *flags,
				unsigned long s, char **tmpname,
				char **newname);

/*
** Parse a word from the current SMAP command.
*/

static char *getword(char **ptr)
{
	char *p= *ptr, *q, *r;

	while (*p && isspace((int)(unsigned char)*p))
		p++;

	if (*p != '"')
	{
		for (q=p; *q; q++)
		{
			if (isspace((int)(unsigned char)*q))
			{
				*q++=0;
				break;
			}
		}

		*ptr=q;
		return p;
	}

	++p;
	r=q=p;

	while (*r)
	{
		if (*r == '"')
		{
			if (r[1] == '"')
			{
				r += 2;
				*q++='"';
				continue;
			}
			++r;
			break;
		}

		*q++ = *r++;
	}

	*q=0;
	*ptr=r;
	return p;
}

#define UC(c) if ( (c) >= 'a' && (c) <= 'z') (c) += 'A' - 'a'

static void up(char *p)
{
	while (*p)
	{
		UC(*p);
		p++;
	}
}

/*
** Write a WORD reply.
*/
static void smapword_s(const char *w);

void smapword(const char *w)
{
	writes("\"");
	smapword_s(w);
	writes("\"");
}

static void smapword_s(const char *w)
{
	while (w && *w)
	{
		size_t i;

		for (i=0; w[i]; i++)
			if (w[i] == '"')
				break;
		if (i)
			writemem(w, i);

		w += i;

		if (*w)
		{
			writes("\"\"");
			++w;
		}
	}
}

struct fn_word_list {
	struct fn_word_list *next;
	char *w;
};

/*
** Create a folder word array by reading words from the SMAP command.
*/

static char **fn_fromwords(char **ptr)
{
	struct fn_word_list *h=NULL, *n, **t=&h;
	size_t cnt=0;
	char *p;
	char **fn;

	while (*(p=getword(ptr)))
	{
		n=malloc(sizeof(struct fn_word_list));

		if (!n || !(n->w=strdup(p)))
		{
			if (n)
				free(n);

			while ((n=h) != NULL)
			{
				h=n->next;
				free(n->w);
				free(n);
			}
			return NULL;
		}

		n->next=NULL;
		*t=n;
		t= &n->next;
		cnt++;
	}

	if (!h)
	{
		errno=EINVAL;
		return NULL;
	}

	fn=malloc((cnt+1)*sizeof(char *));
	cnt=0;

	while ((n=h) != NULL)
	{
		h=n->next;

		if (fn)
			fn[cnt]=n->w;
		else
			free(n->w);
		free(n);
		cnt++;
	}
	if (fn)
		fn[cnt]=0;
	return fn;
}

/*
** LIST-related functions.
*/

struct list_hier {
	struct list_hier *next;
	char *hier;
	int flags;
};

struct list_callback_info {

	struct list_hier *hier; /* Hierarchy being listed */

	struct list_hier *found;
};

static void list(char *folder, const char *descr, int type)
{
	writes("* LIST ");

	smapword(folder);

	writes(" ");

	smapword(descr);

	if (type & LIST_FOLDER)
		writes(" FOLDER");
	if (type & LIST_DIRECTORY)
		writes(" DIRECTORY");
	writes("\n");
}

/*
** Callback from maildir_list.  f="INBOX.folder.name"
**
*/

struct list_callback_utf8 {

	void (*callback_func)(const char *, char **, void *);
	void *callback_arg;
	const char *homedir;
	const char *owner;
};

static void list_callback(const char *f, void *vp)
{
	struct list_callback_utf8 *utf8=(struct list_callback_utf8 *)vp;
	maildir_aclt_list l;

	char **fn=maildir_smapfn_fromutf8(f);

	if (!fn)
	{
		perror(f);
		return;
	}

	if (maildir_acl_read(&l, utf8->homedir, strchr(f, '.')) == 0)
	{
		char *myrights;
		char *owner=malloc(sizeof("user=")+strlen(utf8->owner));

		if (!owner)
			write_error_exit(0);

		strcat(strcpy(owner, "user="), utf8->owner);
		myrights=compute_myrights(&l, owner);
		free(owner);

		if (myrights && strchr(myrights, ACL_LOOKUP[0]) != NULL)
			(*utf8->callback_func)(f, fn, utf8->callback_arg);
		if (myrights)
			free(myrights);

		maildir_aclt_list_destroy(&l);
	}
	maildir_smapfn_free(fn);
}

/*
** list_callback callback that accumulates existing folders beneath a
** certain hierarchy.
*/

static void list_utf8_callback(const char *n, char **f, void *vp)
{
	struct list_callback_info *lci=(struct list_callback_info *)vp;
	struct list_hier *h=lci->hier;

	for (;;)
	{
		if (!*f)
			return;

		if (h)
		{
			if (strcmp(h->hier, *f))
				break;

			h=h->next;
			f++;
			continue;
		}

		for (h=lci->found; h; h=h->next)
		{
			if (strcmp(h->hier, *f) == 0)
				break;
		}

		if (!h)
		{
			if ((h=malloc(sizeof(struct list_hier))) == NULL ||
			    (h->hier=strdup(*f)) == NULL)
			{
				if (h)
					free(h);
				perror("malloc");
				break;
			}

			h->next=lci->found;
			lci->found=h;
			h->flags=0;
		}

		if (f[1])
			h->flags |= LIST_DIRECTORY;
		else
			h->flags |= LIST_FOLDER;
		break;
	}
}

/*
** SMAP1 list command goes here.  Dirty hack: build the hierarchy list on
** the stack.
*/

static void do_listcmd(struct list_hier **head,
		       struct list_hier **tail,
		       char **ptr);

static void listcmd(struct list_hier **head,
		    struct list_hier **tail,
		    char **ptr)
{
	char *p;

	if (*(p=getword(ptr)))
	{
		struct list_hier node;
		node.next=NULL;
		node.hier=p;

		*tail= &node;
		listcmd(head, &node.next, ptr);
		return;
	}
	do_listcmd(head, tail, ptr);
}

struct smap_find_info {
	char *homedir;
	char *maildir;
};

static int smap_find_cb(struct maildir_newshared_enum_cb *cb);
static int smap_list_cb(struct maildir_newshared_enum_cb *cb);
static int read_acls(maildir_aclt_list *aclt_list,
		     struct maildir_info *minfo);

static void do_listcmd(struct list_hier **head,
		       struct list_hier **tail,
		       char **ptr)
{
	struct list_hier *p;
	size_t cnt;
	char **vecs;
	int hierlist=0;

	if (!*head) /* No arguments to LIST */
	{
		list(INBOX, "New Mail", LIST_FOLDER);
		list(INBOX, "Folders", LIST_DIRECTORY);
		list(PUBLIC, "Public Folders", LIST_DIRECTORY);
	}
	else
	{
		struct list_callback_info lci;
		struct list_callback_utf8 list_utf8_info;

		list_utf8_info.callback_func= &list_utf8_callback;
		list_utf8_info.callback_arg= &lci;

		lci.hier= *head;
		lci.found=NULL;

		if (strcmp(lci.hier->hier, PUBLIC) == 0)
		{
			struct maildir_shindex_cache *curcache;
			struct list_hier *p=lci.hier->next;
			struct smap_find_info sfi;
			int eof;
			char *d;

			curcache=maildir_shared_cache_read(NULL, NULL, NULL);

			while (curcache && p)
			{
				size_t i;
				int rc;
				struct list_hier inbox;

				for (i=0; i<curcache->nrecords; i++)
					if (strcmp(curcache->records[i].name,
						   p->hier) == 0)
						break;
				if (i >= curcache->nrecords)
				{
					curcache=NULL;
					break;
				}

				sfi.homedir=NULL;
				sfi.maildir=NULL;
				curcache->indexfile.startingpos=
					curcache->records[i].offset;
				rc=maildir_newshared_nextAt(&curcache
							    ->indexfile,
							    &eof,
							    smap_find_cb,
							    &sfi);

				if (rc || eof)
				{
					fprintf(stderr, "ERR: Internal error -"
						" maildir_newshared_nextAt: %s\n",
						strerror(errno));
					curcache=NULL;
					break;
				}

				if (!sfi.homedir)
				{
					curcache=
						maildir_shared_cache_read(curcache,
									  sfi.maildir,
									  p->hier);
					p=p->next;
					free(sfi.maildir);
					continue;
				}

				inbox.next=p->next;
				inbox.hier=INBOX;

				d=maildir_location(sfi.homedir, sfi.maildir);
				free(sfi.homedir);
				free(sfi.maildir);

				lci.hier= &inbox;
				list_utf8_info.homedir=d;
				list_utf8_info.owner=p->hier;

				maildir_list(d, &list_callback,
					     &list_utf8_info);
				free(d);
				curcache=NULL;
				break;
			}

			if (curcache) /* List a shared hierarchy */
			{
				int rc;

				curcache->indexfile.startingpos=0;
				eof=0;

				do
				{
					rc=(curcache->indexfile.startingpos
						? maildir_newshared_next:
						maildir_newshared_nextAt)
						(&curcache->indexfile, &eof,
						 &smap_list_cb,
						 &list_utf8_info);

					if (rc)
						fprintf(stderr,
							"ERR: Internal error -"
							" maildir_newshared_next: %s\n",
							strerror(errno));
				} while (rc == 0 && !eof);

				hierlist=1;
			}
		}
		else
		{
			list_utf8_info.homedir=".";
			list_utf8_info.owner=getenv("AUTHENTICATED");
			maildir_list(".", &list_callback,
				     &list_utf8_info);
		}

		for (cnt=0, p= *head; p; p=p->next)
			++cnt;

		vecs=malloc(sizeof(char *)*(cnt+2));

		if (!vecs)
		{
			while (lci.found)
			{
				struct list_hier *h=lci.found;

				lci.found=h->next;

				free(h->hier);
				free(h);
			}
			write_error_exit(0);
		}


		for (cnt=0, p= *head; p; p=p->next)
		{
			vecs[cnt]=p->hier;
			++cnt;
		}

		while (lci.found)
		{
			struct list_hier *h=lci.found;
			struct maildir_info minfo;
			maildir_aclt_list aclt_list;

			lci.found=h->next;

			vecs[cnt]=h->hier;
			vecs[cnt+1]=0;

			if (maildir_info_smap_find(&minfo, vecs,
						   getenv("AUTHENTICATED")) == 0)
			{
				if (read_acls(&aclt_list, &minfo) == 0)
				{
					char *acl;

					acl=compute_myrights(&aclt_list,
							     minfo.owner);

					if (acl)
					{
						if (strchr(acl, ACL_LOOKUP[0])
						    == NULL)
						{
							h->flags=LIST_DIRECTORY;

							if (hierlist)
								list(h->hier,
								     h->hier,
								     h->flags);

						}
						else
							list(h->hier, h->hier,
							     h->flags);
						free(acl);
					}
					else
					{
						fprintf(stderr,
							"ERR: Cannot compute"
							" my access rights"
							" for %s: %s\n",
							h->hier,
							strerror(errno));
					}

					maildir_aclt_list_destroy(&aclt_list);
				}
				else
				{
					fprintf(stderr,
						"ERR: Cannot read ACLs"
						" for %s(%s): %s\n",
						minfo.homedir ? minfo.homedir
						: ".",
						minfo.maildir ? minfo.maildir
						: "unknown",
						strerror(errno));
				}
			}
			else
			{
				fprintf(stderr,
					"ERR: Internal error in list():"
					" cannot find folder %s: %s\n",
					h->hier,
					strerror(errno));
			}

			free(h->hier);
			free(h);
		}
		free(vecs);
	}
	writes("+OK LIST completed\n");
}

static int smap_find_cb(struct maildir_newshared_enum_cb *cb)
{
	struct smap_find_info *ifs=(struct smap_find_info *)cb->cb_arg;

	if (cb->homedir)
		ifs->homedir=my_strdup(cb->homedir);
	if (cb->maildir)
		ifs->maildir=my_strdup(cb->maildir);
	return 0;
}

static int smap_list_cb(struct maildir_newshared_enum_cb *cb)
{
	struct list_callback_utf8 *list_utf8_info=
		(struct list_callback_utf8 *)cb->cb_arg;
	struct list_callback_info *lci=
		(struct list_callback_info *)list_utf8_info->callback_arg;
	char *d;

	struct list_hier *h;
	struct stat stat_buf;

	if (cb->homedir == NULL)
	{
		if ((h=malloc(sizeof(struct list_hier))) == NULL ||
		    (h->hier
		     =strdup(cb->name)) == NULL)
		{
			if (h)
				free(h);
			perror("ERR: malloc");
			return 0;
		}

		h->next= lci->found;
		lci->found=h;
		h->flags = LIST_DIRECTORY;
		return 0;
	}

	d=maildir_location(cb->homedir, cb->maildir);

	if (!d)
	{
		perror("ERR: get_topmaildir");
		return 0;
	}

	if (stat(d, &stat_buf) < 0 ||
	    (stat_buf.st_dev == homedir_dev &&
	     stat_buf.st_ino == homedir_ino))
	{
		free(d);
		return 0;
	}

	list_utf8_info->homedir=d;
	list_utf8_info->owner=cb->name;
	lci->hier=NULL;
	h=lci->found;
	lci->found=NULL;
	maildir_list(d, &list_callback, list_utf8_info);
	free(d);

	if (!lci->found)
		lci->found=h;
	else
	{
		char *p;

		while (lci->found->next) /* SHOULDN'T HAPPEN!!! */
		{
			struct list_hier *p=lci->found->next;

			lci->found->next=p->next;
			free(p->hier);
			free(p);
			fprintf(stderr, "ERR: Unexpected folder list"
				" in smap_list_cb()\n");
		}
		lci->found->next=h;

		p=my_strdup(cb->name);
		free(lci->found->hier);
		lci->found->hier=p;
	}

	return (0);
}

/*
** Read the name of a new folder.  Returns the pathname to the folder, suitable
** for immediate creation.
*/

static char *getCreateFolder_int(char **ptr, char *need_perms)
{
	char **fn;
	char *n;
	struct maildir_info minfo;
	size_t i;
	char *save;
	maildir_aclt_list aclt_list;

	fn=fn_fromwords(ptr);
	if (!fn)
		return NULL;


	if (need_perms)
	{
		for (i=0; fn[i]; i++)
			;

		if (i == 0)
		{
			*need_perms=0;
			maildir_smapfn_free(fn);
			errno=EINVAL;
			return NULL;
		}

		save=fn[--i];
		fn[i]=NULL;
		if (maildir_info_smap_find(&minfo, fn,
					   getenv("AUTHENTICATED")) < 0)
		{
			fn[i]=save;
			maildir_smapfn_free(fn);
			return NULL;
		}

		fn[i]=save;

		if (read_acls(&aclt_list, &minfo))
		{
			maildir_smapfn_free(fn);
			maildir_info_destroy(&minfo);
			return NULL;
		}

		save=compute_myrights(&aclt_list, minfo.owner);
		maildir_aclt_list_destroy(&aclt_list);

		for (i=0; need_perms[i]; i++)
			if (save == NULL || strchr(save, need_perms[i])==NULL)
			{
				if (save)
					free(save);
				maildir_smapfn_free(fn);
				maildir_info_destroy(&minfo);
				*need_perms=0;
				errno=EPERM;
				return NULL;
			}

		if (save)
			free(save);

		maildir_info_destroy(&minfo);
	}


	if (maildir_info_smap_find(&minfo, fn, getenv("AUTHENTICATED")) < 0)
	{
		maildir_smapfn_free(fn);
		return NULL;
	}

	maildir_smapfn_free(fn);

	if (minfo.homedir == NULL || minfo.maildir == NULL)
	{
		maildir_info_destroy(&minfo);
		errno=ENOENT;
		return NULL;
	}

	n=maildir_name2dir(minfo.homedir, minfo.maildir);

	if (need_perms && strchr(need_perms, ACL_CREATE[0]))
	{
		/* Initialize the ACL structures */

		if (read_acls(&aclt_list, &minfo) == 0)
			maildir_aclt_list_destroy(&aclt_list);
	}

	maildir_info_destroy(&minfo);

	return n;
}

static char *getCreateFolder(char **ptr, char *perms)
{
	char *p=getCreateFolder_int(ptr, perms);

	if (p && strncmp(p, "./", 2) == 0)
	{
		char *q=p+2;

		while ((q[-2]=*q) != 0)
			q++;
	}
	return p;
}


static int read_acls(maildir_aclt_list *aclt_list,
		     struct maildir_info *minfo)
{
	char *q;
	int rc;

	if (minfo->homedir == NULL || minfo->maildir == NULL)
	{
		if (minfo->mailbox_type == MAILBOXTYPE_NEWSHARED)
		{
			/* Intermediate node in public hier */

			maildir_aclt_list_init(aclt_list);

			if (maildir_aclt_list_add(aclt_list,
						  "anyone",
						  ACL_LOOKUP,
						  NULL) < 0)
			{
				maildir_aclt_list_destroy(aclt_list);
				return -1;
			}
			return 0;
		}

		return -1;
	}

	q=maildir_name2dir(".", minfo->maildir);
	if (!q)
	{
		fprintf(stderr, "ERR: Internal error"
			" in read_acls(%s)\n", minfo->maildir);
		return -1;
	}

	rc=maildir_acl_read(aclt_list, minfo->homedir,
			    q[0] == '.' &&
			    q[1] == '/' ? q+2:q);
	free(q);

	if (current_mailbox)
	{
		q=maildir_name2dir(minfo->homedir, minfo->maildir);

		if (q)
		{
			if (strcmp(q, current_mailbox) == 0)
			{
				char *r=compute_myrights(aclt_list,
							 minfo->owner);

				if (r && strcmp(current_mailbox_acl, r))
				{
					free(current_mailbox_acl);
					current_mailbox_acl=r;
					r=NULL;
				}
				if (r) free(r);
			}
			free(q);
		}
	}
	return rc;
}

static char *getExistingFolder_int(char **ptr,
				   char *rightsWanted)
{
	char **fn;
	char *n;
	struct maildir_info minfo;

	fn=fn_fromwords(ptr);
	if (!fn)
		return NULL;

	if (maildir_info_smap_find(&minfo, fn, getenv("AUTHENTICATED")) < 0)
	{
		maildir_smapfn_free(fn);
		return NULL;
	}
	maildir_smapfn_free(fn);

	if (minfo.homedir == NULL || minfo.maildir == NULL)
	{
		maildir_info_destroy(&minfo);
		errno=ENOENT;
		return NULL;
	}

	n=maildir_name2dir(minfo.homedir, minfo.maildir);

	if (n && rightsWanted)
	{
		maildir_aclt_list aclt_list;
		char *q, *r, *s;

		if (read_acls(&aclt_list, &minfo) < 0)
		{
			free(n);
			maildir_info_destroy(&minfo);
			return NULL;

		}

		q=compute_myrights(&aclt_list, minfo.owner);

		maildir_aclt_list_destroy(&aclt_list);

		if (q == NULL)
		{
			free(n);
			maildir_info_destroy(&minfo);
			return NULL;
		}

		for (r=s=rightsWanted; *r; r++)
			if (strchr(q, *r))
				*s++ = *r;
		*s=0;
		free(q);
	}

	maildir_info_destroy(&minfo);
	return n;
}

static char *getAccessToFolder(char **ptr, char *rightsWanted)
{
	char *p=getExistingFolder_int(ptr, rightsWanted);

	if (p && strncmp(p, "./", 2) == 0)
	{
		char *q=p+2;

		while ((q[-2]=*q) != 0)
			q++;
	}

	return p;
}

static void smap1_noop(int real_noop)
{
	if (current_mailbox)
		doNoop(real_noop);
	writes("+OK Folder updated\n");
}

/* Parse a message set.  Return the next word following the message set */

struct smapmsgset {
	struct smapmsgset *next;
	unsigned nranges;
	unsigned long range[2][2];
};

static struct smapmsgset msgset;
static const char digit[]="0123456789";

static char *markmsgset(char **ptr, int *hasmsgset)
{
	unsigned long n;
	char *w;

	struct smapmsgset *msgsetp;

	while ((msgsetp=msgset.next) != NULL)
	{
		msgset.next=msgsetp->next;
		free(msgsetp);
	}

	msgsetp= &msgset;

	msgsetp->nranges=0;

	*hasmsgset=0;

	n=0;

	while (*(w=getword(ptr)))
	{
		unsigned long a=0, b=0;
		const char *d;

		if (!*w || (d=strchr(digit, *w)) == NULL)
			break;

		*hasmsgset=1;

		while ( *w && (d=strchr(digit, *w)) != NULL)
		{
			a=a * 10 + d-digit;
			w++;
		}

		b=a;

		if (*w == '-')
		{
			++w;
			b=0;
			while ( *w && (d=strchr(digit, *w)) != NULL)
			{
				b=b * 10 + d-digit;
				w++;
			}
		}

		if (a <= n || b < a)
		{
			errno=EINVAL;
			return NULL;
		}

		n=b;

		if (msgsetp->nranges >=
		    sizeof(msgsetp->range)/sizeof(msgsetp->range[0]))
		{
			if ((msgsetp->next=malloc(sizeof(struct smapmsgset)))
			    == NULL)
			{
				write_error_exit(0);
			}

			msgsetp=msgsetp->next;
			msgsetp->next=NULL;
			msgsetp->nranges=0;
		}

		msgsetp->range[msgsetp->nranges][0]=a;
		msgsetp->range[msgsetp->nranges][1]=b;
		++msgsetp->nranges;
	}

	return w;
}

static void parseflags(char *q, struct imapflags *flags)
{
	char *p;

	if ((q=strchr(q, '=')) == NULL)
		return;
	++q;

	while (*q)
	{
		p=q;

		while (*q)
		{
			if (*q == ',')
			{
				*q++=0;
				break;
			}
			q++;
		}

		if (strcmp(p, "SEEN") == 0)
			flags->seen=1;
		else if (strcmp(p, "REPLIED") == 0)
			flags->answered=1;
		else if (strcmp(p, "DRAFT") == 0)
			flags->drafts=1;
		else if (strcmp(p, "DELETED") == 0)
			flags->deleted=1;
		else if (strcmp(p, "MARKED") == 0)
			flags->flagged=1;

	}
}

extern int get_keyword(struct libmail_kwMessage **kwPtr, const char *kw);
extern int valid_keyword(const char *kw);

static void parsekeywords(char *q, struct libmail_kwMessage **msgp)
{
	char *p;

	if ((q=strchr(q, '=')) == NULL)
		return;
	++q;

	while (*q)
	{
		p=q;

		while (*q)
		{
			if (*q == ',')
			{
				*q++=0;
				break;
			}
			q++;
		}

		get_keyword(msgp, p);
	}
}

static int applymsgset( int (*callback_func)(unsigned long, void *),
			void *callback_arg)
{
	struct smapmsgset *msgsetp= &msgset;
	unsigned long n;
	int rc;

	while (msgsetp)
	{
		unsigned i;

		for (i=0; i<msgsetp->nranges; i++)
		{
			for (n=msgsetp->range[i][0];
			     n <= msgsetp->range[i][1]; n++)
			{
				if (current_mailbox == NULL ||
				    n > current_maildir_info.nmessages)
					break;
				rc=(*callback_func)(n-1, callback_arg);
				if (rc)
					return rc;
			}
		}

		msgsetp=msgsetp->next;
	}
	return 0;
}

static int do_attrfetch(unsigned long n, void *vp);

static int applyflags(unsigned long n, void *vp)
{
	struct storeinfo *si=(struct storeinfo *)vp;
	int attrs;
	struct libmail_kwMessage *newKw;

	if (n >= current_maildir_info.nmessages)
		return 0;

	attrs= si->keywords ? FETCH_KEYWORDS:FETCH_FLAGS;

	if (!si->plusminus)
	{
		if (si->keywords == NULL) /* STORE FLAGS= */
			si->keywords=current_maildir_info.msgs[n].keywordMsg;
		else /* STORE KEYWORDS= */
			get_message_flags(current_maildir_info.msgs+n, 0,
					  &si->flags);
	}

	/* do_store may clobber si->keywords.  Punt */

	newKw=si->keywords;
	if (do_store(n+1, 0, si))
	{
		si->keywords=newKw;
		return -1;
	}
	si->keywords=newKw;

	do_attrfetch(n, &attrs);
	return 0;
}

struct smapAddRemoveKeywordInfo {
	struct storeinfo *si;
	void *storeVoidArg;
};

static int addRemoveSmapKeywordsCallback(void *myVoidArg, void *storeVoidArg);

static int addRemoveSmapKeywords(struct storeinfo *si)
{
	struct smapAddRemoveKeywordInfo ar;

	ar.si=si;

	return addRemoveKeywords(addRemoveSmapKeywordsCallback, &ar, si);
}

static int doAddRemoveSmapKeywords(unsigned long n, void *voidArg);

static int addRemoveSmapKeywordsCallback(void *myVoidArg, void *storeVoidArg)
{
	struct smapAddRemoveKeywordInfo *info=
		(struct smapAddRemoveKeywordInfo *)myVoidArg;

	info->storeVoidArg=storeVoidArg;
	return applymsgset(doAddRemoveSmapKeywords, info);
}

static int doAddRemoveSmapKeywords(unsigned long n, void *voidArg)
{
	struct smapAddRemoveKeywordInfo *info=
		(struct smapAddRemoveKeywordInfo *)voidArg;

	return doAddRemoveKeywords(n+1, 0, info->storeVoidArg);
}

static int setdate(unsigned long n, void *vp)
{
	time_t datestamp=*(time_t *)vp;
	char	*filename=maildir_filename(current_mailbox, 0,
					   current_maildir_info.msgs[n]
					   .filename);

	if (filename)
	{
		set_time(filename, datestamp);
		free(filename);
	}
	return 0;
}

static int msg_expunge(unsigned long n, void *vp)
{
	do_expunge(n, n+1, 1);
	return 0;
}

struct smapfetchinfo {
	int peek;
	char *entity;
	char *hdrs;
	char *mimeid;
};

static int hashdr(const char *hdrList, const char *hdr)
{
	if (!hdrList || !*hdrList)
		return 1;

	while (*hdrList)
	{
		size_t n;
		int is_envelope=0;
		int is_mime=0;

		if (*hdrList == ',')
		{
			++hdrList;
			continue;
		}

		if (strncmp(hdrList, ":ENVELOPE", 9) == 0)
		{
			switch (hdrList[9]) {
			case 0:
			case ',':
				is_envelope=1;
				break;
			}
		}

		if (strncmp(hdrList, ":MIME", 5) == 0)
		{
			switch (hdrList[5]) {
			case 0:
			case ',':
				is_mime=1;
				break;
			}
		}


		if (is_envelope || is_mime)
		{
			char hbuf[30];

			hbuf[0]=0;
			strncat(hbuf, hdr, 29);
			up(hbuf);

			if (strcmp(hbuf, "DATE") == 0)
				return 1;
			if (strcmp(hbuf, "SUBJECT") == 0)
				return 1;
			if (strcmp(hbuf, "FROM") == 0)
				return 1;
			if (strcmp(hbuf, "SENDER") == 0)
				return 1;
			if (strcmp(hbuf, "REPLY-TO") == 0)
				return 1;
			if (strcmp(hbuf, "TO") == 0)
				return 1;
			if (strcmp(hbuf, "CC") == 0)
				return 1;
			if (strcmp(hbuf, "BCC") == 0)
				return 1;
			if (strcmp(hbuf, "IN-REPLY-TO") == 0)
				return 1;
			if (strcmp(hbuf, "MESSAGE-ID") == 0)
				return 1;
			if (strcmp(hbuf, "REFERENCES") == 0)
				return 1;

			if (is_mime)
			{
				if (strcmp(hbuf, "MIME-VERSION") == 0)
					return 1;

				if (strncmp(hbuf, "CONTENT-", 8) == 0)
					return 1;
			}
		}

		for (n=0; hdrList[n] && hdrList[n] != ',' && hdr[n]; n++)
		{
			char a=hdrList[n];
			char b=hdr[n];

			UC(b);
			if (a != b)
				break;
		}

		if ((hdrList[n] == 0 || hdrList[n] == ',') && hdr[n] == 0)
			return 1;

		hdrList += n;
		while (*hdrList && *hdrList != ',')
			++hdrList;
	}
	return 0;
}

static void writemimeid(struct rfc2045 *rfcp)
{
	if (rfcp->parent)
	{
		writemimeid(rfcp->parent);
		writes(".");
	}
	writen(rfcp->pindex);
}

static int dump_hdrs(int fd, unsigned long n,
		     struct rfc2045 *rfcp, const char *hdrs,
		     const char *type)
{
	struct rfc2045src *src;
	struct rfc2045headerinfo *h;
	char *header;
	char *value;
	int rc;
        off_t start_pos, end_pos, dummy, start_body;
	off_t nbodylines;
	int get_flags=RFC2045H_NOLC;

	rc=0;

	if (type && strcmp(type, "RAWHEADERS") == 0)
		get_flags |= RFC2045H_KEEPNL;

	if (!rfcp)
	{
		struct stat stat_buf;

		if (fstat(fd, &stat_buf))
			end_pos=8000; /* Heh */
		else
			end_pos=stat_buf.st_size;
		start_pos=0;
		start_body=0;
	}
	else rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body, &dummy,
			     &nbodylines);

	writes("{.");
	writen(start_body - start_pos);
	writes("} FETCH ");
	writen(n+1);
	if (type)
	{
		writes(" ");
		writes(type);
		writes("\n");
	}
	else	/* MIME */
	{
		writes(" LINES=");
		writen(nbodylines);
		writes(" SIZE=");
		writen(end_pos-start_body);
		writes(" \"MIME.ID=");

		if (rfcp->parent)
		{
			writemimeid(rfcp);
			writes("\" \"MIME.PARENT=");
			if (rfcp->parent->parent)
				writemimeid(rfcp->parent);
		}
		writes("\"\n");
	}

	src=rfc2045src_init_fd(fd);
	h=src ? rfc2045header_start(src, rfcp):NULL;

	while (h &&
	       (rc=rfc2045header_get(h, &header, &value, get_flags)) == 0
	       && header)
	{
		if (hashdr(hdrs, header))
		{
			if (*header == '.')
				writes(".");
			writes(header);
			writes(": ");
			writes(value);
			writes("\n");

			header_count += strlen(header)+strlen(value)+3;
		}
	}
	writes(".\n");

	if (h)
		rfc2045header_end(h);
	else
		rc= -1;
	if (src)
		rfc2045src_deinit(src);
	return rc;
}

static int dump_body(FILE *fp, unsigned long msgNum,
		     struct rfc2045 *rfcp, int dump_all)
{
	char buffer[SMAP_BUFSIZ];
        off_t start_pos, end_pos, dummy, start_body;
	int i;
	int first;

	if (!rfcp)
	{
		struct stat stat_buf;

		if (fstat(fileno(fp), &stat_buf) < 0)
			return -1;

		if (dump_all)
		{
			start_pos=start_body=0;
		}
		else
		{
			if (fseek(fp, 0L, SEEK_SET) < 0)
				return -1;

			if (!(rfcp=rfc2045_alloc()))
				return -1;

			do
			{
				i=fread(buffer, 1, sizeof(buffer), fp);

				if (i < 0)
				{
					rfc2045_free(rfcp);
					return -1;
				}

				if (i == 0)
					break;
				rfc2045_parse(rfcp, buffer, i);
			} while (rfcp->workinheader);

			rfc2045_mimepos(rfcp, &start_pos, &end_pos,
					&start_body, &dummy,
					&dummy);
			rfc2045_free(rfcp);

			start_pos=0;
		}
		end_pos=stat_buf.st_size;
	}
	else rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body, &dummy,
			     &dummy);

	if (dump_all)
		start_body=start_pos;

	if (fseek(fp, start_body, SEEK_SET) < 0)
		return -1;

	first=1;
	do
	{
		int n=sizeof(buffer);

		if (n > end_pos - start_body)
			n=end_pos - start_body;

		for (i=0; i<n; i++)
		{
			int ch=getc(fp);

			if (ch == EOF)
			{
				errno=EIO;
				return -1;
			}
			buffer[i]=ch;
		}

		if (first)
		{
			if (start_body == end_pos)
			{
				writes("{.0} FETCH ");
				writen(msgNum+1);
				writes(" CONTENTS\n.");
			}
			else
			{
				writes("{");
				writen(i);
				writes("/");
				writen(end_pos - start_body);
				writes("} FETCH ");
				writen(msgNum+1);
				writes(" CONTENTS\n");
			}
		}
		else
		{
			writen(i);
			writes("\n");
		}

		first=0;
		writemem(buffer, i);

		start_body += i;
		body_count += i;
	} while (start_body < end_pos);
	writes("\n");
	return 0;
}

struct decodeinfo {
	char buffer[SMAP_BUFSIZ];
	size_t bufptr;

	int first;
	unsigned long msgNum;
	off_t estSize;
};

static void do_dump_decoded_flush(struct decodeinfo *);

static struct rfc2045 *decodeCreateRfc(FILE *fp);
static int do_dump_decoded(const char *, size_t, void *);

static int dump_decoded(FILE *fp, unsigned long msgNum,
			struct rfc2045 *rfcp)
{
	struct decodeinfo myDecodeInfo;
	const char *content_type;
	const char *content_transfer_encoding;
	const char *charset;
        off_t start_pos, end_pos, dummy, start_body;

	struct rfc2045src *src;
	struct rfc2045 *myrfcp=NULL;
	int fd;
	int i;

	if (!rfcp)
	{
		rfcp=myrfcp=decodeCreateRfc(fp);
		if (!rfcp)
			return -1;
	}

	if ((fd=dup(fileno(fp))) < 0)
	{
		if (myrfcp)
			rfc2045_free(myrfcp);
		return -1;
	}

	myDecodeInfo.first=1;
	myDecodeInfo.msgNum=msgNum;
	myDecodeInfo.bufptr=0;

	rfc2045_mimeinfo(rfcp, &content_type, &content_transfer_encoding,
			 &charset);
	rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body, &dummy,
			&dummy);
	myDecodeInfo.estSize=end_pos - start_body;

	if (content_transfer_encoding
	    && strlen(content_transfer_encoding) == 6)
	{
		char buf[7];

		strcpy(buf, content_transfer_encoding);
		up(buf);

		if (strcmp(buf, "BASE64") == 0)
			myDecodeInfo.estSize = myDecodeInfo.estSize / 4 * 3;

		/* Better estimate of base64 content */
	}

	src=rfc2045src_init_fd(fd);

	i=src ? rfc2045_decodemimesection(src, rfcp, &do_dump_decoded,
					  &myDecodeInfo):-1;

	do_dump_decoded_flush(&myDecodeInfo);

	if (src)
		rfc2045src_deinit(src);

	close(fd);

	if (i == 0 && myDecodeInfo.first) /* Empty body, punt */
	{
		writes("{.0} FETCH ");
		writen(msgNum+1);
		writes(" CONTENTS\n.");
	}
	writes("\n");
	if (myrfcp)
		rfc2045_free(myrfcp);
	return i;
}

/* Dummy up a rfc2045 structure for retrieving the entire msg body */

static struct rfc2045 *decodeCreateRfc(FILE *fp)
{
	char buffer[SMAP_BUFSIZ];
	struct stat stat_buf;
	int i;
	struct rfc2045 *myrfcp;

	if (fstat(fileno(fp), &stat_buf) < 0)
		return NULL;

	if (fseek(fp, 0L, SEEK_SET) < 0)
		return NULL;

	if (!(myrfcp=rfc2045_alloc()))
		return NULL;

	do
	{
		i=fread(buffer, 1, sizeof(buffer), fp);

		if (i < 0)
		{
			rfc2045_free(myrfcp);
			return NULL;
			}

		if (i == 0)
			break;
		rfc2045_parse(myrfcp, buffer, i);
	} while (myrfcp->workinheader);

	myrfcp->endpos=stat_buf.st_size;
	return myrfcp;
}

static int do_dump_decoded(const char *chunk, size_t chunkSize,
			   void *vp)
{
	struct decodeinfo *myDecodeInfo=(struct decodeinfo *) vp;

	while (chunkSize)
	{
		size_t n;

		if (myDecodeInfo->bufptr >= sizeof(myDecodeInfo->buffer))
			do_dump_decoded_flush(myDecodeInfo);

		n=sizeof(myDecodeInfo->buffer)-myDecodeInfo->bufptr;

		if (n > chunkSize)
			n=chunkSize;
		memcpy(myDecodeInfo->buffer + myDecodeInfo->bufptr, chunk, n);
		myDecodeInfo->bufptr += n;
		chunk += n;
		chunkSize -= n;
	}
	return 0;
}

static void do_dump_decoded_flush(struct decodeinfo *myDecodeInfo)
{
	size_t chunkSize= myDecodeInfo->bufptr;

	myDecodeInfo->bufptr=0;

	if (chunkSize == 0)
		return;

	if (myDecodeInfo->first)
	{
		myDecodeInfo->first=0;
		writes("{");
		writen(chunkSize);
		writes("/");
		writen(myDecodeInfo->estSize);
		writes("} FETCH ");
		writen(myDecodeInfo->msgNum+1);
		writes(" CONTENTS\n");
	}
	else
	{
		writen(chunkSize);
		writes("\n");
	}
	writemem(myDecodeInfo->buffer, chunkSize);
	body_count += chunkSize;
}

static int mime(int fd, unsigned long n,
		struct rfc2045 *rfcp, const char *hdrs)
{
	int rc=dump_hdrs(fd, n, rfcp, hdrs, NULL);

	if (rc)
		return rc;

	for (rfcp=rfcp->firstpart; rfcp; rfcp=rfcp->next)
		if (!rfcp->isdummy)
		{
			rc=mime(fd, n, rfcp, hdrs);
			if (rc)
				return rc;
		}

	return 0;
}

/*
** Find the specified MIME id.
*/

static struct rfc2045 *findmimeid(struct rfc2045 *rfcp,
				  const char *mimeid)
{
	unsigned long n;

	while (mimeid && *mimeid)
	{
		const char *d;

		n=0;

		if (strchr(digit, *mimeid) == NULL)
			return NULL;

		while (*mimeid && (d=strchr(digit, *mimeid)) != NULL)
		{
			n=n * 10 + d-digit;
			mimeid++;
		}

		while (rfcp)
		{
			if (!rfcp->isdummy && rfcp->pindex == n)
				break;
			rfcp=rfcp->next;
		}

		if (!rfcp)
			return NULL;

		if (*mimeid == '.')
		{
			++mimeid;
			rfcp=rfcp->firstpart;
		}
	}
	return rfcp;
}

static int do_fetch(unsigned long n, void *vp)
{
	struct smapfetchinfo *fi=(struct smapfetchinfo *)vp;
	FILE *fp=open_cached_fp(n);
	int rc=0;

	if (!fp)
		return -1;

	if (strcmp(fi->entity, "MIME") == 0)
	{
		struct rfc2045 *rfcp=fetch_alloc_rfc2045(n, fp);
		int fd;

		if (!rfcp)
			return -1;

		fd=dup(fileno(fp));
		if (fd < 0)
			return -1;

		rc=mime(fd, n, rfcp, fi->hdrs);
		close(fd);
	}
	else if (strcmp(fi->entity, "HEADERS") == 0 ||
		 strcmp(fi->entity, "RAWHEADERS") == 0)
	{
		int fd;
		struct rfc2045 *rfcp;

		fd=dup(fileno(fp));
		if (fd < 0)
			return -1;

		if (!fi->mimeid || !*fi->mimeid)
			rfcp=NULL;
		else
		{
			rfcp=fetch_alloc_rfc2045(n, fp);

			rfcp=findmimeid(rfcp, fi->mimeid);

			if (!rfcp)
			{
				close(fd);
				errno=EINVAL;
				return -1;
			}
		}

		rc=dump_hdrs(fd, n, rfcp, fi->hdrs, fi->entity);
		close(fd);
	}
	else if (strcmp(fi->entity, "BODY") == 0
		 || strcmp(fi->entity, "ALL") == 0)
	{
		struct rfc2045 *rfcp;

		if (!fi->mimeid || !*fi->mimeid)
			rfcp=NULL;
		else
		{
			rfcp=fetch_alloc_rfc2045(n, fp);

			rfcp=findmimeid(rfcp, fi->mimeid);

			if (!rfcp)
			{
				errno=EINVAL;
				return -1;
			}
		}

		rc=dump_body(fp, n, rfcp, fi->entity[0] == 'A');
	}
	else if (strcmp(fi->entity, "BODY.DECODED") == 0)
	{
		struct rfc2045 *rfcp;

		if (!fi->mimeid || !*fi->mimeid)
			rfcp=NULL;
		else
		{
			rfcp=fetch_alloc_rfc2045(n, fp);

			rfcp=findmimeid(rfcp, fi->mimeid);

			if (!rfcp)
			{
				errno=EINVAL;
				return -1;
			}
		}

		rc=dump_decoded(fp, n, rfcp);
	}
	else
	{
		rc=0;
	}

	if (rc == 0 && !fi->peek)
	{
		struct	imapflags	flags;

		get_message_flags(current_maildir_info.msgs+n,
				  0, &flags);
		if (!flags.seen)
		{
			flags.seen=1;
			reflag_filename(&current_maildir_info.msgs[n],
					&flags, fileno(fp));
			current_maildir_info.msgs[n].changedflags=1;
		}
	}

	if (current_maildir_info.msgs[n].changedflags)
		fetchflags(n);

	return rc;
}

void smap_fetchflags(unsigned long n)
{
	int items=FETCH_FLAGS | FETCH_KEYWORDS;

	do_attrfetch(n, &items);
}

static int do_attrfetch(unsigned long n, void *vp)
{
	int items=*(int *)vp;

	if (n >= current_maildir_info.nmessages)
		return 0;

	writes("* FETCH ");
	writen(n+1);

	if (items & FETCH_FLAGS)
	{
		char	buf[256];

		get_message_flags(current_maildir_info.msgs+n, buf, 0);

		writes(" FLAGS=");
		writes(buf);

		current_maildir_info.msgs[n].changedflags=0;
	}

	if ((items & FETCH_KEYWORDS) && keywords())
	{
		struct libmail_kwMessageEntry *kme;

		writes(" \"KEYWORDS=");

		if (current_maildir_info.msgs[n].keywordMsg &&
		    current_maildir_info.msgs[n].keywordMsg->firstEntry)
		{
			const char *p="";

			for (kme=current_maildir_info.msgs[n]
				     .keywordMsg->firstEntry;
			     kme; kme=kme->next)
			{
				writes(p);
				p=",";
				writes(keywordName(kme->libmail_keywordEntryPtr));
			}
		}
		writes("\"");
	}

	if (items & FETCH_UID)
	{
		char *p, *q;

		writes(" \"UID=");

		p=current_maildir_info.msgs[n].filename;

		q=strrchr(p, MDIRSEP[0]);
		if (q)
			*q=0;
		smapword_s(p);
		if (q)
			*q=MDIRSEP[0];
		writes("\"");
	}

	if (items & FETCH_SIZE)
	{
		char *p=current_maildir_info.msgs[n].filename;
		unsigned long cnt;

		if (maildir_parsequota(p, &cnt))
		{
			FILE *fp=open_cached_fp(n);
			struct stat stat_buf;

			if (fp && fstat(fileno(fp), &stat_buf) == 0)
				cnt=stat_buf.st_size;
			else
				cnt=0;
		}

		writes(" SIZE=");
		writen(cnt);
	}

	if (items & FETCH_INTERNALDATE)
	{
		struct stat stat_buf;
		FILE *fp=open_cached_fp(n);

		if (fp && fstat(fileno(fp), &stat_buf) == 0)
		{
			char buf[256];

			rfc822_mkdate_buf(stat_buf.st_mtime, buf);
			writes(" \"INTERNALDATE=");
			smapword_s(buf);
			writes("\"");
		}
	}
	writes("\n");
	return 0;
}

struct add_rcptlist {
	struct add_rcptlist *next;
	char *rcptto;
};

static unsigned long add_msg(FILE *fp, const char *format,
			     char *buffer,
			     size_t bufsize)
{
	unsigned long n=0;

	writes("> Go ahead\n");
	writeflush();

	if (*format == '.')
	{
		int last_eol=1;
		int dot_stuffed=0;
		int counter=-1;

		for (;;)
		{
			char c;

			if ( ((counter=counter + 1 ) % 8192) == 0)
				read_timeout(60);

			c=READ();

			if (c == '\r')
				continue;

			if (dot_stuffed && c == '\n')
				break;
			dot_stuffed=0;

			if (c == '.')
			{
				if (last_eol)
				{
					dot_stuffed=1;
					continue;
				}
			}
			last_eol= c == '\n';
			putc( (int)(unsigned char)c, fp);
			n++;
		}

		if (!last_eol)
		{
			putc('\n', fp);
			n++;
		}
	}
	else
	{
		unsigned long chunkSize;
		char last_char='\n';

		while (sscanf(format, "%lu", &chunkSize) == 1)
		{
			while (chunkSize)
			{
				size_t nn=bufsize;
				size_t i;

				if (nn > chunkSize)
					nn=(size_t)chunkSize;

				read_timeout(60);
				nn=doread(buffer, nn);

				chunkSize -= nn;
				n += nn;

				for (i=0; i<nn; i++)
				{
					last_char=buffer[i];

					if (last_char == '\r')
						continue;
					putc((int)(unsigned char)last_char,
					     fp);
				}
			}

			read_timeout(60);
			smap_readline(buffer, bufsize);
			format=buffer;
		}

		if (last_char != '\n')
		{
			putc('\n', fp);
			n++;
		}
	}

	if (n == 0)
	{
		++n;
		putc('\n', fp);
	}

	if (fflush(fp) < 0 || ferror(fp))
		return 0;
	return n;
}

static void adduid(char *n)
{
	char *q;

	q=strrchr(n, '/');
	if (q)
		n=q+1;

	q=strrchr(n, MDIRSEP[0]);
	if (q)
		*q=0;
	writes("* ADD \"UID=");
	smapword_s(n);
	writes("\"\n");
	if (q)
		*q=MDIRSEP[0];
}

static void senderr(char *errmsg)
{
	writes("-ERR ");
	writes(errmsg);
	writes("\n");
}

static int calc_quota(unsigned long n, void *voidptr)
{
	return do_copy_quota_calc(n+1, 0, voidptr);
}

/* Copy msg to another folder */

static void copieduid(unsigned long n, char *newname)
{
	char *p, *q;

	writes("* COPY ");
	writen(n);
	writes(" \"NEWUID=");

	p=strrchr(newname, '/')+1;

	if ((q=strrchr(p, MDIRSEP[0])) != NULL)
		*q=0;

	smapword_s(p);
	writes("\"\n");
}

static int do_copyKeywords(struct libmail_kwMessage *msg,
			   const char *destmailbox,
			   const char *newname)
{
	char *tmpkname, *newkname;

	if (!msg || !msg->firstEntry)
		return 0;

	if (maildir_kwSave(destmailbox, newname,
			   msg, &tmpkname, &newkname, 0))
	{
		perror("maildir_kwSave");
		return -1;
	}

	rename(tmpkname, newkname);
	free(tmpkname);
	free(newkname);
	return 0;
}

static void fixnewfilename(char *p)
{
	char *q;

	/* Nice hack: */

	q=strrchr(strrchr(p, '/'), MDIRSEP[0]);

	if (strcmp(q, MDIRSEP "2,") == 0)
	{
		*q=0;
		memcpy(strrchr(p, '/')-3, "new", 3);
	}
}

static int do_copymsg(unsigned long n, void *voidptr)
{
	char buf[SMAP_BUFSIZ];
	struct copyquotainfo *cqinfo=(struct copyquotainfo *)voidptr;
	struct imapflags new_flags;
	int fd;
	struct stat stat_buf;
	FILE *fp;
	char *tmpname, *newname;

	fd=imapscan_openfile(current_mailbox, &current_maildir_info, n);
	if (fd < 0)	return (0);

	if (fstat(fd, &stat_buf) < 0)
	{
		close(fd);
		return (0);
	}

	get_message_flags(current_maildir_info.msgs+n, 0, &new_flags);

	fp=maildir_mkfilename(cqinfo->destmailbox,
			      &new_flags, stat_buf.st_size,
			      &tmpname, &newname);

	fixnewfilename(newname);

	if (!fp)
	{
		close(fd);
		return (-1);
	}

	while (stat_buf.st_size)
	{
	int	n=sizeof(buf);

		if (n > stat_buf.st_size)
			n=stat_buf.st_size;

		n=read(fd, buf, n);

		if (n <= 0 || fwrite(buf, 1, n, fp) != n)
		{
			close(fd);
			fclose(fp);
			unlink(tmpname);
			free(tmpname);
			free(newname);
			fprintf(stderr,
			"ERR: error copying a message, user=%s, errno=%d\n",
				getenv("AUTHENTICATED"), errno);

			return (-1);
		}
		stat_buf.st_size -= n;
	}
	close(fd);

	if (fflush(fp) || ferror(fp))
	{
		fclose(fp);
		unlink(tmpname);
		free(tmpname);
		free(newname);
		fprintf(stderr,
			"ERR: error copying a message, user=%s, errno=%d\n",
			getenv("AUTHENTICATED"), errno);
		return (-1);
	}
	fclose(fp);

	if (do_copyKeywords(current_maildir_info.msgs[n].keywordMsg,
			    cqinfo->destmailbox,
			    strrchr(newname, '/')+1))
	{
		unlink(tmpname);
		free(tmpname);
		free(newname);
		fprintf(stderr,
			"ERR: error copying keywords, "
			"user=%s, errno=%d\n",
			getenv("AUTHENTICATED"), errno);
		return (-1);
	}

	current_maildir_info.msgs[n].copiedflag=1;

	maildir_movetmpnew(tmpname, newname);
	set_time(newname, stat_buf.st_mtime);
	free(tmpname);

	copieduid(n+1, newname);
	free(newname);
	return 0;
}

static int do_movemsg(unsigned long n, void *voidptr)
{
	char *filename;
	struct copyquotainfo *cqinfo=(struct copyquotainfo *)voidptr;
	char *newfilename;

	if (n >= current_maildir_info.nmessages)
		return 0;

	filename=maildir_filename(current_mailbox, 0,
				  current_maildir_info.msgs[n].filename);

	if (!filename)
		return 0;

	newfilename=malloc(strlen(cqinfo->destmailbox) + sizeof("/cur")
			   + strlen(strrchr(filename, '/')));

	if (!newfilename)
	{
		free(filename);
		write_error_exit(0);
	}

	strcat(strcat(strcpy(newfilename, cqinfo->destmailbox),
		      "/cur"), strrchr(filename, '/'));

	if (do_copyKeywords(current_maildir_info.msgs[n].keywordMsg,
			    cqinfo->destmailbox,
			    strrchr(newfilename, '/')+1))
	{
		fprintf(stderr,
			"ERR: error copying keywords, "
			"user=%s, errno=%d\n",
			getenv("AUTHENTICATED"), errno);

		free(filename);
		free(newfilename);
		return -1;
	}


	if (maildir_movetmpnew(filename, newfilename) == 0)
	{
		copieduid(n+1, newfilename);
		free(filename);
		free(newfilename);
		return 0;
	}

	if (do_copymsg(n, voidptr))
		return -1;

	unlink(filename);
	free(filename);
	free(newfilename);
	return 0;
}

static struct searchinfo *createSearch2(char *w,
					struct searchinfo **head, char **ptr);

static struct searchinfo *createSearch(struct searchinfo **head, char **ptr)
{
	char *w=getword(ptr);
	struct searchinfo *siAnd, *n;

	up(w);

	if (strcmp(w, "MARKED") == 0)
	{
		w=getword(ptr);
		up(w);

		n=createSearch2(w, head, ptr);

		if (!n)
			return NULL;

		siAnd=alloc_search(head);
		siAnd->type=search_and;

		siAnd->b=n;

		siAnd->a=n=alloc_search(head);

		n->type=search_msgflag;
		if (!(n->as=strdup("\\FLAGGED")))
			write_error_exit(0);

		return siAnd;
	}

	if (strcmp(w, "UNMARKED") == 0)
	{
		w=getword(ptr);
		up(w);

		n=createSearch2(w, head, ptr);

		if (!n)
			return NULL;

		siAnd=alloc_search(head);
		siAnd->type=search_and;

		siAnd->b=n;

		siAnd->a=n=alloc_search(head);

		n->type=search_not;

		n=n->a=alloc_search(head);

		n->type=search_msgflag;
		if (!(n->as=strdup("\\FLAGGED")))
			write_error_exit(0);

		return siAnd;
	}

	if (strcmp(w, "ALL") == 0)
	{
		w=getword(ptr);
		up(w);
		return createSearch2(w, head, ptr);
	}

	{
		char *ww=getword(ptr);
		up(ww);
		n=createSearch2(ww, head, ptr);

		if (!n)
			return NULL;

		siAnd=alloc_search(head);
		siAnd->type=search_and;

		siAnd->b=n;

		siAnd->a=n=alloc_search(head);

		n->type=search_messageset;
		if (!(n->as=strdup(w)))
			write_error_exit(0);

		for (ww=n->as; *ww; ww++)
			if (*ww == '-')
				*ww=':';

		if (!ismsgset_str(n->as))
		{
			errno=EINVAL;
			return NULL;
		}
	}
	return siAnd;
}

static struct searchinfo *createSearch2(char *w,
					struct searchinfo **head, char **ptr)
{
	int notflag=0;
	struct searchinfo *n;

	if (strcmp(w, "NOT") == 0)
	{
		notflag=1;
		w=getword(ptr);
		up(w);
	}

	if (strcmp(w, "REPLIED") == 0)
	{
		n=alloc_search(head);
		n->type=search_msgflag;
		if (!(n->as=strdup("\\ANSWERED")))
			write_error_exit(0);
	}
	else if (strcmp(w, "DELETED") == 0)
	{
		n=alloc_search(head);
		n->type=search_msgflag;
		if (!(n->as=strdup("\\DELETED")))
			write_error_exit(0);
	}
	else if (strcmp(w, "DRAFT") == 0)
	{
		n=alloc_search(head);
		n->type=search_msgflag;
		if (!(n->as=strdup("\\DRAFT")))
			write_error_exit(0);
	}
	else if (strcmp(w, "SEEN") == 0)
	{
		n=alloc_search(head);
		n->type=search_msgflag;
		if (!(n->as=strdup("\\SEEN")))
			write_error_exit(0);
	}
	else if (strcmp(w, "KEYWORD") == 0)
	{
		n=alloc_search(head);
		n->type=search_msgkeyword;
		if (!(n->as=strdup(getword(ptr))))
			write_error_exit(0);
	}
	else if (strcmp(w, "FROM") == 0 ||
		 strcmp(w, "TO") == 0 ||
		 strcmp(w, "CC") == 0 ||
		 strcmp(w, "BCC") == 0 ||
		 strcmp(w, "SUBJECT") == 0)
	{
		n=alloc_search(head);
		n->type=search_header;
		if (!(n->cs=strdup(w)))
			write_error_exit(0);
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "HEADER") == 0)
	{
		n=alloc_search(head);
		n->type=search_header;
		if (!(n->cs=strdup(getword(ptr))))
			write_error_exit(0);
		up(n->cs);
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "BODY") == 0)
	{
		n=alloc_search(head);
		n->type=search_body;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "TEXT") == 0)
	{
		n=alloc_search(head);
		n->type=search_text;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "BEFORE") == 0)
	{
		n=alloc_search(head);
		n->type=search_before;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "ON") == 0)
	{
		n=alloc_search(head);
		n->type=search_on;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "SINCE") == 0)
	{
		n=alloc_search(head);
		n->type=search_since;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "SENTBEFORE") == 0)
	{
		n=alloc_search(head);
		n->type=search_sentbefore;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "SENTON") == 0)
	{
		n=alloc_search(head);
		n->type=search_senton;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "SINCE") == 0)
	{
		n=alloc_search(head);
		n->type=search_sentsince;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "SMALLER") == 0)
	{
		n=alloc_search(head);
		n->type=search_smaller;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else if (strcmp(w, "LARGER") == 0)
	{
		n=alloc_search(head);
		n->type=search_larger;
		n->as=strdup(getword(ptr));
		if (!n->as)
			write_error_exit(0);
	}
	else
	{
		errno=EINVAL;
		return NULL;
	}

	if (notflag)
	{
		struct searchinfo *p=alloc_search(head);

		p->type=search_not;
		p->a=n;
		n=p;
	}
	return n;
}

static int do_copyto(char *toFolder,
		     int (*do_func)(unsigned long, void *),
		     const char *acls)
{
	int has_quota=0;
	struct copyquotainfo cqinfo;
	struct maildirsize quotainfo;

	cqinfo.destmailbox=toFolder;
	cqinfo.nbytes=0;
	cqinfo.nfiles=0;
	cqinfo.acls=acls;

	if (maildirquota_countfolder(toFolder))
	{
		if (maildir_openquotafile(&quotainfo, ".") == 0)
		{
			if (quotainfo.fd >= 0)
				has_quota=1;
			maildir_closequotafile(&quotainfo);
		}

		if (has_quota > 0 && applymsgset( &calc_quota, &cqinfo ))
			has_quota= -1;
	}

	if (has_quota > 0 && cqinfo.nfiles > 0)
	{
		if (maildir_quota_add_start(".", &quotainfo,
					    cqinfo.nbytes,
					    cqinfo.nfiles,
					    getenv("MAILDIRQUOTA")))
		{
			errno=ENOSPC;
			return (-1);
		}

		maildir_quota_add_end(&quotainfo,
				      cqinfo.nbytes,
				      cqinfo.nfiles);
	}

	return applymsgset(do_func, &cqinfo);
}

static int copyto(char *toFolder, int do_move, const char *acls)
{
	if (!do_move)
		return do_copyto(toFolder, &do_copymsg, acls);

	if (!current_mailbox_shared &&
	    maildirquota_countfolder(current_mailbox) ==
	    maildirquota_countfolder(toFolder))
	{
		if (do_copyto(toFolder, do_movemsg, acls))
			return -1;

		doNoop(0);
		return(0);
	}

	if (do_copyto(toFolder, &do_copymsg, acls))
		return -1;

	applymsgset(&msg_expunge, NULL);
	doNoop(0);
	return 0;
}

struct smap1_search_results {

	unsigned prev_runs;

	unsigned long prev_search_hit;
	unsigned long prev_search_hit_start;
};

static void smap1_search_cb_range(struct smap1_search_results *searchResults)
{
	if (searchResults->prev_runs > 100)
	{
		writes("\n");
		searchResults->prev_runs=0;
	}

	if (searchResults->prev_runs == 0)
		writes("* SEARCH");

	writes(" ");
	writen(searchResults->prev_search_hit_start);
	if (searchResults->prev_search_hit_start !=
	    searchResults->prev_search_hit)
	{
		writes("-");
		writen(searchResults->prev_search_hit);
	}
	++searchResults->prev_runs;
}

static void smap1_search_cb(struct searchinfo *si,
			    struct searchinfo *sihead,
			    int isuid, unsigned long i, void *dummy)
{
	struct smap1_search_results *searchResults=
		(struct smap1_search_results *)dummy;

	++i;

	if (searchResults->prev_search_hit == 0)
	{
		searchResults->prev_search_hit=
			searchResults->prev_search_hit_start=i;
		return;
	}

	if (i != searchResults->prev_search_hit+1)
	{
		smap1_search_cb_range(searchResults);
		searchResults->prev_search_hit_start=i;
	}

	searchResults->prev_search_hit=i;
}

static void accessdenied(const char *acl_required)
{
	writes("-ERR Access denied: ACL \"");
	writes(acl_required);
	writes("\" is required\n");
}

static int getacl(const char *ident,
		  const maildir_aclt *acl,
		  void *cb_arg)
{
	int *n=(int *)cb_arg;

	if (*n > 5)
	{
		writes("\n");
		*n=0;
	}

	if (*n == 0)
		writes("* GETACL");

	writes(" ");
	smapword(ident);
	writes(" ");
	smapword(maildir_aclt_ascstr(acl));
	++*n;
	return 0;
}

struct setacl_info {
	struct maildir_info minfo;
	char **ptr;
};

static int dosetdeleteacl(void *cb_arg, int);

static int setacl(void *cb_arg)
{
	return dosetdeleteacl(cb_arg, 0);
}

static int deleteacl(void *cb_arg)
{
	return dosetdeleteacl(cb_arg, 1);
}

static int dosetdeleteacl(void *cb_arg, int dodelete)
{
	struct setacl_info *sainfo=(struct setacl_info *)cb_arg;
	char *q;
	int cnt;
	const char *identifier;
	const char *action;
	const char *err_failedrights;
	char *path;

	maildir_aclt_list aclt_list;

	if (read_acls(&aclt_list, &sainfo->minfo) < 0)
	{
		writes("-ERR Unable to read existing ACLS: ");
		writes(strerror(errno));
		writes("\n");
		return 0;
	}

	q=compute_myrights(&aclt_list,
			   sainfo->minfo.owner);

	if (!q || !strchr(q, ACL_ADMINISTER[0]))
	{
		if (q) free(q);
		maildir_aclt_list_destroy(&aclt_list);
		accessdenied(ACL_ADMINISTER);
		return 0;
	}

	free(q);

	while (*(identifier=getword(sainfo->ptr)))
	{
		if (dodelete)
		{
			if (maildir_aclt_list_del(&aclt_list,
						  identifier) < 0)
			{
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;
			}
			continue;
		}

		action=getword(sainfo->ptr);

		if (*action == '+')
		{
			maildir_aclt newacl;
			const maildir_aclt *oldacl;

			if (maildir_aclt_init(&newacl,
					      action+1,
					      NULL) < 0)
			{
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;
			}


			oldacl=maildir_aclt_list_find(&aclt_list,
						      identifier
						      );
			if (oldacl)
			{
				if (maildir_aclt_add(&newacl,
						     NULL,
						     oldacl)
				    < 0)
				{
					maildir_aclt_destroy(&newacl);
					maildir_aclt_list_destroy(&aclt_list);
					writes("-ERR Error: ");
					writes(strerror(errno));
					writes("\n");
					return 0;
				}
			}

			if (maildir_aclt_list_add(&aclt_list,
						  identifier,
						  NULL,
						  &newacl) < 0)
			{
				maildir_aclt_destroy(&newacl);
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;

			}
			maildir_aclt_destroy(&newacl);
			continue;
		}

		if (*action == '-')
		{
			maildir_aclt newacl;
			const maildir_aclt *oldacl;

			oldacl=maildir_aclt_list_find(&aclt_list,
						      identifier
						      );

			if (!oldacl)
				continue;

			if (maildir_aclt_init(&newacl,
					      NULL,
					      oldacl) < 0)
			{
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;
			}


			if (maildir_aclt_del(&newacl,
					     action+1, NULL)
				    < 0)
			{
				maildir_aclt_destroy(&newacl);
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;
			}

			if (strlen(maildir_aclt_ascstr(&newacl))
			    == 0 ?
			    maildir_aclt_list_del(&aclt_list,
						  identifier)
			    :maildir_aclt_list_add(&aclt_list,
						   identifier,
						   NULL,
						   &newacl) < 0)
			{
				maildir_aclt_destroy(&newacl);
				maildir_aclt_list_destroy(&aclt_list);
				writes("-ERR Error: ");
				writes(strerror(errno));
				writes("\n");
				return 0;

			}
			maildir_aclt_destroy(&newacl);
			continue;
		}

		if (strlen(action) == 0 ?
		    maildir_aclt_list_del(&aclt_list,
					  identifier):
		    maildir_aclt_list_add(&aclt_list,
					  identifier,
					  action, NULL) < 0)
		{
			maildir_aclt_list_destroy(&aclt_list);
			writes("-ERR Error: ");
			writes(strerror(errno));
			writes("\n");
			return 0;
		}
	}

	path=maildir_name2dir(".", sainfo->minfo.maildir);

	err_failedrights=NULL;
	if (!path ||
	    maildir_acl_write(&aclt_list, sainfo->minfo.homedir,
			      path[0] == '.' && path[1] == '/'
			      ? path+2:path,
			      sainfo->minfo.owner,
			      &err_failedrights))
	{
		if (path)
			free(path);

		if (err_failedrights)
		{
			writes("* ACLMINIMUM ");
			writes(err_failedrights);
			writes(" ");
			aclminimum(err_failedrights);
			writes("\n");
		}
		writes("-ERR ACL update failed\n");
		maildir_aclt_list_destroy(&aclt_list);
		return 0;
	}

	cnt=0;
	maildir_aclt_list_enum(&aclt_list,
			       getacl, &cnt);
	if (cnt)
		writes("\n");
	maildir_aclt_list_destroy(&aclt_list);

	/* Reread ACLs if the current mailbox's ACLs have changed */

	if (read_acls(&aclt_list, &sainfo->minfo) < 0)
	{
		writes("-ERR Unable to re-read ACLS: ");
		writes(strerror(errno));
		writes("\n");
		return 0;
	}

	maildir_aclt_list_destroy(&aclt_list);
	writes("+OK Updated ACLs\n");
	return 0;
}

static int checkacl(char **folder, struct maildir_info *minfo,
		    const char *acls)
{
	char *q;

	maildir_aclt_list aclt_list;

	if (maildir_info_smap_find(minfo, folder, getenv("AUTHENTICATED")) < 0)
		return -1;

	if (read_acls(&aclt_list, minfo) < 0)
	{
		maildir_info_destroy(minfo);
		return -1;
	}

	q=compute_myrights(&aclt_list, minfo->owner);
	maildir_aclt_list_destroy(&aclt_list);

	while (*acls)
	{
		if (q == NULL || strchr(q, *acls) == NULL)
		{
			if (q) free(q);
			maildir_info_destroy(minfo);
			return -1;
		}
		++acls;
	}
	if (q)
		free(q);
	return 0;
}

void smap()
{
	char buffer[8192];
	char *ptr;
	struct imapflags add_flags;
	int in_add=0;
	char *add_from=NULL;
	char *add_folder=NULL;
	time_t add_internaldate=0;
	char *add_notify=NULL;
	unsigned add_rcpt_count=0;
	struct libmail_kwMessage *addKeywords=NULL;

	struct add_rcptlist *add_rcpt_list=NULL;

	char rights_buf[40];

	enabled_utf8=1;
	imapscan_init(&current_maildir_info);
	memset(&add_flags, 0, sizeof(add_flags));

#define GETFOLDER(acl) ( strcpy(rights_buf, (acl)), \
			getAccessToFolder(&ptr, rights_buf))

	for (;;)
	{
		char *p;

		writeflush();
		read_timeout(30 * 60);
		smap_readline(buffer, sizeof(buffer));

		ptr=buffer;

		p=getword(&ptr);
		up(p);

		if (strcmp(p, "ADD") == 0)
		{
			char **argvec;
			const char *okmsg="So far, so good...";
			int err_sent=0;

			in_add=1;
			while (*(p=getword(&ptr)))
			{
				char *q=strchr(p, '=');

				if (q)
					*q++=0;
				up(p);

				if (strcmp(p, "FOLDER") == 0)
				{
					if (add_folder)
						free(add_folder);

					add_folder=
						GETFOLDER(ACL_INSERT
							  ACL_DELETEMSGS
							  ACL_SEEN
							  ACL_WRITE);
					if (!add_folder)
					{
						writes("-ERR Invalid folder: ");
						writes(strerror(errno));
						writes("\n");
						break;
					}

					if (strchr(rights_buf,
						   ACL_INSERT[0])
					    == NULL)
					{
						accessdenied(ACL_INSERT);
						free(add_folder);
						add_folder=NULL;
						break;
					}

					okmsg="Will add to this folder";
				}

				if (strcmp(p, "MAILFROM") == 0 && q)
				{
					if (add_from)
						free(add_from);
					if ((add_from=strdup(q)) == NULL)
					{
						writes("-ERR ");
						writes(strerror(errno));
						writes("\n");
						break;
					}
					okmsg="MAIL FROM set";
				}

				if (strcmp(p, "NOTIFY") == 0 && q)
				{
					if (add_notify)
						free(add_notify);
					if ((add_notify=strdup(q)) == NULL)
					{
						writes("-ERR ");
						writes(strerror(errno));
						writes("\n");
						break;
					}
					okmsg="NOTIFY set";
				}

				if (strcmp(p, "RCPTTO") == 0 && q)
				{
					struct add_rcptlist *rcpt=
						malloc(sizeof(struct
							      add_rcptlist));

					if (rcpt == NULL ||
					    (rcpt->rcptto=strdup(q)) == NULL)
					{
						if (rcpt)
							free(rcpt);
						writes("-ERR ");
						writes(strerror(errno));
						writes("\n");
						break;
					}
					rcpt->next=add_rcpt_list;
					add_rcpt_list=rcpt;
					++add_rcpt_count;
					okmsg="RCPT TO set";
				}

				if (strcmp(p, "FLAGS") == 0 && q)
				{
					memset(&add_flags, 0,
					       sizeof(add_flags));
					*--q='=';
					parseflags(q, &add_flags);

					if (strchr(rights_buf,
						   ACL_SEEN[0])
					    == NULL)
						add_flags.seen=0;
					if (strchr(rights_buf,
						   ACL_DELETEMSGS[0])
					    == NULL)
						add_flags.deleted=0;
					if (strchr(rights_buf,
						   ACL_WRITE[0])
					    == NULL)
						add_flags.answered=
							add_flags.flagged=
							add_flags.drafts=0;

					okmsg="FLAGS set";
				}

				if (strcmp(p, "KEYWORDS") == 0 && q &&
				    keywords() && strchr(rights_buf,
							 ACL_WRITE[0]))
				{
					if (addKeywords)
						libmail_kwmDestroy(addKeywords);

					addKeywords=libmail_kwmCreate();

					if (addKeywords == NULL)
					{
						write_error_exit(0);
					}

					*--q='=';
					parsekeywords(q, &addKeywords);
					okmsg="KEYWORDS set";
				}

				if (strcmp(p, "INTERNALDATE") == 0 && q)
				{
					if (rfc822_parsedate_chk(q,
								 &add_internaldate)
					    == 0)
						okmsg="INTERNALDATE set";
				}

				if (p[0] == '{')
				{
					char *tmpname, *newname;
					char *s;
					char *tmpKeywords=NULL;
					char *newKeywords=NULL;
					FILE *fp;
					unsigned long n;

					fp=maildir_mkfilename(add_folder
							      ?add_folder
							      :".",
							      &add_flags,
							      0,
							      &tmpname,
							      &newname);

					if (!fp)
					{
						writes("-ERR ");
						writes(strerror(errno));
						writes("\n");
						break;
					}

					fixnewfilename(newname);

					current_temp_fd=fileno(fp);
					current_temp_fn=tmpname;

					n=add_msg(fp, p+1, buffer,
						  sizeof(buffer));

					if (n)
					{
						s=maildir_requota(newname, n);

						if (!s)
							n=0;
						else
						{
							free(newname);
							newname=s;
						}
					}

					current_temp_fd= -1;
					current_temp_fn= NULL;

					if (n > 0 && add_folder &&
					    maildirquota_countfolder(add_folder)
					    && maildirquota_countfile(newname))
					{
						struct maildirsize quotainfo;

						if (maildir_quota_add_start(add_folder, &quotainfo, n, 1,
									    getenv("MAILDIRQUOTA")))
						{
							errno=ENOSPC;
							n=0;
						}
						else
							maildir_quota_add_end(&quotainfo, n, 1);
					}

					fclose(fp);

					chmod(tmpname, 0600);

					if (add_folder && n && addKeywords)
					{
						if (maildir_kwSave(add_folder,
								   strrchr(newname, '/')+1,
								   addKeywords,
								   &tmpKeywords,
								   &newKeywords,
								   0))
						{
							tmpKeywords=NULL;
							newKeywords=NULL;
							n=0;
							perror("maildir_kwSave");
						}
					}

					argvec=NULL;

					if (add_rcpt_count > 0 && n)
					{
						argvec=malloc(sizeof(char *)
							      * (add_rcpt_count
								 +10));

						if (!argvec)
							n=0;
					}

					if (argvec)
					{
						int i=1;
						struct add_rcptlist *l;

						argvec[i++]="-oi";

						argvec[i++]="-f";
						argvec[i++]=add_from
							? add_from:
							(char *)
							defaultSendFrom();

						if (add_notify)
						{
							argvec[i++]="-N";
							argvec[i++]=add_notify;
						}

						for (l=add_rcpt_list; l;
						     l=l->next)
						{
							argvec[i++]=l->rcptto;
						}
						argvec[i]=0;

						i=imapd_sendmsg(tmpname, argvec,
								&senderr);
						free(argvec);
						if (i)
						{
							n=0;
							err_sent=1;
						}
					}

					if (tmpKeywords)
					{
						rename(tmpKeywords,
						       newKeywords);
						free(tmpKeywords);
						free(newKeywords);
					}

					if (add_folder && n)
					{
						if (maildir_movetmpnew(tmpname,
								       newname)
						    )
							n=0;
						else
						{
							if (add_internaldate)
								set_time(newname,
									 add_internaldate);
							adduid(newname);
						}
					}

					if (n == 0)
					{
						unlink(tmpname);
						free(tmpname);
						free(newname);
						if (!err_sent)
						{
							writes("-ERR ");
							writes(strerror(errno));
							writes("\n");
						}
						break;
					}

					unlink(tmpname);

					free(tmpname);
					free(newname);
					okmsg="Message saved";
					p=NULL;
					break;
				}
			}

			if (p && *p)
				continue; /* Error inside the loop */

			writes("+OK ");
			writes(okmsg);
			writes("\n");

			if (p)
				continue;
		}

		if (in_add)
		{
			struct add_rcptlist *l;

			while ((l=add_rcpt_list) != NULL)
			{
				add_rcpt_list=l->next;
				free(l->rcptto);
				free(l);
			}
			memset(&add_flags, 0, sizeof(add_flags));
			if (add_from)
				free(add_from);
			if (add_folder)
				free(add_folder);
			if (add_notify)
				free(add_notify);

			if (addKeywords)
				libmail_kwmDestroy(addKeywords);

			in_add=0;
			add_from=NULL;
			add_folder=NULL;
			add_internaldate=0;
			add_notify=NULL;
			addKeywords=NULL;
			add_rcpt_count=0;
			if (!p)
				continue; /* Just added a message */
		}

		if (strcmp(p, "LOGOUT") == 0)
			break;

		if (strcmp(p, "RSET") == 0)
		{
			writes("+OK Reset\n");
			continue;
		}

		if (strcmp(p, "GETACL") == 0 ||
		    strcmp(p, "ACL") == 0)
		{
			char **fn=fn_fromwords(&ptr);
			struct maildir_info minfo;
			maildir_aclt_list aclt_list;
			char *q;
			int cnt;

			if (!fn)
			{
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (maildir_info_smap_find(&minfo, fn,
						   getenv("AUTHENTICATED"))
			    < 0)
			{
				maildir_smapfn_free(fn);
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (read_acls(&aclt_list, &minfo) < 0)
			{
				maildir_info_destroy(&minfo);
				maildir_smapfn_free(fn);
				writes("-ERR Unable to read"
				       " existing ACLS: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			q=compute_myrights(&aclt_list,
					   minfo.owner);

			if (!q ||
			    strcmp(p, "ACL") ?
			    !strchr(q, ACL_ADMINISTER[0])
			    :
			    !maildir_acl_canlistrights(q)
			    )
			{
				if (q) free(q);
				maildir_aclt_list_destroy(&aclt_list);
				maildir_info_destroy(&minfo);
				maildir_smapfn_free(fn);
				accessdenied(ACL_ADMINISTER);
				continue;
			}
			if (strcmp(p, "ACL") == 0)
			{
				writes("* ACL ");
				smapword(q);
				writes("\n");
				free(q);
			}
			else
			{
				free(q);
				cnt=0;
				maildir_aclt_list_enum(&aclt_list,
						       getacl, &cnt);
				if (cnt)
					writes("\n");
			}
			maildir_aclt_list_destroy(&aclt_list);
			maildir_info_destroy(&minfo);
			maildir_smapfn_free(fn);
			writes("+OK ACLs retrieved\n");
			continue;
		}

		if (strcmp(p, "SETACL") == 0 ||
		    strcmp(p, "DELETEACL") == 0)
		{
			char **fn=fn_fromwords(&ptr);
			struct setacl_info sainfo;

			if (!fn)
			{
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (maildir_info_smap_find(&sainfo.minfo,
						   fn, getenv("AUTHENTICATED"))
			    < 0)
			{
				maildir_smapfn_free(fn);
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}


			sainfo.ptr= &ptr;

			acl_lock(sainfo.minfo.homedir,
				 *p == 'S' ? setacl:deleteacl,
				 &sainfo);

			maildir_smapfn_free(fn);
			maildir_info_destroy(&sainfo.minfo);
			continue;
		}

		if (strcmp(p, "LIST") == 0)
		{
			struct list_hier *hier=NULL;

			listcmd(&hier, &hier, &ptr);
			continue;
		}

		if (strcmp(p, "STATUS") == 0)
		{
			char *t;
			struct imapscaninfo other_info, *loaded_infoptr,
				*infoptr;
			unsigned long n, i;

			getword(&ptr);

			t=GETFOLDER(ACL_LOOKUP);

			if (!t)
			{
				writes("-ERR Cannot read folder status: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (strchr(rights_buf, ACL_LOOKUP[0]) == NULL)
			{
				accessdenied(ACL_LOOKUP);
				continue;
			}

			if (current_mailbox &&
			    strcmp(current_mailbox, t) == 0)
			{
				loaded_infoptr=0;
				infoptr= &current_maildir_info;
			}
			else
			{
				loaded_infoptr= &other_info;
				infoptr=loaded_infoptr;

				imapscan_init(loaded_infoptr);

				if (imapscan_maildir(infoptr, t, 1, 1, NULL))
				{
					writes("-ERR Cannot read"
					       " folder status: ");
					writes(strerror(errno));
					writes("\n");
					continue;
				}
			}

			writes("* STATUS EXISTS=");
			writen(infoptr->nmessages+infoptr->left_unseen);

			n=infoptr->left_unseen;

			for (i=0; i<infoptr->nmessages; i++)
			{
				const char *p=infoptr->msgs[i].filename;

				p=strrchr(p, MDIRSEP[0]);
				if (p && strncmp(p, MDIRSEP "2,", 3) == 0 &&
				    strchr(p, 'S'))	continue;
				++n;
			}
			writes(" UNSEEN=");
			writen(n);
			writes("\n+OK Folder status retrieved\n");
			if (loaded_infoptr)
				imapscan_free(loaded_infoptr);
			continue;
		}

		if (strcmp(p, "CREATE") == 0)
		{
			char *t;

			strcpy(rights_buf, ACL_CREATE);
			t=getCreateFolder(&ptr, rights_buf);

			if (t)
			{
				if (mdcreate(t))
				{
					writes("-ERR Cannot create folder: ");
					writes(strerror(errno));
					writes("\n");
				}
				else
				{
					writes("+OK Folder created\n");
				}
				free(t);
			}
			else
			{
				if (rights_buf[0] == 0)
					accessdenied(ACL_CREATE);
				else
				{
					writes("-ERR Cannot create folder: ");
					writes(strerror(errno));
					writes("\n");
				}
			}
			continue;
		}

		if (strcmp(p, "MKDIR") == 0)
		{
			char *t;

			strcpy(rights_buf, ACL_CREATE);
			t=getCreateFolder(&ptr, rights_buf);

			if (t)
			{
				writes("+OK Folder created\n");
				free(t);
			}
			else if (rights_buf[0] == 0)
				accessdenied(ACL_CREATE);
			else
			{
				writes("-ERR Cannot create folder: ");
				writes(strerror(errno));
				writes("\n");
			}
			continue;
		}

		if (strcmp(p, "RMDIR") == 0)
		{
			char *t;

			strcpy(rights_buf, ACL_DELETEFOLDER);
			t=getCreateFolder(&ptr, rights_buf);

			if (t)
			{
				writes("+OK Folder deleted\n");
				free(t);
			}
			else if (rights_buf[0] == 0)
				accessdenied(ACL_DELETEFOLDER);
			else
			{
				writes("-ERR Cannot create folder: ");
				writes(strerror(errno));
				writes("\n");
			}
			continue;
		}

		if (strcmp(p, "DELETE") == 0)
		{
			char **fn;
			char *t=NULL;

			fn=fn_fromwords(&ptr);

			if (fn)
			{
				struct maildir_info minfo;

				if (maildir_info_smap_find(&minfo, fn,
							   getenv("AUTHENTICATED")) == 0)
				{
					if (minfo.homedir && minfo.maildir)
					{
						maildir_aclt_list list;
						char *q;

						if (strcmp(minfo.maildir,
							   INBOX) == 0)
						{
							writes("-ERR INBOX may"
							       " not be deleted\n");
							maildir_info_destroy(&minfo);
							continue;
						}

						if (read_acls(&list, &minfo)
						    < 0)
						{
							maildir_info_destroy(&minfo);
							accessdenied(ACL_DELETEFOLDER);
							continue;
						}

						q=compute_myrights(&list,
								   minfo.owner
								   );

						if (!q ||
						    strchr(q,
							   ACL_DELETEFOLDER[0])
						    == NULL)
						{
							if (q)
								free(q);
							maildir_info_destroy(&minfo);
							accessdenied(ACL_DELETEFOLDER);
							continue;
						}
						free(q);
						maildir_aclt_list_destroy(&list);
						t=maildir_name2dir(minfo.homedir,
								   minfo.maildir);
					}
					maildir_info_destroy(&minfo);
				}
			}

			if (t && current_mailbox &&
			    strcmp(t, current_mailbox) == 0)
			{
				writes("-ERR Cannot DELETE currently open folder.\n");
				free(t);
				continue;
			}


			if (t)
			{
				if (mddelete(t) == 0)
				{
					maildir_quota_recalculate(".");
					writes("+OK Folder deleted");
				}
				else
				{
					writes("-ERR Cannot delete folder: ");
					writes(strerror(errno));
				}
				writes("\n");
				free(t);

			}
			else
			{
				if (t)
				{
					free(t);
					errno=EINVAL;
				}

				writes("-ERR Unable to delete folder: ");
				writes(strerror(errno));
				writes("\n");
			}
			continue;
		}

		if (strcmp(p, "RENAME") == 0)
		{
			struct maildir_info msrc, mdst;
			char **fnsrc, **fndst;
			size_t i;
			char *save;
			const char *errmsg;

			if ((fnsrc=fn_fromwords(&ptr)) == NULL)
			{
				writes("-ERR ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if ((fndst=fn_fromwords(&ptr)) == NULL)
			{
				maildir_smapfn_free(fnsrc);
				writes("-ERR ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			for (i=0; fndst[i]; i++)
				;

			if (i == 0)
			{
				maildir_smapfn_free(fnsrc);
				maildir_smapfn_free(fndst);
				writes("-ERR Invalid destination folder name\n");
				continue;
			}

			if (checkacl(fnsrc, &msrc, ACL_DELETEFOLDER))
			{
				maildir_smapfn_free(fnsrc);
				maildir_smapfn_free(fndst);
				accessdenied(ACL_DELETEFOLDER);
				continue;
			}
			save=fndst[--i];
			fndst[i]=NULL;

			if (checkacl(fndst, &mdst, ACL_CREATE))
			{
				fndst[i]=save;
				maildir_smapfn_free(fnsrc);
				maildir_smapfn_free(fndst);
				maildir_info_destroy(&msrc);
				accessdenied(ACL_CREATE);
				continue;
			}

			fndst[i]=save;

			maildir_info_destroy(&mdst);

			if (maildir_info_smap_find(&mdst, fndst,
						   getenv("AUTHENTICATED")) < 0)
			{
				maildir_smapfn_free(fnsrc);
				maildir_smapfn_free(fndst);
				maildir_info_destroy(&msrc);
				writes("-ERR Internal error in RENAME: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (folder_rename(&msrc, &mdst, &errmsg))
			{
				writes("-ERR ");
				writes(*errmsg == '@' ? errmsg+1:errmsg);
				if (*errmsg == '@')
					writes(strerror(errno));
				writes("\n");
			}
			else
			{
				writes("+OK Folder renamed.\n");
			}
			maildir_info_destroy(&msrc);
			maildir_info_destroy(&mdst);
			continue;
		}

		if (strcmp(p, "OPEN") == 0 ||
		    strcmp(p, "SOPEN") == 0)
		{
			char **fn;
			char *q;
			const char *snapshot=0;
			struct maildir_info minfo;
			maildir_aclt_list aclt_list;

			if (current_mailbox)
			{
				free(current_mailbox);
				imapscan_free(&current_maildir_info);
				imapscan_init(&current_maildir_info);
				current_mailbox=0;
			}
			if (current_mailbox_acl)
				free(current_mailbox_acl);
			current_mailbox_acl=NULL;
			current_mailbox_shared=0;

			fetch_free_cache();

			if (p[0] == 'S')
				snapshot=getword(&ptr);

			fn=fn_fromwords(&ptr);

			if (!fn)
			{
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (maildir_info_smap_find(&minfo, fn,
						   getenv("AUTHENTICATED"))
			    < 0)
			{
				maildir_smapfn_free(fn);
				writes("-ERR Invalid folder: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			if (read_acls(&aclt_list, &minfo) < 0)
			{
				maildir_info_destroy(&minfo);
				maildir_smapfn_free(fn);
				writes("-ERR Unable to read"
				       " existing ACLS: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}

			q=compute_myrights(&aclt_list, minfo.owner);
			maildir_aclt_list_destroy(&aclt_list);
			maildir_smapfn_free(fn);

			if (!q || strchr(q, ACL_READ[0]) == NULL)
			{
				if (q) free(q);
				maildir_info_destroy(&minfo);
				accessdenied(ACL_READ);
				maildir_info_destroy(&minfo);
				continue;
			}
			current_mailbox_acl=q;
			current_mailbox=maildir_name2dir(minfo.homedir,
							 minfo.maildir);

			if (current_mailbox == NULL)
			{
				fprintf(stderr, "ERR: Internal error"
					" in maildir_name2dir(%s,%s)\n",
					minfo.homedir,
					minfo.maildir);
				maildir_info_destroy(&minfo);
				continue;
			}
			maildir_info_destroy(&minfo);

			snapshot_select(snapshot != NULL);

			if (snapshot_init(current_mailbox, snapshot))
			{
				writes("* SNAPSHOTEXISTS ");
				smapword(snapshot);
				writes("\n");
				smap1_noop(0);
				continue;
			}

			if (imapscan_maildir(&current_maildir_info,
					     current_mailbox, 0, 0, NULL) == 0)
			{
				snapshot_init(current_mailbox, NULL);
				writes("* EXISTS ");
				writen(current_maildir_info.nmessages);
				writes("\n+OK Folder opened\n");
				continue;
			}

			writes("-ERR Cannot open the folder: ");
			writes(strerror(errno));
			writes("\n");

			free(current_mailbox);
			current_mailbox=NULL;
			continue;
		}

		if (strcmp(p, "CLOSE") == 0)
		{
			if (current_mailbox)
			{
				free(current_mailbox);
				imapscan_free(&current_maildir_info);
				imapscan_init(&current_maildir_info);
				current_mailbox=0;
			}
			writes("+OK Folder closed\n");
			continue;
		}

		if (strcmp(p, "NOOP") == 0)
		{
			smap1_noop(1);
			continue;
		}

		if (strcmp(p, "IDLE") == 0)
		{
			if ((p=getenv("IMAP_ENHANCEDIDLE")) == NULL
			    || !atoi(p)
			    || imapenhancedidle())
				imapidle();

			read_timeout(60);
			smap_readline(buffer, sizeof(buffer));
			ptr=buffer;
			p=getword(&ptr);
			up(p);
			if (strcmp(p, "RESUME"))
			{
				writes("-ERR RESUME is required to follow IDLE\n");
			}
			else
				writes("+OK Resumed...\n");
			continue;
		}

		if (!current_mailbox)
			p=""; /* FALLTHROUGH */

		if (strcmp(p, "EXPUNGE") == 0)
		{
			int hasSet;

			p=markmsgset(&ptr, &hasSet);

			if (p)
			{
				if (strchr(current_mailbox_acl,
					   ACL_EXPUNGE[0]) == NULL)
				{
					accessdenied(ACL_EXPUNGE);
					continue;
				}

				if (hasSet)
				{
					if (strchr(current_mailbox_acl,
						   ACL_DELETEMSGS[0]) == NULL)
					{
						accessdenied(ACL_DELETEMSGS);
						continue;
					}

					applymsgset( &msg_expunge, NULL);
				}
				else
					expunge();
				smap1_noop(0);
				continue;
			}
		}

		if (strcmp(p, "STORE") == 0)
		{
			struct storeinfo si;
			int dummy;

			p=markmsgset(&ptr, &dummy);

			dummy=0;

			if (!p)
				dummy=1;

			while (p && *p)
			{
				char *q=strchr(p, '=');

				if (q)
					*q=0;
				up(p);
				/* Uppercase only the keyword, for now */
				if (q)
					*q='=';

				if (strncmp(p, "FLAGS=", 6) == 0)
				{
					memset(&si, 0, sizeof(si));
					up(p);
					parseflags(p, &si.flags);
					if ((dummy=applymsgset(&applyflags,
							       &si)) != 0)
						break;
				}
				else if (strncmp(p, "+FLAGS=", 7) == 0 ||
					 strncmp(p, "-FLAGS=", 7) == 0)
				{
					memset(&si, 0, sizeof(si));
					up(p);
					si.plusminus=p[0];
					parseflags(p, &si.flags);
					if ((dummy=applymsgset(&applyflags,
							       &si)) != 0)
						break;
				}
				else if (strncmp(p, "KEYWORDS=", 9) == 0 &&
					 keywords())
				{
					struct libmail_kwMessage *kwm;

					memset(&si, 0, sizeof(si));
					kwm=si.keywords=libmail_kwmCreate();

					if (!kwm)
						write_error_exit(0);

					parsekeywords(p, &si.keywords);
					dummy=applymsgset(&applyflags,
							  &si);

					libmail_kwmDestroy(kwm);

					if (dummy != 0)
						break;
				}
				else if ((strncmp(p, "+KEYWORDS=", 10) == 0 ||
					  strncmp(p, "-KEYWORDS=", 10) == 0) &&
					 keywords())
				{
					memset(&si, 0, sizeof(si));
					si.keywords=libmail_kwmCreate();

					if (!si.keywords)
						write_error_exit(0);
					si.plusminus=p[0];
					parsekeywords(p, &si.keywords);
					dummy=applymsgset(&applyflags,
							  &si);

					if (dummy == 0)
						dummy=addRemoveSmapKeywords(&si);
					libmail_kwmDestroy(si.keywords);

					if (dummy != 0)
						break;
				}
				else if (strncmp(p, "INTERNALDATE=", 13) == 0)
				{
					time_t t;

					up(p);

					if (rfc822_parsedate_chk(p+13, &t)
					    == 0 &&
					    (dummy=applymsgset(&setdate, &t))
					    != 0)
						break;
				}

				p=getword(&ptr);
			}
			if (dummy)
			{
				writes("-ERR Cannot update folder status: ");
				writes(strerror(errno));
				writes("\n");
			}
			else
				writes("+OK Folder status updated\n");
			continue;
		}

		if (strcmp(p, "FETCH") == 0)
		{
			int dummy;
			struct smapfetchinfo fi;
			int fetch_items=0;

			for (p=markmsgset(&ptr, &dummy);
			     p && *p; p=getword(&ptr))
			{
				if ((fi.entity=strchr(p, '=')) == NULL)
				{
					up(p);

					if (strcmp(p, "UID") == 0)
						fetch_items |= FETCH_UID;
					if (strcmp(p, "SIZE") == 0)
						fetch_items |= FETCH_SIZE;
					if (strcmp(p, "FLAGS") == 0)
						fetch_items |= FETCH_FLAGS;
					if (strcmp(p, "KEYWORDS") == 0)
						fetch_items |= FETCH_KEYWORDS;
					if (strcmp(p, "INTERNALDATE") == 0)
						fetch_items
							|= FETCH_INTERNALDATE;
					continue;
				}

				*fi.entity++=0;

				fi.hdrs=strrchr(fi.entity, '(');
				if (fi.hdrs)
				{
					char *q;

					*fi.hdrs++=0;

					q=strrchr(fi.hdrs, ')');
					if (q)
						*q=0;
					up(fi.hdrs);
				}

				fi.mimeid=strrchr(fi.entity, '[');
				if (fi.mimeid)
				{
					char *q;

					*fi.mimeid++=0;
					q=strrchr(fi.mimeid, ']');
					if (q)
						*q=0;
				}

				up(p);

				if (strcmp(p, "CONTENTS") == 0 ||
				    strcmp(p, "CONTENTS.PEEK") == 0)
				{
					fi.peek=strchr(p, '.') != NULL;
					if (applymsgset(&do_fetch, &fi) == 0)
					{
						continue;
					}
				}
				else
				{
					continue;
				}

				writes("-ERR Cannot retrieve message: ");
				writes(strerror(errno));
				writes("\n");
				break;
			}

			if (!p || !*p)
			{
				if (fetch_items &&
				    applymsgset(&do_attrfetch, &fetch_items))
				{
					writes("-ERR Cannot retrieve message: ");
					writes(strerror(errno));
					writes("\n");
				}
				else
					writes("+OK Message retrieved.\n");
			}
			continue;
		}

		if (strcmp(p, "COPY") == 0
		    || strcmp(p, "MOVE") == 0)
		{
			int dummy;
			int domove= *p == 'M';

			p=markmsgset(&ptr, &dummy);

			if (dummy && *p == 0)
			{
				p=GETFOLDER(ACL_INSERT
					    ACL_DELETEMSGS
					    ACL_SEEN
					    ACL_WRITE);

				if (p)
				{
					if (strchr(rights_buf, ACL_INSERT[0])
					    == NULL)
					{
						free(p);
						accessdenied(ACL_INSERT);
						continue;
					}

					if (copyto(p, domove, rights_buf) == 0)
					{
						free(p);
						writes("+OK Messages copied.\n"
						       );
						continue;
					}
					free(p);
				}

				writes("-ERR Cannot copy messages: ");
				writes(strerror(errno));
				writes("\n");
				continue;
			}
			writes("-ERR Syntax error.\n");
			continue;
		}

		if (strcmp(p, "SEARCH") == 0)
		{
			struct searchinfo *searchInfo=NULL;
			struct searchinfo *si;
			struct smap1_search_results searchResults;

			if ((si=createSearch(&searchInfo, &ptr)) == NULL)
			{
				writes("-ERR SEARCH failed: ");
				writes(strerror(errno));
				writes("\n");
				free_search(searchInfo);
				continue;
			}

			searchResults.prev_runs=0;
			searchResults.prev_search_hit=0;
			searchResults.prev_search_hit_start=0;

			search_internal(si, searchInfo, "utf-8", 0,
					smap1_search_cb, &searchResults);

			if (searchResults.prev_search_hit)
				smap1_search_cb_range(&searchResults);

			if (searchResults.prev_runs)
				writes("\n");

			writes("+OK Search completed.\n");
			free_search(searchInfo);
			continue;
		}


		writes("-ERR Syntax error.\n");
	}

	writes("* BYE Courier-SMAP server shutting down\n"
	       "+OK LOGOUT completed\n");
	writeflush();
}
