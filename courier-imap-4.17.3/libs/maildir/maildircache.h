/*
** Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/



/*

The login cache is used to try to eliminate a call to getpw for each and
every http request, which can be quite expensive on systems with large number
of users and heave web traffic.  The following information is saved in the
cache:

userid
groupid
directory (presumably of a maildir)
certain environment variables


The interface is abstracted into these functions:

maildir_cache_init(seconds, cachedir, cacheowner, authvars)

maildir_cache_start()
maildir_cache_save(userid, login_time, homedir, uid, gid)
maildir_cache_cancel()

maildir_cache_search(userid, login_time, callback_func, callback_func_arg)

The prepare, save, and cancel functions are used to cache the login
information.
maildir_cache_start should be called before we attempt to log in, when we're
running as root.  We're about to drop root privileges after a successful
login, but we need to be root in order to update the cache directory, so
maildir_cache_start forks a child process, which will wait patiently in
the background.

maildir_cache_save will sends the cacheable information to the child process,
over a secured pipe.  maildir_cache_save will be called after a successful
login.

maildir_cache_cancel shall be called if the login failed.  It will kill the
child process.

The search function is called to query the cache file.  If it succeeds, it
calls the callback function with the userid, groupid, and homedir.

There is no need to manually remove an expired cache entry upon logout.
It will be cleaned up by a separate cron job.


init_login_cache should be called before any other function.  It's argument
specifies: that hard timeout interval - the fixed amount of time after which
any login becomes invalid; the login cache directory; userid that owns the
login cache directory; the environment variables to save in the login cache
directory.

The login cache functions receive the saved original login time.  The login
cache information is saved in a directory that should be writable by cache
owner only.  The cache directory contains subdirectories whose name is derived
by dividing the login time by the hard timeout interval.  For example, when
logging on in the afternoon of November 27, 1999, the current time, in seconds,
is 943725152.  With the login interval being the default of 2 hours, 7200
seconds, the top level directory would be 943725152 / 7200 or 131072.

What this allows us to do is to quickly remove expired login entries, simply
by reading the top level cache directory, and recursively deleting subdirs
whose names are too old to contain any logins that are still active.

If the login name is 'john', the cached login will be saved in the file
131072/jo/john, creating the subdirectories if necessary.

Because the login name can contain special characters, the special characters
will be escaped.  See the code for more info.

*/

#ifndef	logincache_h
#define	logincache_h

#include	<sys/types.h>
#include	<time.h>
#include	<pwd.h>


extern int maildir_cache_init(time_t, const char *, const char *,
			      const char * const *);
extern void maildir_cache_start(void);
extern void maildir_cache_save(const char *, time_t, const char *, uid_t,
			       gid_t);
extern void maildir_cache_cancel(void);

extern int maildir_cache_search(const char *, time_t,
				int (*)(uid_t, gid_t, const char *, void *),
				void *);

extern void maildir_cache_purge();

#endif
