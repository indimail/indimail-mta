/*
 * $Log: rpmattr.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2021-06-27 11:36:54+05:30  Cprogrammer
 * removed call to uidinit
 *
 * Revision 1.6  2021-06-24 12:17:06+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.5  2011-05-13 10:57:00+05:30  Cprogrammer
 * make rpmattr work for non-indimail installations
 *
 * Revision 1.4  2009-12-09 23:57:46+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.3  2009-06-25 12:40:28+05:30  Cprogrammer
 * display 4 digits perms
 *
 * Revision 1.2  2009-02-03 13:02:40+05:30  Cprogrammer
 * display the actual user/group when get_user() or get_group() returns nobody
 *
 * Revision 1.1  2009-02-02 20:09:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define WARNING "rpmattr: warn: "

static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof sserrbuf);
static char     strnum[FMT_ULONG];

void
outs_octal(int dec)
{
	int             k, o, i;
	char            octal[5];

	k = 0;
	o = dec;
	i = 3;
	while (o > 0) {
		k = o % 8;
		o = o / 8;
		octal[i--] = k + '0';
		if (i == -1 && o > 0) {
			if (substdio_puts(&sserr, "invalid number: ") == -1)
				_exit(1);
			strnum[fmt_ulong(strnum, dec)] = 0;
			if (substdio_puts(&sserr, strnum) == -1)
				_exit(1);
			if (substdio_puts(&sserr, "\n") == -1)
				_exit(1);
			if (substdio_flush(&sserr) == -1)
				_exit(1);
			_exit(1);
		}
	}
	octal[4] = 0;
	if (!i && substdio_puts(&ssout, "0") == -1)
		_exit(1);
	if (substdio_puts(&ssout, octal + i + 1) == -1)
		_exit(1);
}

int
main(int argc, char **argv)
{
	int             i;
	char           *user, *group;
	struct stat     statbuf;
	struct passwd  *pw;
	struct group   *gr;

	for (i = 1;i < argc;i++) {
		if (lstat(argv[i], &statbuf)) {
			strerr_warn4(WARNING, "lstat: ", argv[i], ": ", &strerr_sys);
			continue;
		}
		if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
			if (substdio_puts(&ssout, "%attr(") == -1)
				return (1);
		} else
		if (substdio_puts(&ssout, "%dir %attr(") == -1)
			return (1);
		outs_octal(statbuf.st_mode & 07777);
		if (substdio_puts(&ssout, ",") == -1)
			return (1);
		pw = getpwuid(statbuf.st_uid);
		user = pw->pw_name;
		if (substdio_puts(&ssout, user) == -1)
			return (1);
		if (substdio_puts(&ssout, ",") == -1)
			return (1);
		gr = getgrgid(statbuf.st_gid);
		group = gr->gr_name;
		if (substdio_puts(&ssout, group) == -1)
			return (1);
		if (substdio_puts(&ssout, ") ") == -1)
			return (1);
		if (substdio_puts(&ssout, argv[i]) == -1)
			return (1);
		if (substdio_puts(&ssout, "\n") == -1)
			return (1);
		if (substdio_flush(&ssout) == -1)
			return (1);
	}
	return (0);
}

void
getversion_rpmattr_c()
{
	const char     *x = "$Id: rpmattr.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
	x++;
}
