/*
 * $Log: set_environment.c,v $
 * Revision 1.10  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2023-05-31 12:06:05+05:30  Cprogrammer
 * skip local env variable processing of .defaultqueue if SKIP_LOCAL_ENVIRONMENT is set
 *
 * Revision 1.8  2023-01-03 17:01:54+05:30  Cprogrammer
 * removed auto_sysconfdir.h dependency
 *
 * Revision 1.7  2021-07-14 13:10:55+05:30  Cprogrammer
 * change for change in envdir function
 *
 * Revision 1.6  2021-07-13 23:15:03+05:30  Cprogrammer
 * minor refactoring to make code smaller
 *
 * Revision 1.5  2021-07-05 21:18:24+05:30  Cprogrammer
 * new argument root_rc to allow root to load $HOME/.defaultqueue
 *
 * Revision 1.4  2021-07-04 23:58:27+05:30  Cprogrammer
 * skip .defaultqueue if running as root
 *
 * Revision 1.3  2021-06-12 19:56:17+05:30  Cprogrammer
 * removed calls to chdir to avoid messing cwd
 *
 * Revision 1.2  2021-05-26 10:46:50+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.1  2021-05-13 12:37:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <env.h>
#include <envdir.h>
#include <error.h>
#include <strerr.h>
#include <pathexec.h>
#include <stralloc.h>
#include "auto_control.h"
#include "variables.h"

static stralloc tmp = { 0 };

void
set_environment(const char *warn, const char *fatal, int root_rc)
{
	const char     *qbase, *home, *err;
	char           **e;
	int             i, unreadable = 0;

	/*-
	 * allow $HOME/.defaultqueue for non-root
	 * or root with non-zero root_rc
	 * root_rc is non-zero for qmail-inject, new-inject,
	 * qmail-qread, queue-fix, maildirserial
	 */
	if (!env_get("SKIP_LOCAL_ENVIRONMENT") && (home = env_get("HOME")) && (getuid() || root_rc)) {
		if (!stralloc_copys(&tmp, home) ||
				!stralloc_catb(&tmp, "/.defaultqueue", 14) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, fatal, "out of memory");
		if (!access(tmp.s, X_OK)) {
			if ((i = envdir(tmp.s, &err, 0, &unreadable))) {
				strerr_warn5(warn, envdir_str(i), ": ", err, ": ", &strerr_sys);
				pathexec_clear();
			} else
			if ((e = pathexec(0)))
				environ = e;
		}
	}
	if (!(qbase = env_get("QUEUE_BASE"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&tmp, controldir) ||
				!stralloc_catb(&tmp, "/defaultqueue", 13) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, fatal, "out of memory");
		if (!access(tmp.s, X_OK)) {
			if ((i = envdir(tmp.s, &err, 1, &unreadable)))
				strerr_die5sys(111, fatal, envdir_str(i), ": ", err, ": ");
			if ((e = pathexec(0)))
				environ = e;
		} else
		if (errno != error_noent)
			strerr_die2sys(111, fatal, "unable to access defaultqueue: ");
	}
	return;
}

void
getversion_set_environment_c()
{
	const char     *x = "$Id: set_environment.c,v 1.10 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
