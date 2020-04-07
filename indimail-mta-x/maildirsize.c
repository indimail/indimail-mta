/*
 * $Log: maildirsize.c,v $
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
#include <pwd.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include "sgetopt.h"
#include "subfd.h"
#include "strerr.h"
#include "fmt.h"
#include "open.h"
#include "qcount_dir.h"

#define FATAL "maildirsize: fatal: "

struct substdio ssout;
char            ssoutbuf[256];

int
main(int argc, char **argv)
{
	int             fd, len, opt;
	size_t          mailcount;
	int64_t         mailsize;
	char            strnum[FMT_ULONG];
	char           *user = (char *) 0, *dir = (char *) 0;
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
	if (optind + 1 == argc)
		dir = argv[optind++];
	if (!(pw = getpwnam(user)))
		strerr_die3x(111, FATAL, "unknown account ", user);
	if (!dir)
		dir = pw->pw_dir;
	if (chdir(dir))
		strerr_die4sys(111, FATAL, "chdir: ", dir, ": ");
	if (access("./Maildir/", F_OK))
		strerr_die4sys(111, FATAL, ": ", dir, "/Maildir/: ");
	mailcount = 0;
	mailsize = qcount_dir("./Maildir/", &mailcount);
	if ((fd = open_trunc("./Maildir/maildirsize")) == -1)
		strerr_die2sys(111, FATAL, "./Maildir/maildirsize: ");
	if (fchown(fd, pw->pw_uid, pw->pw_gid))
		strerr_die4sys(111, FATAL, "chown: ", dir, "/Maildir/maildirsize: ");
	else
	if (fchmod(fd, 0644))
		strerr_die4sys(111, FATAL, "chmod: ", dir, "/Maildir/maildirsize: ");
	substdio_fdbuf(&ssout, write, fd, ssoutbuf, sizeof(ssoutbuf));
	if (statfs("./Maildir/", &statbuf))
		strerr_die3sys(111, FATAL, "statfs: ", "./Maildir/: ");
	strnum[len = fmt_ulong(strnum, statbuf.f_bavail * statbuf.f_bsize)] = 0;
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
	return (0);
}

void
getversion_maildirsize_c()
{
	static char    *x = "$Id: maildirsize.c,v 1.9 2020-04-07 10:05:48+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
