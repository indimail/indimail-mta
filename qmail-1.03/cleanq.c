/*
 * $Log: cleanq.c,v $
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
#include "substdio.h"
#include "error.h"
#include "strerr.h"
#include "direntry.h"
#include "auto_ageout.h"
#include "auto_uids.h"
#include "wait.h"
#include "str.h"
#include <time.h>

#define FATAL "cleanq: fatal: "
#define WARNING "cleanq: warning: "

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF(write, 1, buf1, sizeof(buf1));
static int      flaglog = 0;

void
die_usage(void)
{
	strerr_die1x(100, "cleanq: usage: cleanq [-l]");
}

void
my_puts(s)	/*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
	char           *s;
{
	if (substdio_puts(&ss1, s) == -1)
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
	if (substdio_flush(&ss1) == -1)
		_exit(111);
}

void
warn_file(const char *d, const char *w)
{
	log_file(WARNING, d, w);
}

void
direrror(void)
{
	strerr_warn2(FATAL, "unable to read directory: ", &strerr_sys);
}

void
remove(const char *d)
{
	DIR            *dir = 0;
	struct dirent  *f = 0;
	int             err = 0;

	/*- Change directories and open */
	if (chdir(d) == -1)
	{
		strerr_warn4(WARNING, "unable to chdir to ", (char *) d, ": ", &strerr_sys);
		return;
	}
	if (chdir("work") == -1)
	{
		/*- Warn admin */
		strerr_warn4(WARNING, "unable to chdir to ", (char *) d, "/work: ", &strerr_sys);
		chdir(".."); /*- get out!  */
		rmdir(d); /*- no harm in trying */
		return;
	}
	dir = opendir(".");
	if (!dir)
	{
		direrror();
		goto cdup;
	}

	/*- Scan it */
	for (;;)
	{
		errno = 0;
		f = readdir(dir);
		err = errno;
		if (!f)
			break;
		if (!str_diff(f->d_name, ".") || !str_diff(f->d_name, ".."))
			continue;
		if (unlink(f->d_name) == -1)
			strerr_warn4(WARNING, "unable to delete ", f->d_name, ": ", &strerr_sys);
	}
	if (err)
	{
		direrror();
		strerr_warn4(WARNING, "unable to read directory ", (char *) d, ": ", &strerr_sys);
	}
	closedir(dir);
cdup:
	if (chdir("..") == -1)
		strerr_die1x(100, "Can't chdir to \"..\"; don't know where I am!");
	if (rmdir("work") == -1)
		strerr_warn2(WARNING, "unable to remove directory \"work\": ", &strerr_sys);
	if (chdir("..") == -1)
		strerr_die1x(100, "Can't chdir to \"..\"; don't know where I am!");
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
	if (stat(d, &st) == -1)
	{
		strerr_warn4(WARNING, "unable to stat ", (char *) d, ": ", &strerr_sys);
		return;
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR)
	{
		warn_file(d, "not a directory. Deleting...");
		unlink(d); /*- If it fails, oh well.  */
		return;
	}
	/*- 3 */
	if (d[0] != '@')
	{
		warn_file(d, "name doesn't start with '@'");
		return;
	}
	/*- 4 */
	if (!(st.st_mode & S_ISVTX)) /*- not sticky */
	{
		log_file("deleting: ", d, "not sticky");
		remove(d);
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
	remove(d);
}

int
main(int argc, char **argv)
{
	DIR            *dir;
	direntry       *d;
	int             opt;
	uid_t           uid;

	if (uidinit(0) == -1)
		_exit(67);
	while ((opt = getopt(argc, argv, "l")) != -1)
	{
		switch (opt)
		{
		case 'l':
			flaglog = 1;
			break;
		default:
			die_usage();
		}
	}
	if (flaglog)
	{
		my_puts("cleanq starting\n");
		if (substdio_flush(&ss1) == -1)
			_exit(111);
	}
	uid = getuid();
	if (uid != auto_uidc && setreuid(auto_uidc, auto_uidc))
	{
		if (flaglog)
			strerr_die2sys(111, FATAL, "setreuid failed: ");
		_exit(111);
	}
	/*- Open the current directory */
	dir = opendir(".");
	if (!dir)
	{
		direrror();
		_exit(1);
	}
	/*- Scan it */
	for (;;)
	{
		errno = 0;
		d = readdir(dir);
		if (!d)
			break;
		tryclean(d->d_name);
	}
	if (errno)
	{
		direrror();
		closedir(dir);
		_exit(1);
	}
	closedir(dir);
	_exit(0);
}

void
getversion_cleanq_c()
{
	static char    *x = "$Id: cleanq.c,v 1.6 2014-07-27 15:16:18+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
