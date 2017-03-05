#ifndef	maildiraclt_h
#define	maildiraclt_h

#ifdef  __cplusplus
extern "C" {
#endif


/*
** Copyright 2003-2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif


/*
** A basic ACL entity.  Be generic, it's just a character string.
** However, we do keep it in collating order.
**
** These functions return 0 on success, <0 on error.
*/

typedef char *maildir_aclt;


/*
** Initialize an aclt.  The second or third args specify its initial value.
** Both may be NULL.  Only one can be non-NULL.
*/

int maildir_aclt_init(maildir_aclt *aclt,
		      const char *initvalue_cstr,
		      const maildir_aclt *initvalue_cpy);

/* Destroy an aclt after it is no longer used. */

void maildir_aclt_destroy(maildir_aclt *aclt);


/* Add or remove access chars. */

int maildir_aclt_add(maildir_aclt *aclt,
		      const char *add_strs,
		      const maildir_aclt *add_aclt);

int maildir_aclt_del(maildir_aclt *aclt,
		     const char *del_strs,
		     const maildir_aclt *del_aclt);

/* return a const char * that contains the acl */

#define maildir_aclt_ascstr(t) (*(t) ? (const char *)*(t):"")


/* Next level up, a list of <identifier,acl>s */

struct maildir_aclt_node {
	struct maildir_aclt_node *prev;
	struct maildir_aclt_node *next;
	char *identifier;
	maildir_aclt acl;
};

typedef struct {
	struct maildir_aclt_node *head;
	struct maildir_aclt_node *tail;
} maildir_aclt_list;


/* Initialize and destroy the list */

void maildir_aclt_list_init(maildir_aclt_list *aclt_list);
void maildir_aclt_list_destroy(maildir_aclt_list *aclt_list);

/* Add an <identifier,acl> pair.  Returns 0 on success, -1 on failure */

int maildir_aclt_list_add(maildir_aclt_list *aclt_list,
			  const char *identifier,
			  const char *aclt_str,
			  maildir_aclt *aclt_cpy);

/* Remove an identifier */

int maildir_aclt_list_del(maildir_aclt_list *aclt_list,
			  const char *identifier);

/*
** Enumerate the ACL list.  The callback function, cb_func, gets
** invoked for each ACL list entry.  The callback function receives:
** identifier+rights pair; as well as the transparent pass-through
** argument.  A nonzero return from the callback function terminates
** the enumeration, and maildir_aclt_list_enum itself returns
** non-zero.  A zero return continues the enumeration.  After the
** entire list is enumerated maildir_aclt_list_enum returns 0.
*/

int maildir_aclt_list_enum(maildir_aclt_list *aclt_list,
			   int (*cb_func)(const char *identifier,
					  const maildir_aclt *acl,
					  void *cb_arg),
			   void *cb_arg);

/* Find an identifier */
 
const maildir_aclt *maildir_aclt_list_find(maildir_aclt_list *aclt_list,
					   const char *identifier);

/* maildir-level acl ops */

#define ACL_LOOKUP "l"
#define ACL_READ "r"
#define ACL_SEEN "s"
#define ACL_WRITE "w"
#define ACL_INSERT "i"
#define ACL_POST "p"
#define ACL_CREATE "c"
#define ACL_DELETEFOLDER "x"
#define ACL_DELETEMSGS "t"
#define ACL_EXPUNGE "e"
#define ACL_ADMINISTER "a"

#define ACL_ALL \
	ACL_ADMINISTER \
	ACL_CREATE \
	ACL_EXPUNGE \
	ACL_INSERT \
	ACL_LOOKUP \
	ACL_READ \
	ACL_SEEN \
	ACL_DELETEMSGS \
	ACL_WRITE \
	ACL_DELETEFOLDER

#define ACL_DELETE_SPECIAL "d"

#define ACLFILE "courierimapacl"
#define ACLHIERDIR "courierimaphieracl"


#define MAILDIR_ACL_ANYONE(s) \
	(strcmp( (s), "anonymous") == 0 || \
	 strcmp( (s), "anyone") == 0)


/*
** Set maildir_acl_disabled to 1 to effectively disable ACL support, and its
** overhead.
**
** If maildir_acl_disabled is set, maildir_acl_read never goes to disk to
** read the ACL file, instead it returns a fixed ACL list which only contains
** an entry for "owner", and gives "owner" all ACL rights, except the
** ADMINISTER right, relying on higher level code to refuse to set new
** ACLs unless the existing ACL gives administer right.
**
** Additionally, maildir_acl_disabled turns off the hook in maildir_acl_compute
** that grants ADMINISTER to "owner" irrespective of what the ACLs actually
** say.
*/

extern int maildir_acl_disabled;

/*
** Read ACLs for maildir maildir.path.
**
** maildir: Path to the main maildir.
**
** path: ".folder.subfolder".
**
** aclt_list is an uninitialized maildir_aclt_list
**
** Returns 0 for success, <0 for failure.
*/

int maildir_acl_read(maildir_aclt_list *aclt_list,
		     const char *maildir,
		     const char *path);

/*
** Write ACLs for maildir maildir.path.
**
** Returns 0 for success, <0 for failure.
**
** Additional parameters:
**
** owner: the owner entity of the folder represented by 'path'.
**
** err_failedrights: if not NULL, *err_failedrights will be initialized to
** a non-null identifier string if maildir_acl_set fails because aclt_list
** illegally revokes minimum rights from the identifier (admin/lookup).
**
*/

int maildir_acl_write(maildir_aclt_list *aclt_list,
		      const char *maildir,
		      const char *path,
		      const char *owner,
		      const char **err_failedrights);

/* Remove stale ACL entries */

int maildir_acl_reset(const char *maildir);

/* Remove a particular ACL entry */

int maildir_acl_delete(const char *maildir,
		       const char *path);   /* .folder.subfolder */

/*
** Compute my access rights.  Initializes 'aclt'. 'aclt_list' is the ACL.
**
** The callback function should return >0 if identifier refers to the entity
** whose access rights are to be computed; 0 if it does not, <0 if an error
** occured.
**
** As a special case, maildir_acl_compute() handles "anonymous" and "anyone"
** identifiers on its own.
**
** As a special case, if the callback function returns >0 for the identifier
** "owner", the computed access rights will always include the ADMIN right.
**
** maildir_aclt_compute() uses ACL2=UNION; the computed access rights
** consist of the union of all rights granted to all identifiers that include
** the entity, minus the union of all reights revoked from all identifiers
** that include the entity.
*/
int maildir_acl_compute(maildir_aclt *aclt, maildir_aclt_list *aclt_list,
			int (*cb_func)(const char *identifier,
				       void *void_arg), void *void_arg);

/*
** A wrapper for maildir_acl_compute that compares against a
** const char * array.
*/

int maildir_acl_compute_array(maildir_aclt *aclt,
			      maildir_aclt_list *aclt_list,
			      const char * const *identifiers);

/*
** A wrapper for maildir_acl_compute.
**
** Compute 'rights' - my rights on the mailbox.
**
** acl_list: the mailbox's ACL.
**
** me: my login identifier.
**
** folder_owner: the owner of the mailbox folder whose rights are computed
**
** OTHER: The "OPTIONS" environment variable is parsed to obtain a list of
** account groups 'me' belongs to.
**
** Returns 0 upon success, after placing the computed access rights in
** 'rights'.
*/

int maildir_acl_computerights(maildir_aclt *rights,
			      maildir_aclt_list *acl_list,
			      const char *me,
			      const char *folder_owner);

/*
** Convenience functions:
**
** maildir_acl_canlistrights: return true if the given rights indicate that
** the rights themselves can be viewed (one of the following must be present:
** ACL_LOOKUP, ACL_READ, ACL_INSERT[0], ACL_CREATE[0], ACL_DELETEFOLDER,
** ACL_EXPUNGE[0], or ACL_ADMINISTER).
*/

int maildir_acl_canlistrights(const char *myrights);

#ifdef  __cplusplus
}
#endif

#endif
