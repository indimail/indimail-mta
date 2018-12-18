#ifndef maildirwatch_h
#define maildirwatch_h
/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/


#ifdef  __cplusplus
extern "C" {
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif

/*
** These function leverage libfam.a to watch for maildir changes.
**
** If libfam.a is not available, these functions are compiled to no-ops
*/

#if HAVE_FAM
#include <fam.h>
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

#if HAVE_FAM
struct maildirwatch_fam {
	FAMConnection fc;
	int broken;
	unsigned refcnt;
};

#endif

struct maildirwatch {
	char *maildir;

#if HAVE_FAM
	struct maildirwatch_fam *fam;
#endif
	time_t now;
	time_t timeout;

};

#define WATCHDOTLOCK	"tmp/courier.lock"

#define KEYWORDDIR "courierimapkeywords"

struct maildirwatch *maildirwatch_alloc(const char *maildir);

void maildirwatch_free(struct maildirwatch *w);
	/*
	** Wait for WATCHDOTLOCK to go away
	*/

void maildirwatch_cleanup();
	/* Final cleanup before prog terminates */

int maildirwatch_unlock(struct maildirwatch *w, int nseconds);

	/*********** Wait for changes to new and cur subdirs ************/

	/* Caller must allocate the follownig structure: */

struct maildirwatch_contents {
	struct maildirwatch *w;

#if HAVE_FAM
	FAMRequest new_req;
	FAMRequest cur_req;
	FAMRequest courierimapkeywords_req;

	unsigned short endexists_received;
	unsigned short ack_received;

	unsigned short cancelled;

#endif

};

/*
** maildirwatch_start() initiates the process of monitoring the maildir.
** The monitoring process does not get started right away, since FAM needs
** to acknowledge th monitoring requests first.
**
** Returns: 0 - monitoring request sent.
**          1 - FAM is not available, will fall back to 60 second polls.
**         -1 - Fatal error.
*/

int maildirwatch_start(struct maildirwatch *p,
		       struct maildirwatch_contents *w);

/*
** Check if FAM started monitoring yet.
**
** Returns: 1 - Monitoring has started, or we're in fallback mode.
**          0 - Not yet, *fdret is initialized to file descriptor to wait on.
**         -1 - A fatal error occured, fall back to polling mode.
**
** maildirwatch_started() returns right away, without blocking.
*/

int maildirwatch_started(struct maildirwatch_contents *w,
			 int *fdret);

/*
** Check if maildir's contents have changed.
**
** Returns: 0 - Monitoring in progress.  *changed set to non-zero if maildir
**              was changed.
**         -1 - Fatal error.
**
** *fdret and *timeout get initialized to the file descriptor to wait on,
** and the requested timeout.  *fdret may be negative in polling mode.
*/

int maildirwatch_check(struct maildirwatch_contents *w,
		       int *changed,
		       int *fdret,
		       int *timeout);

	/*
	** Clean everything up.
	*/
void maildirwatch_end(struct maildirwatch_contents *w);


	/*
	** Courier-IMAP compatible maildir lock.
	**
	** Returns a non-NULL filename on success.  To unlock:
	**
	** unlink(filename); free(filename);
	**
	** A NULL return with tryAnyway != 0 means that the lock failed
	** probably as a result of misconfigured FAM, or something.
	**
	*/
char *maildir_lock(const char *maildir,
		   struct maildirwatch *w, /* If NULL, we sleep() */
		   int *tryAnyway);

#ifdef  __cplusplus
}
#endif

#endif
