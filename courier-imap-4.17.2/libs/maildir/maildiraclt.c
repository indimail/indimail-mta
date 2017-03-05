/*
** Copyright 2003-2012 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildiraclt.h"
#include	"maildirmisc.h"
#include	"maildircreate.h"
#include	<time.h>
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
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>


int maildir_acl_disabled=0;

static int compar_aclt(const void *a, const void *b)
{
	char ca=*(const char *)a;
	char cb=*(const char *)b;

	return (int)(unsigned char)ca - (int)(unsigned char)cb;
}

/* Post-op fixup of an aclt: collate, remove dupes. */

static void fixup(maildir_aclt *aclt)
{
	char *a, *b;

	qsort(*aclt, strlen(*aclt), 1, compar_aclt);

	for (a=b=*aclt; *a; a++)
	{
		if (*a == a[1])
			continue;
		if ((int)(unsigned char)*a <= ' ')
			continue; /* Silently drop bad access rights */

		*b++= *a;
	}
	*b=0;
}

static int validacl(const char *p)
{
	while (*p)
	{
		if ((int)(unsigned char)*p <= ' ')
		{
			errno=EINVAL;
			return -1;
		}
		++p;
	}

	return 0;
}

int maildir_aclt_init(maildir_aclt *aclt,
		      const char *initvalue_cstr,
		      const maildir_aclt *initvalue_cpy)
{
	if (initvalue_cpy)
		initvalue_cstr= *initvalue_cpy;

	*aclt=NULL;

	if (!initvalue_cstr || !*initvalue_cstr)
		return 0;

	if (validacl(initvalue_cstr) < 0)
		return -1;

	if ( (*aclt=strdup(initvalue_cstr)) == NULL)
		return -1;
	fixup(aclt);
	return 0;
}

/* Destroy an aclt after it is no longer used. */

void maildir_aclt_destroy(maildir_aclt *aclt)
{
	if (*aclt)
		free(*aclt);
}


/* Add or remove access chars. */

int maildir_aclt_add(maildir_aclt *aclt,
		     const char *add_strs,
		     const maildir_aclt *add_aclt)
{
	if (add_aclt)
		add_strs= *add_aclt;

	if (!add_strs || !*add_strs)
		return 0;

	if (validacl(add_strs) < 0)
		return -1;

	if (*aclt)
	{
		char *p=realloc(*aclt, strlen(*aclt)+strlen(add_strs)+1);

		if (!p)
			return -1;
		strcat(p, add_strs);
		*aclt=p;

	}
	else if ( ((*aclt)=strdup(add_strs)) == NULL)
		return -1;

	fixup(aclt);
	return 0;
}

int maildir_aclt_del(maildir_aclt *aclt,
		     const char *del_strs,
		     const maildir_aclt *del_aclt)
{
	char *a, *b;

	if (del_aclt)
		del_strs= *del_aclt;

	if (!del_strs)
		return 0;

	if (!*aclt)
		return 0;

	for (a=b=*aclt; *a; a++)
	{
		if (strchr(del_strs, *a))
			continue;
		*b++= *a;
	}
	*b=0;

	if (**aclt == 0)
	{
		free(*aclt);
		*aclt=NULL;
	}
	return 0;
}

/* -------------------------------------------------------------------- */


void maildir_aclt_list_init(maildir_aclt_list *aclt_list)
{
	aclt_list->head=NULL;
	aclt_list->tail=NULL;
}

void maildir_aclt_list_destroy(maildir_aclt_list *aclt_list)
{
	struct maildir_aclt_node *p;

	for (p=aclt_list->head; p; )
	{
		struct maildir_aclt_node *q=p->next;

		free(p->identifier);
		maildir_aclt_destroy(&p->acl);
		free(p);
		p=q;
	}
	maildir_aclt_list_init(aclt_list);
}


/* Add an <identifier,acl> pair.  Returns 0 on success, -1 on failure */

