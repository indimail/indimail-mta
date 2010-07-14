/** \file db_lock.c
 * \brief Lock handler to detect application crashes
 * \author Matthias Andree
 * \date 2004
 *
 * GNU GPL v2
 * with optimization ideas by Pavel Kankovsky
 *
 * \attention
 * This code uses signal handlers and must pay extra attention to
 * reentrancy!
 *
 * \par Lock file layout:
 * the lock file has a list of cells, which can be either 0 or 1.
 * - 0 means: slot is free
 * - 1 means: slot in use
 *   - 1 with fcntl lock: process running
 *   - 1 without lock: process quit prematurely, recovery needed
 *
 * \sa http://article.gmane.org/gmane.mail.bogofilter.devel/3240\n
 *     http://article.gmane.org/gmane.mail.bogofilter.devel/3260\n
 *     http://article.gmane.org/gmane.mail.bogofilter.devel/3270
 */

#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "db_lock.h"
#include "debug.h"
#include "error.h"
#include "globals.h"
#include "mxcat.h"
#include "system.h"
#include "xmalloc.h"
#include "xstrdup.h"

/** figure out which flag we pass to open() to enforce synchronous data
 * I/O completion */
#if HAVE_DECL_O_DSYNC
static const int syncflag = O_DSYNC;
#elif HAVE_DECL_O_SYNC
static const int syncflag = O_SYNC;
#elif HAVE_DECL_O_FSYNC
static const int syncflag = O_FSYNC;
#endif

/** Type we use for a lock cell. */
typedef char bf_cell_t;

/** Periodic check interval in seconds, for set_lock(). */
static const int chk_intval = 30;
/** String to append to base directory, for process table file. */
static const char aprt[] = DIRSEP_S "lockfile-p";
/** Size of a cell, must match sizeof(bf_cell_t). */
static const off_t cellsize = 1;
/** Offset of our lock cell inside the lock file. */
static off_t lockpos;
/** Boolean marker to remember if we hold the lock. */
static int locked;
/** File descriptor of the open lock file, or -1 when not open.
 * The lock file descriptor must be long-lived because fcntl() or
 * lockf() will immediately release all locks held on a file once we
 * call close() for the file for the first time no matter how many file
 * descriptors to the file we still hold, so we cannot use
 * open-check-close cycles but need to keep the
 * descriptor open until we'll let go of the locks.
 */
static int lockfd = -1;

/** Constant cell content for cells that are in use. */
static const bf_cell_t cell_inuse = '1';
/** Constant cell content for cells that are \b not in use. */
static const bf_cell_t cell_free = '0';

/** Save area for previous SIGALRM signal handler. */
static struct sigaction oldact;

/** Convert the fcntl lock type to a string.
 * \return constant string to resolve the numeric \a locktype. */
static const char *s_locktype(int locktype) {
    /* reentrant */
    switch (locktype) {
	case F_UNLCK: return "F_UNLCK";
	case F_RDLCK: return "F_RDLCK";
	case F_WRLCK: return "F_WRLCK";
	default:      return "UNKNOWN";
    }
}

/** Checks if the cell in file \a fd at given \a offset is locked.
 * \return 1 if locked, 0 if unlocked, negative if error */
/* part of the signal handler */
static int check_celllock(int fd, off_t offset) {
    /* reentrant */
    struct flock fl;
    int r;

    /* check our own lock explicitly first, F_GETLK will return F_UNLCK
     * for our own locks because we can demote, upgrade, reset, set our
     * own locks at will, IOW, F_GETLK will only detect locks set by
     * other processes */
    if (offset == lockpos && locked)
	return 1;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = cellsize;
    r = fcntl(fd, F_GETLK, &fl);
    if (r) {
	/* cannot use fprintf for debugging here - isn't reentrant! */
	return -1;
    }
    /* cannot use fprintf for debugging here - isn't reentrant! */
    return fl.l_type == F_UNLCK ? 0 : 1;
}

