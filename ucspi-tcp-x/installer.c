/*
 * $Log: installer.c,v $
 * Revision 1.1  2020-09-13 01:24:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <getln.h>
#include <open.h>
#include <error.h>
#include <strerr.h>
#include <byte.h>
#include <scan.h>

stralloc        target = { 0 };
char           *mailuser;
char           *mailgroup;
char           *to;
const char      FATAL[] = "installer: fatal: ";
char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[SUBSTDIO_OUTSIZE];
substdio        ssin;
substdio        ssout;

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
print_info(char *str, char *source, char *dest, mode_t mode, uid_t uid, gid_t gid)
{
	int             i, d, count;
	int             a[4];
	char            octal[5];

	substdio_puts(subfdout, str);
	substdio_puts(subfdout, " ");
	substdio_puts(subfdout, dest);
	substdio_puts(subfdout, " -user ");
	substdio_puts(subfdout, mailuser);
	substdio_puts(subfdout, " -group ");
	substdio_puts(subfdout, mailgroup);
	substdio_puts(subfdout, " -mode ");
	d = mode == -1 ? 0 : mode;
	for (count = i = 0;d != 0 && i < 4;++i) {
		a[i] = (d % 8) + '0';
		d /= 8;
		count++;
	}
	substdio_puts(subfdout, "0");
	d = 0;
	for (i = count - 1;i >= 0;--i)
		octal[d++] = a[i];
	octal[d] = 0;
	substdio_puts(subfdout, octal);
	if (source) {
		substdio_puts(subfdout, " -source ");
		substdio_puts(subfdout, source);
	}
	substdio_puts(subfdout, "\n");
	substdio_flush(subfdout);
}

void
doit(stralloc *line)
{
	char           *x;
	unsigned int    xlen, i;
	char           *type, *uidstr, *gidstr, *modestr, *mid, *name;
	struct passwd  *pw;
	struct group   *gr;
	uid_t           my_uid;
	gid_t           my_gid;
	unsigned long   uid, gid, mode;
	int             fdin, fdout, opt;

	x = line->s;
	xlen = line->len;
	opt = (*x == '?');
	x += opt;
	xlen -= opt;

	type = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	uidstr = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	gidstr = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	modestr = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	mid = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	name = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	target.len = 0;
	if (to && !stralloc_copys(&target, to))
		nomem();
	if (mid && !stralloc_cats(&target, mid))
		nomem();
	if (*type != 'l' && name && !stralloc_cats(&target, name))
		nomem();
	if (!stralloc_0(&target))
		nomem();
	target.len--;
	if (!target.len)
		return;
	if (xlen > 0)
		name = x;

	uid = gid = mode = -1;
	if (*uidstr)
		scan_ulong(uidstr, &uid);
	if (*gidstr)
		scan_ulong(gidstr, &gid);
	if (*modestr)
		scan_8long(modestr, &mode);
	my_gid = getgid();
	if (!(gr = getgrgid(my_gid)))
		strerr_die2sys(111, FATAL, "unable to get gids: ");
	mailgroup = gr->gr_name;
	my_uid = getuid();
	if (!(pw = getpwuid(my_uid)))
		strerr_die2sys(111, FATAL, "unable to get uids: ");
	mailuser = pw->pw_name;
	switch (*type)
	{
	case 'l':
		if (symlink(name, target.s) == -1 && errno != error_exist)
			strerr_die6sys(111, FATAL, "unable to symlink ", name, " to ", target.s, ": ");
		break;
	case 'd':
		print_info("makedir", 0, target.s, (mode_t) mode, (uid_t) uid, (gid_t) gid);
		if (mkdir(target.s, mode == -1 ? 0700 : mode) == -1 && errno != error_exist)
			strerr_die4sys(111, FATAL, "unable to mkdir ", target.s, ": ");
		if (uid != -1 && gid != -1 && !my_uid && chown(target.s, (uid_t) uid, (gid_t) gid) == -1)
			strerr_die4sys(111, FATAL, "unable to chown ", target.s, ": ");
		break;

	case 'c':
		print_info("install file", name, target.s, (mode_t) mode, (uid_t) uid, (gid_t) gid);
		if ((fdin = open_read(name)) == -1) {
			if (opt)
				return;
			else
				strerr_die4sys(111, FATAL, "unable to read ", name, ": ");
		}
		substdio_fdbuf(&ssin, read, fdin, inbuf, sizeof (inbuf));

		if ((fdout = open_trunc(target.s)) == -1)
			strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
		substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof (outbuf));

		switch (substdio_copy(&ssout, &ssin))
		{
		case -2:
			strerr_die4sys(111, FATAL, "unable to read ", name, ": ");
		case -3:
			strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
		}

		close(fdin);
		if (substdio_flush(&ssout) == -1)
			strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
		if (fsync(fdout) == -1)
			strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
		close(fdout);
		if (mode != -1 && chmod(target.s, (mode_t) mode) == -1)
			strerr_die4sys(111, FATAL, "unable to chmod ", target.s, ": ");
		if (uid != -1 && gid != -1 && !my_uid && chown(target.s, (uid_t) uid, (gid_t) gid) == -1)
			strerr_die4sys(111, FATAL, "unable to chown ", target.s, ": ");
		break;

	default:
		return;
	}

}

char            buf[256];
substdio        in = SUBSTDIO_FDBUF(read, 0, buf, sizeof (buf));
stralloc        line = { 0 };

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             match;

	umask(077);

	if (argc == 1)
		to = (char *) 0;
	else
	if (argc == 2)
		to = argv[1];
	else
		strerr_die2x(100, FATAL, "usage: installer [install_dir]");
	for (;;) {
		if (getln(&in, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input");
		if (line.len > 0)
			line.s[--line.len] = 0;
		doit(&line);
		if (!match)
			_exit(0);
	}
	(void) argc;
}

void
getversion_installer_c()
{
	static char    *x = "$Id: installer.c,v 1.1 2020-09-13 01:24:56+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