int maildir_aclt_list_add(maildir_aclt_list *aclt_list,
			  const char *identifier,
			  const char *aclt_str,
			  maildir_aclt *aclt_cpy)
{
	struct maildir_aclt_node *p;
	const char *q;

	/* Check for valid identifiers */

	for (q=identifier; *q; q++)
		if ( (int)(unsigned char)*q <= ' ')
		{
			errno=EINVAL;
			return -1;
		}

	if (*identifier == 0)
	{
		errno=EINVAL;
		return -1;
	}

	if (aclt_cpy && *aclt_cpy)
		aclt_str= *aclt_cpy;

	for (p=aclt_list->head; p; p=p->next)
	{
		if (strcmp(p->identifier, identifier) == 0)
		{
			maildir_aclt_destroy(&p->acl);
			return maildir_aclt_init(&p->acl, aclt_str, NULL);
		}
	}

	if ((p=malloc(sizeof(*p))) == NULL ||
	    (p->identifier=strdup(identifier)) == NULL)
	{
		if (p) free(p);
		return -1;
	}

	if (maildir_aclt_init(&p->acl, aclt_str, NULL) < 0)
	{
		free(p->identifier);
		free(p);
		return -1;
	}

	p->next=NULL;
	if ((p->prev=aclt_list->tail) != NULL)
		p->prev->next=p;
	else
		aclt_list->head=p;
	aclt_list->tail=p;
	return 0;
}

/*
** Remove 'identifier' from the ACL list.
*/

int maildir_aclt_list_del(maildir_aclt_list *aclt_list,
			  const char *identifier)
{
	struct maildir_aclt_node *p;

	for (p=aclt_list->head; p; p=p->next)
	{
		if (strcmp(p->identifier, identifier) == 0)
		{
			if (p->prev)
				p->prev->next=p->next;
			else aclt_list->head=p->next;

			if (p->next)
				p->next->prev=p->prev;
			else aclt_list->tail=p->prev;

			maildir_aclt_destroy(&p->acl);
			free(p->identifier);
			free(p);
			return 0;
		}
	}
	return 0;
}

/*
** Generic enumeration.
*/

int maildir_aclt_list_enum(maildir_aclt_list *aclt_list,
			   int (*cb_func)(const char *identifier,
					  const maildir_aclt *acl,
					  void *cb_arg),
			   void *cb_arg)
{
	struct maildir_aclt_node *p;
	int rc;

	for (p=aclt_list->head; p; p=p->next)
	{
		rc= (*cb_func)(p->identifier, &p->acl, cb_arg);

		if (rc)
			return rc;
	}
	return 0;
}

const maildir_aclt *maildir_aclt_list_find(maildir_aclt_list *aclt_list,
					   const char *identifier)
{
	struct maildir_aclt_node *p;

	for (p=aclt_list->head; p; p=p->next)
	{
		if (strcmp(p->identifier, identifier) == 0)
			return &p->acl;
	}
	return NULL;
}

/* ---------------------------------------------------------------------- */

static int maildir_acl_read_check(maildir_aclt_list *aclt_list,
				 const char *maildir,
				 const char *path);

int maildir_acl_read(maildir_aclt_list *aclt_list,
		     const char *maildir,
		     const char *path)
{
	int rc=maildir_acl_read_check(aclt_list, maildir, path);
	char *p, *q;

	if (rc)
		maildir_aclt_list_destroy(aclt_list);

	if (rc <= 0)
		return rc;

	/*
	** If the ACL config file for this folder was not found,
	** check for the ACL config file for its parent folder.
	*/

	if ((p=strdup(path)) == NULL)
		return -1;

	strcpy(p, path);

	q=strrchr(p, '.');

	if (!q)
	{
		free(p);
		errno=EIO; /* Should not happen */
		return -1;
	}

	*q=0;

	rc=maildir_acl_read(aclt_list, maildir, p);
	if (rc == 0)
	{
		/* Make sure to save the default acl list */

		rc=maildir_acl_write(aclt_list, maildir, path, NULL, NULL);
		if (rc >= 0) /* Ok if rc=1 */
			rc=0;
		if (rc)
			maildir_aclt_list_destroy(aclt_list);
	}
	free(p);
	return rc;
}

/*
** Attempt to retrieve the ACL set for the specified folder.
**
** Returns -1 if error.
** Returns 0 if the ACL was retrieved.
** Returns 1 if the ACL configuration file does not exist.
*/

static int maildir_aclt_add_default_admin(maildir_aclt_list *aclt_list);

