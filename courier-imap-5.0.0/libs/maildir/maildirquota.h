#ifndef	maildirquota_h
#define	maildirquota_h

/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>

#include	"numlib/numlib.h"

#ifdef  __cplusplus
extern "C" {
#endif


#define	MDQUOTA_SIZE	'S'	/* Total size of all messages in maildir */
#define	MDQUOTA_BLOCKS	'B'	/* Total # of blocks for all messages in
				maildir -- NOT IMPLEMENTED */
#define	MDQUOTA_COUNT	'C'	/* Total number of messages in maildir */

struct maildirquota {
	int64_t nbytes;	/* # of bytes, 0 - unlimited */
	int nmessages;	/* # of messages, 0 - unlimited */
};

	/*
	** The maildirsize file
	*/

struct maildirsize {

	int fd;	/* Opened file descriptor for the maildirsize file */

	char *maildir;			/* Pathname to the maildir */
	char *maildirsizefile;		/* The name of the maildirsize file */

	struct maildirquota quota;	/* 1st line in maildirsize */

	struct maildirquota size;	/* Actual counts 2+ line */

	int recalculation_needed;	/* size is not calculated */

	struct stat	statbuf;	/* The stat on the maidlirsize file */
	unsigned nlines;	/* # of lines in the maildirsize file */
};


/*
** maildir_openquotafile initializes a maildirsize structure from a maildirsize
** file.  This is really an internal, undocumented, function.
**
** return 0 for success, -1 if the file could not be opened
*/

int maildir_openquotafile(struct maildirsize *info,	/* Initialized */
			  const char *			/* maildir */
			  );

/*
** maildir_closequotafile releases all resources allocated by maildirsize
** struct.
*/

void maildir_closequotafile(struct maildirsize *info);




int maildir_checkquota(struct maildirsize *, /* Opened maildir */
		       int64_t,  /* Extra bytes planning to add/remove from
				 maildir */
		       int);  /* Extra messages planning to add/remove from
				 maildir */

int maildir_addquota(struct maildirsize *, /* Opened maildir */
		     int64_t,	/* +/- bytes */
		     int);	/* +/- files */

int maildir_readquota(struct maildirsize *);	/* Directory */

int maildir_parsequota(const char *, unsigned long *);
	/* Attempt to parse file size encoded in filename.  Returns 0 if
	** parsed, non-zero if we didn't parse. */


	/* Here are some high-level functions that call the above */

	/* Adding messages to the maildir, in two easy steps: */

int maildir_quota_add_start(const char *maildir,
			    struct maildirsize *info,
			    int64_t msgsize, int nmsgs,
			    const char *newquota);

void maildir_quota_add_end(struct maildirsize *info,
			   int64_t msgsize, int nmsgs);

/* When we're deleting messages, we want an unconditional quota update */

void maildir_quota_deleted(const char *maildir,
			   int64_t nbytes,	/* Must be negative */
			   int nmsgs);	/* Must be negative */


/* Can we delete or undelete messages?  This is like maildir_quota_add_start
** and maildir_quota_add_end, except that if deleted messages are included in
** the quota, they are compiled to no-ops
*/

int maildir_quota_delundel_start(const char *maildir,
				 struct maildirsize *info,
				 int64_t msgsize, int nmsgs);

void maildir_quota_delundel_end(struct maildirsize *info,
				int64_t msgsize, int nmsgs);

	/* Set a new quota on the maildir; */

void maildir_quota_set(const char *dir, const char *quota);

	/* Forcibly recalculate the maildir's quota */

void maildir_quota_recalculate(const char *maildir);


	/*
	** Should the following folder/file be included in the quota?
	** (excludes TRASH and deleted files, if configured to do so)
	*/

int maildirquota_countfolder(const char *folder);
int maildirquota_countfile(const char *filename);

void maildir_deliver_quota_warning(const char *dir, const int percent,
				   const char *msgquotafile);

#ifdef  __cplusplus
}
#endif

#endif