/** Fast lock function, uses F_SETLK so does not wait. Not reentrant. */
static int set_celllock(int fd, off_t offset, int locktype) {
    struct flock fl;
    int r;

    fl.l_type = locktype;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = cellsize;
    r = fcntl(fd, F_SETLK, &fl);
    if (DEBUG_DATABASE(2))
	fprintf(dbgout, "set_celllock(fd=%d, offset=%ld, type=%d (%s)) = %d%s%s\n",
		fd, (long)offset, locktype, s_locktype(locktype),
		r, r < 0 ? ", " : "", r < 0 ? strerror(errno) : "");
    return r;
}

/** initialize the lock file which is presumed to exist and have the
 * file name \a fn, and which will be deleted in case of trouble.
 */
static int init_lockfile(const char *fn) {
    char b[1024];	/* XXX FIXME: make lock size configurable */
    int rc = 0;

    memset(b, (unsigned char)cell_free, sizeof(b)); /* XXX FIXME: only works for char */
    if (lseek(lockfd, (off_t)0, SEEK_SET) != (off_t)0)
	rc = -1;

    if (rc
	    || sizeof(b) != write(lockfd, b, sizeof(b))
	    || fsync(lockfd))
    {
	close(lockfd);
	lockfd = -1;
	if (fn)
	    unlink(fn);
	return -1;
    }
    return 0;
}

/** Create a lock file with name \a fn and open modes \a modes to which
 * O_CREAT and O_EXCL are or'd. Will retry when a race was detected.
 * \return descriptor of open lock file for success,
 * -1 for error
 */
static int create_lockfile(const char *fn, int modes) {
    char *tmp = NULL;
    int count=1;

    do {
	char buf[50];
	snprintf(buf, sizeof(buf), ".%ld.%d", (long)getpid(), count++);
	if (tmp) free(tmp);
	tmp = mxcat(fn, buf, NULL);
	lockfd = open(tmp, modes|O_CREAT|O_EXCL, DS_MODE); /* umask will decide about group writability */
    } while (lockfd < 0 && errno == EEXIST);

    if (lockfd >= 0) {
	if (init_lockfile(tmp)) {
	    free(tmp);
	    return -1;
	}
	if (link(tmp, fn)) {
	    int e = errno;
	    close(lockfd);
	    lockfd = -1;
	    unlink(tmp);
	    free(tmp);
	    errno = e;
	    return -1;
	}
	unlink(tmp);
    }
    return lockfd;
}

/** Open and possibly create the lock file.
 * This will do nothing and flag success if the file is open already
 * \return 0 for success, -1 for error.
 */
static int open_lockfile(const char *bogohomedir) {
    char *fn;
    int modes = O_RDWR|syncflag;

    if (lockfd >= 0) return 0;
    fn = mxcat(bogohomedir, aprt, NULL);

    do {
	lockfd = open(fn, modes);
	if (lockfd < 0 && errno == ENOENT)
	    lockfd = create_lockfile(fn, modes);
    } while (lockfd < 0 && errno == EEXIST);

    if (lockfd < 0) {
	print_error(__FILE__, __LINE__, "open_lockfile: open(%s): %s",
		fn, strerror(errno));
    } else {
	if (DEBUG_DATABASE(1)) {
	    fprintf(dbgout, "open_lockfile: open(%s) succeeded, fd #%d\n", fn, lockfd);
	}
    }

    xfree(fn);
    return (lockfd < 0) ? -1 : 0;
}

/** Close the lock file, releasing all locks.
 * \return is propagated from the underlying close() function. */
static int close_lockfile(void) {
    int r = 0;

    if (lockfd >= 0) {
	if (DEBUG_DATABASE(1))
	    fprintf(dbgout, "close_lockfile\n");
	r = close(lockfd);
	lockfd = -1;
	if (r) {
	    int e = errno;
	    print_error(__FILE__, __LINE__, "close_lockfile: close(%d) failed: %s",
		    lockfd, strerror(errno));
	    errno = e;
	}
    }
    return r;
}

/** This function checks if any processes have previously crashed.
 * \return
 * - 0 - no zombies
 * - 1 - zombies
 * - 2 - file not found
 * - -1 - other error
 */