static int maildir_acl_read_check(maildir_aclt_list *aclt_list,
				  const char *maildir,
				  const char *path)
{
	char *p, *q;
	FILE *fp;
	char buffer[BUFSIZ];

	maildir_aclt_list_init(aclt_list);

	if (!maildir || !*maildir)
		maildir=".";
	if (!path || !*path)
		path=".";

	if (strchr(path, '/') || *path != '.')
	{
		errno=EINVAL;
		return -1;
	}

	if (maildir_acl_disabled)
	{
		if (maildir_aclt_list_add(aclt_list, "owner",
					  ACL_LOOKUP ACL_READ
					  ACL_SEEN ACL_WRITE ACL_INSERT
					  ACL_CREATE
					  ACL_DELETEFOLDER
					  ACL_DELETEMSGS ACL_EXPUNGE,
					  NULL) < 0 ||
		    maildir_aclt_add_default_admin(aclt_list))
		{
			maildir_aclt_list_destroy(aclt_list);
			return -1;
		}
		return 0;
	}

	p=malloc(strlen(maildir)+strlen(path)+2);

	if (!p)
		return -1;

	strcat(strcat(strcpy(p, maildir), "/"), path);

	q=malloc(strlen(p)+sizeof("/" ACLFILE));
	if (!q)
	{
		free(p);
		return -1;
	}
	fp=fopen(strcat(strcpy(q, p), "/" ACLFILE), "r");
	free(p);
	free(q);

	if (fp == NULL)
	{
		if (strcmp(path, ".") == 0)
		{
			/* INBOX ACL default */

			if (maildir_aclt_list_add(aclt_list, "owner",
						  ACL_ALL, NULL) < 0 ||
			    maildir_aclt_add_default_admin(aclt_list))
			{
				return -1;
			}
			return 0;
		}

		q=malloc(strlen(maildir)+sizeof("/" ACLHIERDIR "/") +
			 strlen(path));
		if (!q)
			return -1;

		strcat(strcat(strcpy(q, maildir), "/" ACLHIERDIR "/"),
		       path+1);

		fp=fopen(q, "r");
		free(q);
	}

	if (!fp && errno != ENOENT)
		return -1;

	if (!fp)
		return 1;

	errno=0;

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		char *p=strchr(buffer, '\n');

		if (p) *p=0;

		for (p=buffer; *p; p++)
			if (*p == ' ')
			{
				*p=0;
				do
				{
					++p;
				} while (*p && *p == ' ');
				break;
			}

		if (maildir_aclt_list_add(aclt_list, buffer, p, NULL) < 0)
		{
			if (errno != EINVAL)
				return -1;
			/* Sweep crap in the ACL file under the carpet */
		}
	}
	if (ferror(fp))
	{
		fclose(fp);
		return -1;
	}
	fclose(fp);
	if (maildir_aclt_add_default_admin(aclt_list))
		return -1;
	return 0;
}

/*
** Add the default ACL permissions to the administrators group.
**
** Make sure that the ACL entry for "administrators" includes all
** rights.
**
** Make sure that any ACL entries for "-administrators" or
** "-group=administrators" do not have LOOKUP and ADMIN.
*/

static int maildir_aclt_add_default_admin(maildir_aclt_list *aclt_list)
{
	const maildir_aclt *old_acl;

	static const char * const drop_acls[]={"-administrators",
					       "-group=administrators"};
	size_t i;

	if ((old_acl=maildir_aclt_list_find(aclt_list, "group=administrators"))
	    != NULL)
	{
		maildir_aclt new_acl;

		if (maildir_aclt_init(&new_acl, ACL_ALL, NULL))
			return -1;

		if (maildir_aclt_add(&new_acl, NULL, old_acl) ||
		    maildir_aclt_list_add(aclt_list, "group=administrators",
					  NULL, &new_acl))
		{
			maildir_aclt_destroy(&new_acl);
			return -1;
		}
		maildir_aclt_destroy(&new_acl);
	}
	else
	{
		maildir_aclt new_acl;

		old_acl=maildir_aclt_list_find(aclt_list, "administrators");

		if (maildir_aclt_init(&new_acl, ACL_ALL, NULL))
			return -1;

		if (maildir_aclt_add(&new_acl, NULL, old_acl) ||
		    maildir_aclt_list_add(aclt_list, "administrators",
					  NULL, &new_acl))
		{
			maildir_aclt_destroy(&new_acl);
			return -1;
		}
		maildir_aclt_destroy(&new_acl);
	}

	for (i=0; i<2; i++)
	{
		const char *n=drop_acls[i];
		if (maildir_aclt_list_del(aclt_list, n) < 0)
			return -1;
	}

	return 0;
}

