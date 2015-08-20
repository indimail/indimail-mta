
#ifndef liblockmail_h
#define liblockmail_h

/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifdef	__cplusplus
extern "C" {
#endif

	/*
	** Functions for mbox mail locks
	*/

struct ll_mail {
	char *file;		/* File being locked */

	/* c-client type lock */

	int cclientfd;
	char *cclientfile;


	/* dotlock */

	char *dotlock;
};

struct ll_mail *ll_mail_alloc(const char *filename);

/*
** Create a c-client type lock.  NOTE: c-clients will ping you with SIGUSR2,
** which must be ignored for this implementation.
** Returns: 0 - ok, < 0 - error.
**
** An error return from ll_mail_lock carries some additional context in
** errno:
**
** errno == EAGAIN: potential race condition.  The current lock holder MIGHT
** have just terminated.  The caller should sleep for AT LEAST 5 seconds, then
** try again.
**
** errno == EEXIST: another process on this server DEFINITELY has the lock.
**
** Implementations might choose to wait and try again on EEXIST as well.
*/

int ll_mail_lock(struct ll_mail *);

/*
** Open the mail file, read/write (creating a dot-lock).
** Returns: >= 0 - file descriptor, < 0 - error (if EPERM, try ll_open_ro).
**
** errno == EEXIST: another process appears to hold a dot-lock.
**
** errno == EAGAIN: We just blew away a stale dotlock, should try again
** in at least five seconds.  Should NOT get two EAGAIN's in a row.
*/

int ll_mail_open(struct ll_mail *);

/*
** Open in read-only mode.
*/

int ll_mail_open_ro(struct ll_mail *);

/*
** Release all locks, deallocate structure.  NOTE: file descriptor from
** ll_mail_open(_ro)? is NOT closed, it's your responsibility to do that.
*/

void ll_mail_free(struct ll_mail *);

/*
** As long as we have the logic done already, here's a generic dot-locking
** function.
**
** dotlock - the actual filename of a dotlock file.
** tmpfile - the filename of a temporary file to create first.
** timeout - optional timeout.
**
** Return code: 0: dotlock is created.  Just unlink(dotlock) when you're done.
**
** -1 error.  Check errno:
**
**    EEXIST - dotlock is locked
**
**    EAGAIN - dotlock is stale (dotlock created on this machine, and the
**             process no longer exists, or dotlock created on another
**             machine, and timeout argument was > 0, and the dotlock's
**             timestamp was older than timeout seconds.
**
**    E????? - something's broken.
**           
*/

int ll_dotlock(const char *dotlock, const char *tmpfile,
	       int timeout);

#ifdef	__cplusplus
}
#endif

#endif
