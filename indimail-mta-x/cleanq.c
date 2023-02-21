/*
 * $Log: cleanq.c,v $
 * Revision 1.13  2023-02-14 07:45:58+05:30  Cprogrammer
 * renamed auto_uidc, auto_gidc to auto_uidv, auto_gidv
 *
 * Revision 1.12  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.11  2021-06-27 10:35:07+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.10  2021-06-13 00:32:05+05:30  Cprogrammer
 * do clean exit on shutdown
 *
 * Revision 1.9  2019-06-07 11:25:48+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.8  2017-05-04 20:19:42+05:30  Cprogrammer
 * make cleanq run continuously
 *
 * Revision 1.7  2017-05-03 09:33:07+05:30  Cprogrammer
 * refactored code. Added safety check to ensure cleanq starts in dir owned by qscand
 *
 * Revision 1.6  2014-07-27 15:16:18+05:30  Cprogrammer
 * change to qscand uid if run through non qscand user
 *
 * Revision 1.5  2014-01-29 13:59:37+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2013-08-27 08:19:10+05:30  Cprogrammer
 * use sticky bit definition from header file
 *
 * Revision 1.3  2004-10-22 20:23:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-11 13:51:00+05:30  Cprogrammer
 * use getopt instead of sgetopt
 *
 * Revision 1.1  2004-09-22 22:24:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <substdio.h>
#include <subfd.h>
#include <sgetopt.h>
#include <error.h>
#include <strerr.h>
#include <direntry.h>
#include <wait.h>
#include <str.h>
#include <scan.h>
#include <sig.h>
#include <noreturn.h>
#include "auto_ageout.h"
#include "auto_uids.h"

#define FATAL "cleanq: fatal: "
#define WARNING "cleanq: warning: "

static int      flaglog = 0, dir_flag = 0;

no_return void
die_usage(void)
{
	strerr_die1x(100, "cleanq: usage: cleanq [-l] [-s interval] [directory]");
}

void
my_puts(char *s)	/*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
{
	if (substdio_puts(subfdout, s) == -1)
		_exit(111);
}

void
log_file(const char *h, const char *d, const char *m)
{
	if (!flaglog)
		return;
	my_puts((char *) h);
	my_puts((char *) d);
	my_puts(": ");
	my_puts((char *) m);
	my_puts("\n");
	if (substdio_flush(subfdout) == -1)
		_exit(111);
}

void
warn_file(const char *d, const char *w)
{
	log_file(WARNING, d, w);
}

void
remove_files(const char *d)
{
	DIR            *dir = 0;
	struct dirent  *f = 0;

	/*- Change directories and open */
	if (chdir(d) == -1) {
		strerr_warn4(WARNING, "unable to chdir to ", (char *) d, ": ", &strerr_sys);
		return;
	}
	dir_flag = 1;
	if (chdir("work") == -1) {
		/*- Warn admin */
		strerr_warn4(WARNING, "unable to chdir to ", (char *) d, "/work: ", &strerr_sys);
		if (!chdir("..")) /*- get out!  */
			dir_flag = 0;
		if (rmdir(d)) /*- no harm in trying */
			strerr_warn4(WARNING, "unable to remove directory ", (char *) d, ": ", &strerr_sys);
		return;
	}
	if (!(dir = opendir("."))) {
		strerr_warn2(FATAL, "unable to read directory 'work': ", &strerr_sys);
		goto cdup;
	}

	/*- Scan it */
	for (;;) {
		if (!(f = readdir(dir)))
			strerr_warn4(WARNING, "unable to read directory ", (char *) d, "/work: ", &strerr_sys);
		if (!f)
			break;
		if (!str_diff(f->d_name, ".") || !str_diff(f->d_name, ".."))
			continue;
		if (unlink(f->d_name) == -1)
			strerr_warn6(WARNING, "unable to delete ", (char *) d, "/", f->d_name, ": ", &strerr_sys);
	}
	closedir(dir);
cdup:
	if (chdir("..") == -1)
		strerr_die1x(100, "Can't chdir to \"..\"; don't know where I am!");
	if (rmdir("work") == -1)
		strerr_warn2(WARNING, "unable to remove directory \"work\": ", &strerr_sys);
	if (chdir("..") == -1)
		strerr_die1x(100, "Can't chdir to \"..\"; don't know where I am!");
	dir_flag = 0;
	if (rmdir(d) == -1)
		strerr_warn4(WARNING, "unable to remove directory ", (char *) d, ": ", &strerr_sys);
}