int maildir_acl_delete(const char *maildir,
		       const char *path)
{
	char *p, *q;

#if 0
	if (strcmp(path, SHARED) == 0)
		return 0;

	if (strncmp(path, SHARED ".", sizeof(SHARED)) == 0)
		return 0;
#endif
	if (!maildir || !*maildir)
		maildir=".";
	if (!path || !*path)
		path=".";

	if (strchr(path, '/') || *path != '.')
	{
		errno=EINVAL;
		return -1;
	}

	p=malloc(strlen(maildir)+strlen(path)+2);

	if (!p)
		return -1;

	strcat(strcat(strcpy(p, maildir), "/"), path);

	q=malloc(strlen(p)+sizeof("/" ACLFILE));
	if (!q)
	{
		free(p);
		return -1;
	}

	unlink(strcat(strcpy(q, p), "/" ACLFILE));
	free(p);
	free(q);

	if (strcmp(path, ".") == 0)
	{
		/* INBOX ACL default */

		return 0;
	}

	q=malloc(strlen(maildir)+sizeof("/" ACLHIERDIR "/") +
		 strlen(path));
	if (!q)
	{
		return -1;
	}
	strcat(strcat(strcpy(q, maildir), "/" ACLHIERDIR "/"),
	       path+1);

	unlink(q);
	free(q);
	return 0;
}

static int save_acl(const char *identifier, const maildir_aclt *acl,
		    void *cb_arg);


static int is_owner(const char *isme, void *void_arg)
{
	if (void_arg && strcmp(isme, (const char *)void_arg) == 0)
		return 1;

	return strcmp(isme, "owner") == 0;
}

static int is_admin(const char *isme, void *void_arg)
{
	return strcmp(isme, "administrators") == 0;

	/* We don't need to check for group=administrators, see chk_admin() */
}

static int check_adminrights(maildir_aclt *list)
{
	if (strchr(maildir_aclt_ascstr(list), ACL_LOOKUP[0]) == NULL ||
	    strchr(maildir_aclt_ascstr(list), ACL_ADMINISTER[0]) == NULL)
	{
		maildir_aclt_destroy(list);
		return -1;
	}

	maildir_aclt_destroy(list);
	return 0;
}

static int check_allrights(maildir_aclt *list)
{
	const char *all=ACL_ALL;

	while (*all)
	{
		if (strchr(maildir_aclt_ascstr(list), *all) == NULL)
		{
			maildir_aclt_destroy(list);
			return -1;
		}
		++all;
	}

	maildir_aclt_destroy(list);
	return 0;
}

static int maildir_acl_compute_chkowner(maildir_aclt *aclt,
					maildir_aclt_list *aclt_list,
					int (*cb_func)(const char *isme,
						       void *void_arg),
					void *void_arg,
					int chkowner);

