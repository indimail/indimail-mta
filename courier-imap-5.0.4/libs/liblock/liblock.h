
#ifndef liblock_h
#define liblock_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifdef	__cplusplus
extern "C" {
#endif

#if	HAVE_CONFIG_H
#include	"liblock/config.h"
#endif
#include	<sys/types.h>

#define	ll_whence_start	0
#define	ll_whence_curpos 1
#define	ll_whence_end 2

#define	ll_readlock	0
#define	ll_writelock	4
#define	ll_unlock	8
#define	ll_wait		16

int	ll_lockfd(int,	/* File descriptor */
		int,	/* ll_ bitmask */
		LL_OFFSET_TYPE,	/* Start */
		LL_OFFSET_TYPE);	/* Length */


/* Some useful macros: ll_lock_ex - exclusive lock on a file,
		ll_lock_ex_test - attempt an exclusive lock on a file
		ll_unlock_ex - unlock a file
*/

#define	ll_lock_ex(f)	\
	ll_lockfd( (f), ll_writelock|ll_whence_start|ll_wait, 0, 0)

#define	ll_lock_ex_test(f)	\
	ll_lockfd( (f), ll_writelock|ll_whence_start, 0, 0)

#define	ll_unlock_ex(f)	\
	ll_lockfd( (f), ll_unlock|ll_whence_start, 0, 0)


/*
** Value-added: functions that reliably start and stop a daemon process,
** permitting only one daemon process running.  Utilizes a lock file, and a
** pidfile.
*/

int ll_daemon_start(const char *lockfile);
void ll_daemon_started(const char *pidfile, int fd);
int ll_daemon_resetio();
int ll_daemon_stop(const char *lockfile, const char *pidfile);
int ll_daemon_restart(const char *lockfile, const char *pidfile);

/*
  The basic scenario

main()
{
    if ((fd=ll_daemon_start(lockfilename)) < 0)
    {
           error();  exit(1);
    }

    ... Some custom initialization here ...

    ll_daemon_started(pidfile, fd);

    ll_daemon_resetio();   ... this one is optional
}

To stop this daemon:

ll_daemon_stop (lockfilename, pidfile)


ll_daemon_start attempts to start a daemon process going.  It does only
a partial setup.  If it detects that the daemon process is already
running, it itself does an exit(0), not returning to the parent.

If there was a failure starting a daemon process, -1 is return, else
we return a transparent file descriptor, which will have to be passed as
the secodn argument to ll_daemon_started().

When ll_daemon_start returns, we're already running in a partially set-up
daemon process.  The setup isn't complete just yet.  The parent function
can perform any other custom initialization.  If initialization fails,
the parent function can simply exit.  Otherwise, if the initialization
completes, ll_daemon_started must be called in order to save this daemon
process's pid in the pid file (2nd arg must be the return from ll_daemon_start.

To stop a daemon process, simply call ll_daemon_stop.  Nothing too
sophisticated here.

To send the daemon process a SIGHUP, call ll_daemon_restart.
*/

#ifdef	__cplusplus
}
#endif

#endif
