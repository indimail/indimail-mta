/*
 * $Id: maildirsize.c,v 1.14 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <pwd.h>
#include <unistd.h>
#include <inttypes.h>
#ifdef sun
#include <sys/types.h>
#include <sys/statvfs.h>
#elif defined(DARWIN) || defined(FREEBSD)
#include <sys/param.h>
#include <sys/mount.h>
#elif defined(linux)
#include <sys/vfs.h>
#endif
#include <sys/stat.h>
#include <sgetopt.h>
#include <subfd.h>
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#include <open.h>
#include "qcount_dir.h"

#define FATAL "maildirsize: fatal: "

struct substdio ssout;
char            ssoutbuf[256];
stralloc        maildirsizefn = {0};

void
update_maildirsize(char *maildirsize_fn, ssize_t avail, ssize_t mailsize, size_t mailcount, uid_t uid, gid_t gid)
{
	int             fd, len;
	char            strnum[FMT_ULONG];

	if ((fd = open_trunc(maildirsize_fn)) == -1)
		strerr_die3sys(111, FATAL, maildirsize_fn, ": ");
	if (fchown(fd, uid, gid))
		strerr_die4sys(111, FATAL, "chown: ", maildirsize_fn, ": ");
	else
	if (fchmod(fd, 0644))
		strerr_die4sys(111, FATAL, "chmod: ", maildirsize_fn, ": ");
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, ssoutbuf, sizeof(ssoutbuf));
	strnum[len = fmt_ulong(strnum, avail)] = 0;
	if (substdio_put(&ssout, strnum, len) || substdio_put(subfdout, strnum, len))
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_put(&ssout, "S\n", 2) || substdio_put(subfdout, "S\n", 2))
		strerr_die2sys(111, FATAL, "write: ");
	strnum[len = fmt_ulong(strnum, mailsize)] = 0;
	if (substdio_put(&ssout, strnum, len) || substdio_put(subfdout, strnum, len))
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_put(&ssout, " ", 1) || substdio_put(subfdout, " ", 1))
		strerr_die2sys(111, FATAL, "write: ");
	strnum[len = fmt_ulong(strnum, mailcount)] = 0;
	if (substdio_put(&ssout, strnum, len) || substdio_put(subfdout, strnum, len))
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_put(&ssout, "\n", 1) || substdio_put(subfdout, "\n", 1))
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(&ssout))
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(subfdout))
		strerr_die2sys(111, FATAL, "write: ");
	if (close(fd) == -1)
		strerr_die2sys(111, FATAL, "close: ");
	return;
}

int
main(int argc, char **argv)
{
	int             opt;
	size_t          mailcount;
	int64_t         mailsize;
	char           *user = (char *) 0;
	struct statfs   statbuf;
	struct passwd  *pw;

	while ((opt = getopt(argc, argv, "u:d:")) != opteof) {
		switch (opt) {
		case 'u':
			user = optarg;
			break;
		default:
			strerr_die1x(100, "usage: maildirsize -u user [dir]");
		}
	}
	if (!user)
		strerr_die1x(100, "usage: maildirsize -u user [dir]");
	if (optind + 1 == argc) {
		if (chdir(argv[optind]))
			strerr_die4sys(111, FATAL, "chdir: ", argv[optind], ": ");
		if (!stralloc_copys(&maildirsizefn, argv[optind]) ||
				!stralloc_catb(&maildirsizefn, "/maildirsize", 12) ||
				!stralloc_0(&maildirsizefn))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!(pw = getpwnam(user)))
			strerr_die3x(111, FATAL, "unknown account ", user);
		mailcount = 0;
		mailsize = qcount_dir(argv[optind], &mailcount);
		if (statfs(argv[optind], &statbuf))
			strerr_die4sys(111, FATAL, "statfs: ", argv[optind], ": ");
	} else {
		if (!(pw = getpwnam(user)))
			strerr_die3x(111, FATAL, "unknown account ", user);
		if (chdir(pw->pw_dir))
			strerr_die4sys(111, FATAL, "chdir: ", pw->pw_dir, ": ");
		if (access("./Maildir/", F_OK))
			strerr_die4sys(111, FATAL, ": ", pw->pw_dir, "/Maildir/: ");
		if (!stralloc_copys(&maildirsizefn, pw->pw_dir) ||
				!stralloc_catb(&maildirsizefn, "/Maildir/maildirsize", 20) ||
				!stralloc_0(&maildirsizefn))
			strerr_die2sys(111, FATAL, "out of memory: ");
		mailcount = 0;
		mailsize = qcount_dir("./Maildir/", &mailcount);
		if (statfs("./Maildir/", &statbuf))
			strerr_die3sys(111, FATAL, "statfs: ", "./Maildir/: ");
	}
	update_maildirsize(maildirsizefn.s, statbuf.f_bavail * statbuf.f_bsize, mailsize, mailcount, pw->pw_uid, pw->pw_gid);
	return (0);
}

void
getversion_maildirsize_c()
{
	const char     *x = "$Id: maildirsize.c,v 1.14 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: maildirsize.c,v $
 * Revision 1.14  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.13  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.12  2020-09-16 19:00:57+05:30  Cprogrammer
 * freeBSD fix
 *
 * Revision 1.11  2020-05-11 11:03:06+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.10  2020-04-07 11:45:07+05:30  Cprogrammer
 * omit ./Maildir when directory is specfied on command line
 *
 * Revision 1.9  2020-04-07 10:05:48+05:30  Cprogrammer
 * added getopt to allow command line argument for user and optional directory
 *
 * Revision 1.8  2020-04-01 16:16:17+05:30  Cprogrammer
 * fixed typo in error message
 *
 * Revision 1.7  2020-03-24 13:02:14+05:30  Cprogrammer
 * use qcount_dir() instead of loadLibrary() to load count_dir() function from libindimail
 *
 * Revision 1.6  2019-05-27 20:29:13+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.5  2019-05-27 12:35:45+05:30  Cprogrammer
 * set libfn with full path of libindimail control file
 *
 * Revision 1.4  2019-05-26 12:31:13+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.3  2019-04-20 19:51:25+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.2  2018-11-11 14:08:11+05:30  Cprogrammer
 * print maildirsize on stdout
 *
 * Revision 1.1  2018-11-11 13:52:20+05:30  Cprogrammer
 * Initial revision
 *
 */