int maildir_acl_write(maildir_aclt_list *aclt_list,
		      const char *maildir,
		      const char *path,

		      const char *owner,
		      const char **err_failedrights)
{
	int trycreate;
	struct maildir_tmpcreate_info tci;
	FILE *fp;
	char *p, *q;
	const char *dummy_string;
	maildir_aclt chkacls;

	if (!err_failedrights)
		err_failedrights= &dummy_string;

	if (!maildir || !*maildir)
		maildir=".";
	if (!path || !*path)
		path=".";

	if (strchr(path, '/') || *path != '.')
	{
		errno=EINVAL;
		return -1;
	}

	if (strcmp(path, ".")) /* Sanity check */
		for (dummy_string=path; *dummy_string; dummy_string++)
			if (*dummy_string == '.' &&
			    (dummy_string[1] == '.' ||
			     dummy_string[1] == 0))
			{
				errno=EINVAL;
				return -1;
			}


	if (maildir_acl_compute_chkowner(&chkacls, aclt_list, is_owner, NULL,
					 0))
	{
		maildir_aclt_destroy(&chkacls);
		errno=EINVAL;
		return -1;
	}

	if (check_adminrights(&chkacls))
	{
		*err_failedrights="owner";
		errno=EINVAL;
		return -1;
	}

	if (owner)
	{
		if (maildir_acl_compute_chkowner(&chkacls, aclt_list, is_owner,
						 (void *)owner, 0))
		{
			maildir_aclt_destroy(&chkacls);
			errno=EINVAL;
			return -1;
		}
		if (check_adminrights(&chkacls))
		{
			*err_failedrights=owner;
			errno=EINVAL;
			return -1;
		}
	}

	if (maildir_acl_compute(&chkacls, aclt_list, is_admin, NULL))
	{
		maildir_aclt_destroy(&chkacls);
		errno=EINVAL;
		return -1;
	}
	if (check_allrights(&chkacls))
	{
		errno=EINVAL;
		return -1;
	}

	p=malloc(strlen(maildir)+strlen(path)+2);

	if (!p)
		return -1;

	strcat(strcat(strcpy(p, maildir), "/"), path);

	maildir_tmpcreate_init(&tci);

	tci.maildir=p;
	tci.uniq="acl";
	tci.doordie=1;

	fp=maildir_tmpcreate_fp(&tci);

	trycreate=0;

	if (fp)
	{
		q=malloc(strlen(p) + sizeof("/" ACLFILE));
		if (!q)
		{
			fclose(fp);
			unlink(tci.tmpname);
			maildir_tmpcreate_free(&tci);
			free(p);
			return -1;
		}
		strcat(strcpy(q, p), "/" ACLFILE);
		free(tci.newname);
		tci.newname=q;
		free(p);
	}
	else
	{
		free(p);

		q=malloc(strlen(maildir)+sizeof("/" ACLHIERDIR "/") +
			 strlen(path));
		if (!q)
		{
			maildir_tmpcreate_free(&tci);
			return -1;
		}
		strcat(strcat(strcpy(q, maildir), "/" ACLHIERDIR "/"), path+1);

		tci.maildir=maildir;
		tci.uniq="acl";
		tci.doordie=1;

		fp=maildir_tmpcreate_fp(&tci);

		if (!fp)
		{
			free(q);
			maildir_tmpcreate_free(&tci);
			return -1;
		}
		free(tci.newname);
		tci.newname=q;
		trycreate=1;
	}

	if (maildir_aclt_list_enum(aclt_list, save_acl, fp) < 0 ||
	    ferror(fp) || fflush(fp) < 0)
	{
		fclose(fp);
		unlink(tci.tmpname);
		maildir_tmpcreate_free(&tci);
		return -1;
	}
	fclose(fp);

	if (rename(tci.tmpname, tci.newname) < 0)
	{
		/* Perhaps ACLHIERDIR needs to be created? */

		if (!trycreate)
		{
			unlink(tci.tmpname);
			maildir_tmpcreate_free(&tci);
			return -1;
		}

		p=strrchr(tci.newname, '/');
		*p=0;
		mkdir(tci.newname, 0755);
		*p='/';

		if (rename(tci.tmpname, tci.newname) < 0)
		{
			unlink(tci.tmpname);
			maildir_tmpcreate_free(&tci);
			return -1;
		}
	}
	maildir_tmpcreate_free(&tci);
	return 0;
}

static int save_acl(const char *identifier, const maildir_aclt *acl,
		    void *cb_arg)
{
	if (fprintf((FILE *)cb_arg, "%s %s\n",
		    identifier,
		    maildir_aclt_ascstr(acl)) < 0)
		return -1;
	return 0;
}

struct maildir_acl_resetList {
	struct maildir_acl_resetList *next;
	char *mbox;
};

/*
** When a maildir is opened check for stale entries in Maildir/ACLHIERDIR.
**
** Maildir/ACLHIERDIR/folder.subfolder should be removed unless there exists
** Maildir/.folder.subfolder.subsubfolder
**
**
** acl_check_cb is the callback function for maildir_list, which receives
** INBOX.folder.subfolder.subsubfolder.  It goes through the link list with
** Maildir/ACLHIERDIR's contents, and removes folder.subfolder if its found.
**
** After maildir_list is done, anything that's left in the list can be safely
** removed.
*/

static void acl_check_cb(const char *mbox, void *voidarg)
{
	struct maildir_acl_resetList **l=
		(struct maildir_acl_resetList **)voidarg;

	if (strncmp(mbox, INBOX ".", sizeof(INBOX ".")-1))
		return; /* Huh? */

	mbox += sizeof(INBOX ".")-1;

	while (*l)
	{
		int cl= strlen( (*l)->mbox );

		if (strncmp(mbox, (*l)->mbox, cl) == 0 &&
		    mbox[cl] == '.')
		{
			struct maildir_acl_resetList *p= *l;

			*l= p->next;
			free(p->mbox);
			free(p);
			continue;
		}

		l= &(*l)->next;
	}
}