void
tryclean(const char *d)
{
	struct stat     st;
	time_t          now;

	/*-
	 * 1. Ignore "." and ".."
	 * 2. Try unlinking non-directories. These should never exist.
	 * 3. Ignore files not beginning with "@"
	 * 4. Delete if not sticky.
	 * 5. Skip if it's not old enough.
	 */

	/*- 1 */
	if (!str_diff((char *) d, ".") || !str_diff((char *) d, ".."))
		return;
	/*- 2 */
	if (stat(d, &st) == -1) {
		strerr_warn4(WARNING, "unable to stat ", (char *) d, ": ", &strerr_sys);
		return;
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR) {
		warn_file(d, "not a directory. Deleting...");
		if (unlink(d)) /*- If it fails, oh well.  */
			strerr_warn4(WARNING, "unlink: ", (char *) d, ": ", &strerr_sys);
		return;
	}
	/*- 3 */
	if (d[0] != '@') {
		warn_file(d, "name doesn't start with '@'");
		return;
	}
	/*- 4 */
	if (!(st.st_mode & S_ISVTX)) { /*- not sticky */
		log_file("deleting: ", d, "not sticky");
		remove_files(d);
		return;
	}
	/*- 5 */
	now = time(NULL);
	if ((now - st.st_ctime) < 3 * MAX_AGE) /*- not old */
		return;
	/*
	 * Delete it. Optionally, log the action.
	 */
	log_file("deleting: ", d, "too old");
	remove_files(d);
}

no_return void
sigterm()
{
	if (flaglog)
		strerr_warn2(WARNING, "going down on SIGTERM", 0);
	_exit(0);
}

int
main(int argc, char **argv)
{
	DIR            *dir;
	direntry       *d;
	int             opt, fdsourcedir = -1;
	unsigned int    interval = 300;
	uid_t           uid;
	struct stat     st;

	sig_termcatch(sigterm);
	if (uidinit(1, 1) == -1)
		strerr_die2sys(111, FATAL, "uidinit: ");
	while ((opt = getopt(argc, argv, "ls:")) != opteof) {
		switch (opt)
		{
		case 'l':
			flaglog = 1;
			break;
		case 's':
			scan_uint(optarg, &interval);
			break;
		default:
			die_usage();
		}
	}
	if (optind + 1 == argc && chdir(argv[optind++]) == -1)
		strerr_die3sys(111, FATAL, "chdir: ", argv[optind]);
	uid = getuid();
	if (uid != auto_uidv && setreuid(auto_uidv, auto_uidv))
		strerr_die2sys(111, FATAL, "setreuid failed: ");
	if ((fdsourcedir = open(".", O_RDONLY | O_NDELAY)) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	for (;;) {
		if (flaglog == 1) {
			flaglog++;
			my_puts("cleanq starting\n");
		} else
		if (flaglog)
			my_puts("cleanq scanning\n");
		if (substdio_flush(subfdout) == -1)
			_exit(111);
		if (stat(".", &st) == -1)
			strerr_die2sys(111, FATAL, "unable to stat '.'");
		if (st.st_uid != auto_uidv) {
			strerr_warn2(FATAL, "current directory not owned by qscand", 0);
			_exit(111);
		}
		/*- Open the current directory */
		if (!(dir = opendir("."))) {
			strerr_warn2(FATAL, "unable to read directory '.': ", &strerr_sys);
			_exit(1);
		}
		/*- Scan it */
		for (;;) {
			if (!(d = readdir(dir)))
				break;
			tryclean(d->d_name);
		}
		closedir(dir);
		sleep(interval);
		if (dir_flag && fchdir(fdsourcedir) == -1)
			strerr_die2sys(111, FATAL, "unable to switch back to original directory: ");
		else
			dir_flag = 0;
	}
	_exit(errno ? 111 : 0);
}

void
getversion_cleanq_c()
{
	static char    *x = "$Id: cleanq.c,v 1.13 2023-02-14 07:45:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