/* part of the signal handler */
/* reentrant */
static int check_zombies(void) {
    ssize_t r;
    off_t pos, savepos;
    bf_cell_t cell;

    savepos = lseek(lockfd, 0, SEEK_CUR);
    if (savepos < 0)
	return -1;

    if (lseek(lockfd, 0, SEEK_SET) < 0)
	return -1;

    for (;;) {
	pos = lseek(lockfd, 0, SEEK_CUR);
	r = read(lockfd, &cell, sizeof(cell));
	if (r != sizeof(cell)) break;
	if (cell == cell_inuse && 1 != check_celllock(lockfd, pos)) {
	    if (lseek(lockfd, savepos, SEEK_SET) != savepos)
		return -1;
	    return 1;
	}
    }

    if (lseek(lockfd, savepos, SEEK_SET) != savepos)
	return -1;

    return r == 0 ? 0 : -1;
}

/** Signal handler, checks if processes have crashed and if so,
 * writes an error message to STDERR_FILENO and calls _exit(). */
static void check_lock(int unused) {
    (void)unused;

    if (0 != check_zombies()) {
	const char *text = "bogofilter or related application has crashed or directory damaged, aborting.\n";
	(void)write(STDERR_FILENO, text, strlen(text));
	_exit(EX_ERROR);	/* use _exit, not exit, to avoid running the atexit handler that might deadlock */
    }
    alarm(chk_intval);
}

/** Initialize signal handler and start the timer. \return 0 for
 * success, -1 for error */
static int init_sig(void) {
#ifdef	SA_RESTART
    struct sigaction sa;
    sigset_t ss;

    sigemptyset(&ss);

    sa.sa_handler = check_lock;
    sa.sa_mask = ss;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa, &oldact)) return -1;

    alarm(chk_intval);
#endif
    return 0;
}

/** Shut down the timer and restore the previous SIGALRM handler.
 * \return is propagated from sigaction(). */
static int shut_sig(void) {
    alarm(0);
    return sigaction(SIGALRM, &oldact, NULL);
}

int set_lock(void) {
    bf_cell_t cell;
    ssize_t r;

    if (lseek(lockfd, 0, SEEK_SET) < 0)
	return -1;

    for (;;) {
	lockpos = lseek(lockfd, 0, SEEK_CUR);
	r = read(lockfd, &cell, sizeof(cell));
	if (r != sizeof(cell))
	    return -1; /* XXX FIXME: retry? */
	if (cell == cell_free && 0 == set_celllock(lockfd, lockpos, F_WRLCK)) {
	    if (lseek(lockfd, lockpos, SEEK_SET) >= 0) {
		r = read(lockfd, &cell, sizeof(cell));
		if (r != sizeof(cell) || cell != cell_free) {
		    /* found fresh zombie */
		    set_celllock(lockfd, lockpos, F_UNLCK);
		    return -2;
		}
		if (lseek(lockfd, lockpos, SEEK_SET) < 0)
		    return -1;
		if (cellsize != write(lockfd, &cell_inuse, cellsize))
		    return -1;
#if 0
/* disabled for now, O_{D,,F}SYNC should handle this for us */
#ifdef HAVE_FDATASYNC
		if (fdatasync(lockfd))
		    return -1;
#else
		if (fsync(lockfd))
		    return -1;
#endif
#endif

		init_sig();
		locked = 1;
		return 0;
	    }
	}
    }
}

int clear_lock(void) {
    shut_sig();
    if (lseek(lockfd, lockpos, SEEK_SET) < 0)
	return -1;
    locked = 0;
    if (cellsize != write(lockfd, &cell_free, cellsize))
	return -1;
    if (set_celllock(lockfd, lockpos, F_UNLCK))
	return -1;
    if (close_lockfile())
	return -1;
    return 0;
}

int init_dbl(const char *bogodir) {
    return open_lockfile(bogodir) ? -1 : 0;
}

int needs_recovery(void) {
    return 0 != check_zombies();
}

int clear_lockfile(void) {
    if (init_lockfile(NULL))
	return -1;
    return 0;
}
