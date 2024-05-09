/*
 * $Log: qmail-popbull.c,v $
 * Revision 1.11  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.10  2021-05-26 10:44:34+05:30  Cprogrammer
 * replaced strerror() with error_str()
 *
 * Revision 1.9  2020-09-16 19:05:06+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.8  2020-06-08 22:51:47+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.7  2004-10-22 20:28:37+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-07-17 21:21:02+05:30  Cprogrammer
 * added RCS log
 *
 *
 * written by Russell Nelson for USWest.
 * ported to 1.03 by Bruce Guenter
 * multiple bulletins bug noticed and fixed by Brian Mullen
 * cur timestamp can't be used, noticed by DJB, fixed by Brett Rabe.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <direntry.h>
#include <substdio.h>
#include <stralloc.h>
#include <subfd.h>
#include <open.h>
#include <fmt.h>
#include <error.h>
#include <datetime.h>
#include <now.h>
#include <str.h>
#include <noreturn.h>

static char     fntmptph[80 + FMT_ULONG * 2];

no_return void
die()
{
	_exit(100);
}

no_return void
die_temp()
{
	_exit(111);
}

no_return void
die_usage()
{
	substdio_putsflush(subfderr, "qmail-popbull: usage: qmail-popbull bulldir pop3d maildir\n");
	die_temp();
}

no_return void
die_nobulldir()
{
	substdio_putsflush(subfderr, "qmail-popbull: fatal: unable to read bulldir\n");
	die_temp();
}

no_return void
die_nomaildir()
{
	substdio_putsflush(subfderr, "qmail-popbull: fatal: unable to write to maildir\n");
	die_temp();
}

no_return void
die_nocdmaildir()
{
	substdio_putsflush(subfderr, "qmail-popbull: fatal: unable to change to maildir\n");
	die_temp();
}

no_return void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-popbull: fatal: out of memory\n");
	die_temp();
}

void
fnmake_maildir()
{
	unsigned long   pid;
	unsigned long   time;
	char            host[64];
	char           *s;
	int             loop;
	struct stat     st;

	pid = getpid();
	host[0] = 0;
	gethostname(host, sizeof(host));
	for (loop = 0;; ++loop) {
		time = now();
		s = fntmptph;
		s += fmt_str(s, "new/");
		s += fmt_ulong(s, time);
		*s++ = '.';
		s += fmt_ulong(s, pid);
		*s++ = '.';
		s += fmt_strn(s, host, sizeof(host));
		*s++ = 0;
		if (stat(fntmptph, &st) == -1)
			if (errno == error_noent)
				break;
		/*
		 * really should never get to this point
		 */
		if (loop == 2)
			_exit(1);
		sleep(2);
	}
}

int
main(int argc, char **argv)
{
	int             fd;
	struct stat     st;
	datetime_sec    ts_date;
	char           *bulldirname, *programname, *maildirname;
	char          **childargs;
	DIR            *bulldir;
	direntry       *d;
	stralloc        errorstr = { 0 }, fn = { 0 };

	if (!(bulldirname = argv[1]))
		die_usage();
	if (!(programname = argv[2]))
		die_usage();
	if (!(maildirname = argv[3]))
		die_usage();

	if (chdir(maildirname) == -1)
		die_nocdmaildir();
	argv[3] = (char *) ".";
	if (stat(".timestamp", &st) == -1)
		ts_date = 0;
	else
		ts_date = st.st_mtime;
	fd = open_trunc(".timestamp");
	close(fd);
	bulldir = opendir(bulldirname);
	if (!bulldir)
		die_nobulldir();
	while ((d = readdir(bulldir))) {
		if (str_equal(d->d_name, "."))
			continue;
		if (str_equal(d->d_name, ".."))
			continue;
		if (!stralloc_copys(&fn, bulldirname))
			die_nomem();
		if (!stralloc_cats(&fn, "/"))
			die_nomem();
		if (!stralloc_cats(&fn, d->d_name))
			die_nomem();
		if (!stralloc_0(&fn))
			die_nomem();
		if (stat(fn.s, &st) == -1)
			die();
		if ((st.st_mode & 0222) == 0)
			continue;
		if (st.st_mtime > ts_date) {
			fnmake_maildir();
			if (symlink(fn.s, fntmptph) == -1)
				;
		}
	}
	closedir(bulldir);
	childargs = argv + 2;
	execvp(*childargs, childargs);
	if (!stralloc_copys(&errorstr, "qmail-popbull: execvp: "))
		die_nomem();
	if (!stralloc_cats(&errorstr, error_str(errno)))
		die_nomem();
	if (!stralloc_0(&errorstr))
		die_nomem();
	substdio_putsflush(subfderr, errorstr.s);
	_exit(1);		
	/*- Not reached */
	return(1);
}

void
getversion_qmail_popbull_c()
{
	const char     *x = "$Id: qmail-popbull.c,v 1.11 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
