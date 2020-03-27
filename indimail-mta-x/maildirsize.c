/*
 * $Log: maildirsize.c,v $
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
#include "subfd.h"
#include "strerr.h"
#include "fmt.h"
#include "open.h"
#include "env.h"
#include "stralloc.h"
#include "control.h"
#include "variables.h"
#include "auto_control.h"
#include "qcount_dir.h"

#define FATAL "maildirsize: fatal: "

struct substdio ssout;
char            outbuf[256];

int
main(int argc, char **argv)
{
	int             fd, len;
	size_t          mailcount;
	int64_t         mailsize;
	char            strnum[FMT_ULONG];
	struct statfs   statbuf;
	struct passwd  *pw;

	if (argc != 2)
		strerr_die1x(100, "usage: maildirsize user");
	if (!(pw = getpwnam(argv[1])))
		strerr_die3x(111, FATAL, "unknown account ", argv[1]);
	if (chdir(pw->pw_dir))
		strerr_die3sys(111, FATAL, "chdir: ", pw->pw_dir);
	if (access("./Maildir/", F_OK))
		strerr_die4sys(111, FATAL, "chdir: ", pw->pw_dir, "/Maildir/");
	mailcount = 0;
	mailsize = qcount_dir("./Maildir/", &mailcount);
	if ((fd = open_trunc("./Maildir/maildirsize")) == -1)
		strerr_die2sys(111, FATAL, "./Maildir/maildirsize: ");
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
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
	static char    *x = "$Id: maildirsize.c,v 1.7 2020-03-24 13:02:14+05:30 Cprogrammer Exp mbhangui $";

	x++;
}