int maildir_acl_reset(const char *maildir)
{
	DIR *dirp;
	struct dirent *de;
	char *p;
	struct maildir_acl_resetList *rl=NULL;
	struct maildir_acl_resetList *r;
	time_t now;
	struct stat stat_buf;

	p=malloc(strlen(maildir) + sizeof("/" ACLHIERDIR));
	if (!p)
		return -1;

	strcat(strcpy(p, maildir), "/" ACLHIERDIR);

	dirp=opendir(p);

	if (!dirp)
	{
		mkdir(p, 0755);
		dirp=opendir(p);
	}
	free(p);

	while (dirp && (de=readdir(dirp)) != NULL)
	{
		if (de->d_name[0] == '.')
			continue;

		if ((r=malloc(sizeof(struct maildir_acl_resetList))) == NULL
		    || (r->mbox=strdup(de->d_name)) == NULL)
		{
			if (r)
				free(r);

			while (rl)
			{
				r=rl;
				rl=r->next;
				free(r->mbox);
				free(r);
			}
			closedir(dirp);
			return -1;
		}

		r->next=rl;
		rl=r;
	}
	if (dirp) closedir(dirp);

	maildir_list(maildir, acl_check_cb, &rl);

	time(&now);

	while (rl)
	{
		r=rl;
		rl=r->next;

		p=malloc(strlen(maildir)+strlen(r->mbox) +
			 sizeof("/" ACLHIERDIR "/"));
		if (p)
		{
			strcat(strcat(strcpy(p, maildir),
				      "/" ACLHIERDIR "/"), r->mbox);

			/* Only unlink stale entries after 1 hour (race) */

			if (stat(p, &stat_buf) == 0 &&
			    stat_buf.st_mtime < now - 60*60)
				unlink(p);
			free(p);
		}
		free(r->mbox);
		free(r);
	}
	return 0;
}

/*
** An ACL entry for "administrators" or "group=administrators" will match
** either one.
*/

static int chk_admin(int (*cb_func)(const char *isme,
				    void *void_arg),
		     const char *identifier,
		     void *void_arg)
{
	if (strcmp(identifier, "administrators") == 0 ||
	    strcmp(identifier, "group=administrators") == 0)
	{
		int rc=(*cb_func)("administrators", void_arg);

		if (rc == 0)
			rc=(*cb_func)("group=administrators", void_arg);
		return rc;
	}

	return (*cb_func)(identifier, void_arg);
}

#define ISIDENT(s) \
	(MAILDIR_ACL_ANYONE(s) ? 1: chk_admin(cb_func, (s), void_arg))

static int maildir_acl_compute_chkowner(maildir_aclt *aclt,
					maildir_aclt_list *aclt_list,
					int (*cb_func)(const char *isme,
						       void *void_arg),
					void *void_arg,
					int chkowner)
{
	struct maildir_aclt_node *p;
	int rc;

	if (maildir_aclt_init(aclt, "", NULL) < 0)
		return -1;

	for (p=aclt_list->head; p; p=p->next)
	{
		if (p->identifier[0] == '-')
			continue;

		rc= ISIDENT(p->identifier);

		if (rc < 0)
		{
			maildir_aclt_destroy(aclt);
			return rc;
		}

		if (rc == 0)
			continue;

		if (maildir_aclt_add(aclt, NULL, &p->acl) < 0)
		{
			maildir_aclt_destroy(aclt);
			return rc;
		}
	}

	for (p=aclt_list->head; p; p=p->next)
	{
		if (p->identifier[0] != '-')
			continue;

		rc= ISIDENT(p->identifier+1);

		if (rc < 0)
		{
			maildir_aclt_destroy(aclt);
			return rc;
		}

		if (rc == 0)
			continue;

		if (maildir_aclt_del(aclt, NULL, &p->acl) < 0)
		{
			maildir_aclt_destroy(aclt);
			return rc;
		}
	}

	/*
	** In our scheme, the owner always gets admin rights.
	*/

	rc=chkowner ? (*cb_func)("owner", void_arg):0;

	if (maildir_acl_disabled)
		rc=0;	/* Except when ACLs are disabled */

	if (rc < 0)
	{
		maildir_aclt_destroy(aclt);
		return rc;
	}

	if (rc)
	{
		if (maildir_aclt_add(aclt, ACL_ADMINISTER, NULL) < 0)
		{
			maildir_aclt_destroy(aclt);
			return rc;
		}
	}
	return 0;
}

