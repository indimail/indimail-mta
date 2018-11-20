#ifndef	waitlib_h
#define	waitlib_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif


#include <sys/types.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

/*
** Ok, wait() pecularities are handled by the following functions.  Except
** for them, nobody else should care about wait's quirks.
**
** First, call signal(SIGCHLD) as usual to set up your signal handler.
** Within the signal handler, call wait_reap to reap one (or more)
** child processes.
*/

void wait_reap( void (*)(pid_t, int), /* Called to process reaped child */
	RETSIGTYPE (*)(int));	/* Should point back to signal handler */

/*
** Main program can call wait_block and wait_clear to temporarily
** suspend reaping while in a critical section.
*/

void wait_block();
void wait_clear(RETSIGTYPE (*)(int));	/* The signal handler */

/*
** wait_restore should be called instead of signal(SIGCHLD, SIG_DFL)
** to restore the signal handler just before exiting.  It should also
** be called by any forked process.
*/

void wait_restore();

/*
** Sometimes the parent wants to wait for one child to terminate.
** call wait_forchild for that.  First, wait_block() must be called to
** suspend all asynchronous reaping.  Then, call wait_forchild.  Before
** wait_forchild returns, the reaper function is guaranteed to be called.
** Asynchronous reaping is still blocked upon exit, call wait_clear() to
** reenable it.
*/

void wait_forchild( void (*)(pid_t, int), /* Reaper */
	RETSIGTYPE (*)(int));	/* Signal handler stub */

/*
** wait_startchildren() is a convenient function to start a given number
** of child processes.  The function returns 0 for the parent process, >0
** for each child process, and < 0 if there was an error starting child
** processes.  pidptr points to a pointer to the array of started pids,
** which wait_startchildren initializes, if *pidptr is NULL, wait_startchildren
** mallocs this array.  If pidptr is NULL, wait_startchildren uses its own
** internal array.
*/

int wait_startchildren(unsigned nchildren, pid_t **pidptr);

/*
** wait_reforkchild() is used in conjunction with wait_startchildren's array,
** and is intended to be called from the wait handler.  It checks if the
** pid is one of the listed children, and, if so, reforks it.
** wait_reforkchild() returns < 0 if there was a problem reforking the child
** process, 0 if the child process started succesfully, or if the terminated
** pid is unknown, > 0 in the reforked process.
*/

int wait_reforkchild(unsigned nchildren, pid_t *pidptr, pid_t pid);



#endif
