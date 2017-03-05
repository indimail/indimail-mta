#ifndef	maildir_maildirinfo_h
#define	maildir_maildirinfo_h

#include "config.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
** Copyright 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/


struct maildir_info {
	int mailbox_type;
	char *homedir;
	char *maildir;
	char *owner;
};

void maildir_info_destroy(struct maildir_info *); /* Deallocate memory */

int maildir_info_imap_find(struct maildir_info *info, const char *path,
			   const char *myid);

/*
** Initialize info based on path.  Returns 0 for success, -1 if path is
** syntactically invalid.  The mailbox may not actually exist.
**
** 'myid' is my login id, used to initialize owner (see below) for INBOX
** folders.
**
** homedir is set to the mailbox's homedir, which may not necessarily be
** "." if path points to #shared.user.folder.
** maildir is the local mailbox path, such as INBOX.folder
**
** owner will be set to the mailbox's owner, for ACL purposes.
**
** maildir will be NULL for a node in the legacy shared hierarchy.
*/

#define MAILBOXTYPE_INBOX 0	/* Inbox maildir */
#define MAILBOXTYPE_OLDSHARED 1	/* Legacy shared hierarchy */
#define MAILBOXTYPE_NEWSHARED 2 /* #shared hierarchy */

#define MAILBOXTYPE_IGNORE  255 /* Ignore this mailbox */

/*
** The application must define the following callback function that returns
** non-zero if the filename refers to the current account's maildir, and
** should be suppressed from the shared folder hierarchy.
*/
extern int maildir_info_suppress(const char *maildir);

/*
** The SMAP version:
*/
int maildir_info_smap_find(struct maildir_info *info, char **folder,
			   const char *myid);

char **maildir_smapfn_fromutf7(const char *modutf7);
void maildir_smapfn_free(char **fn);

/*
** The shared index files use UTF-8.  Convenience function to convert
** names into IMAP-compatible modified-UTF7.
*/

extern void maildir_info_munge_complex(int);
	/* If true, use "complex" munging */

extern char *maildir_info_imapmunge(const char *name);

#ifdef  __cplusplus
}
#endif

#endif