int maildir_acl_compute(maildir_aclt *aclt, maildir_aclt_list *aclt_list,
			int (*cb_func)(const char *isme,
				       void *void_arg), void *void_arg)
{
	return maildir_acl_compute_chkowner(aclt, aclt_list, cb_func, void_arg,
					    1);
}

static int chk_array(const char *identifier, void *void_arg);

int maildir_acl_compute_array(maildir_aclt *aclt,
			      maildir_aclt_list *aclt_list,
			      const char * const *identifiers)
{
	return maildir_acl_compute(aclt, aclt_list, chk_array,
				   (void *)identifiers);
}

static int chk_array(const char *identifier, void *void_arg)
{
	const char * const *p=(const char * const *)void_arg;
	size_t i;

	for (i=0; p[i]; i++)
		if (strcmp(identifier, p[i]) == 0)
			return 1;
	return 0;
}

/* -------------------------------------------------------------------- */

int maildir_acl_canlistrights(const char *myrights)
{
	return (strchr(myrights, ACL_LOOKUP[0]) ||
		strchr(myrights, ACL_READ[0]) ||
		strchr(myrights, ACL_INSERT[0]) ||
		strchr(myrights, ACL_CREATE[0]) ||
		strchr(myrights, ACL_DELETEFOLDER[0]) ||
		strchr(myrights, ACL_EXPUNGE[0]) ||
		strchr(myrights, ACL_ADMINISTER[0]));
}

/* --------------------------------------------------------------------- */

/*
** Compute owner ACL identifiers applicable to a mailbox that's owned by
** 'mailbox_owner'.
*/

static int get_owner_list( int (*cb_func)(const char *, void *),
			   const char *c,
			   const char *mailbox_owner, void *arg)
{
	char *a;
	int rc;
	const char *p, *q;

	a=malloc(sizeof("user=")+strlen(c));
	if (!a)
		return -1;

	strcat(strcpy(a, "user="), c);

	rc=(*cb_func)(a, arg);

	if (rc == 0 && strcmp(a, mailbox_owner) == 0)
		rc=(*cb_func)("owner", arg);
	free(a);

	if (rc)
		return rc;

	c=getenv("OPTIONS");

	for (p=c; p && *p; )
	{
		if (*p == ',')
		{
			++p;
			continue;
		}

		q=p;
		while (*p && *p != ',')
			++p;

		if (strncmp(q, "group=", 6) == 0)
		{
			a=malloc(p-q+1);
			if (!a)
				return -1;

			memcpy(a, q, p-q);
			a[p-q]=0;
			rc=(*cb_func)(a, arg);
			free(a);
			if (rc)
				return rc;
		}
	}
	return 0;
}

static int count_owner_list(const char *o, void *arg)
{
	++*(size_t *)arg;

	return 0;
}

static int save_owner_list(const char *o, void *arg)
{
	char ***p=(char ***)arg;

	**p=strdup(o);

	if (**p == NULL)
		return -1;

	++*p;
	return 0;
}

int maildir_acl_computerights(maildir_aclt *rights,
			      maildir_aclt_list *acl_list,
			      const char *me,
			      const char *folder_owner)
{
	char **owner_array;
	size_t owner_cnt;
	char **p;
	int rc;

	owner_cnt=1;

	if (get_owner_list(count_owner_list, me, folder_owner,
			   (void *)&owner_cnt) ||
	    (owner_array=calloc(owner_cnt, sizeof(char *))) == NULL)
		return -1;

	p=owner_array;

	if (get_owner_list(save_owner_list, me, folder_owner, (void *)&p))
	{
		for (owner_cnt=0; owner_array[owner_cnt]; ++owner_cnt)
			free(owner_array[owner_cnt]);
		free(owner_array);
		return -1;
	}

	rc=maildir_acl_compute_array(rights, acl_list,
				     (const char * const *)owner_array);

	for (owner_cnt=0; owner_array[owner_cnt]; ++owner_cnt)
		free(owner_array[owner_cnt]);
	free(owner_array);
	return rc;
}
