#ifndef	imapscanclient_h
#define	imapscanclient_h

#include "config.h"
#include "maildir/maildirkeywords.h"

/*
** Copyright 1998 - 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
** Stuff we want to know about an individual message in the maildir.
*/

struct imapscanmessageinfo {
	unsigned long uid;	/* See RFC 2060 */
	char *filename;
	struct libmail_kwMessage *keywordMsg; /* If not NULL - keywords */
	char recentflag;
	char changedflags;	/* Set by imapscan_open */
	char copiedflag;	/* This message was copied to another folder */

	char storeflag;  /* Used by imap_addRemoveKeywords() */

	/* When reading keywords, hash messages by filename */

	struct imapscanmessageinfo *firstBucket, *nextBucket;

	} ;

/*
** Stuff we want to know about the maildir.
*/

struct imapscaninfo {
	unsigned long nmessages;	/* # of messages */
	unsigned long uidv;		/* See RFC 2060 */
	unsigned long left_unseen;
	unsigned long nextuid;

	struct libmail_kwHashtable *keywordList; /* All defined keywords */

	struct imapscanmessageinfo *msgs;
	struct maildirwatch *watcher;
	} ;

/*
** In imapscan_maildir, move the following msgs to cur.
*/

struct uidplus_info {
	struct uidplus_info *next;
	char *tmpfilename;
	char *curfilename;

	char *tmpkeywords;
	char *newkeywords;

	unsigned long uid; /* Initialized by imapscan_maildir2 */
	unsigned long old_uid; /* Initialized by do_copy() */
	
	time_t mtime;
} ;


void imapscan_init(struct imapscaninfo *p);
void imapscan_copy(struct imapscaninfo *a,
		   struct imapscaninfo *b);

int imapscan_maildir(struct imapscaninfo *, const char *, int, int,
		     struct uidplus_info *);
void imapscan_free(struct imapscaninfo *);

int imapscan_openfile(const char *, struct imapscaninfo *, unsigned);


struct libmail_kwMessage *imapscan_createKeyword(struct imapscaninfo *,
					      unsigned long n);

int imapscan_updateKeywords(const char *filename,
			    struct libmail_kwMessage *newKeywords);

int imapscan_restoreKeywordSnapshot(FILE *, struct imapscaninfo *);
int imapscan_saveKeywordSnapshot(FILE *fp, struct imapscaninfo *);

int imapmaildirlock(struct imapscaninfo *scaninfo,
		    const char *maildir,
		    int (*func)(void *),
		    void *void_arg);

char *readline(unsigned, FILE *);

#endif
