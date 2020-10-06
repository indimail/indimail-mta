/*
 * $Log: installer.c,v $
 * Revision 1.6  2020-10-06 11:51:26+05:30  Cprogrammer
 * fixed handling of symbolic files
 *
 * Revision 1.5  2020-10-05 14:57:42+05:30  Cprogrammer
 * added new features
 * 1. uninstall option (-u, -i)
 * 2. create devices
 * 3. create fifo
 *
 * Revision 1.4  2020-10-02 17:00:54+05:30  Cprogrammer
 * set permissions for directory only if it is a new directory
 *
 * Revision 1.3  2020-09-16 18:59:40+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.2  2020-09-13 17:33:04+05:30  Cprogrammer
 * sync permissions with indimail-mta.spec file
 *
 * Revision 1.1  2020-09-13 01:24:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fifo.h>
#include <pwd.h>
#include <grp.h>
#include <substdio.h>
#include <sgetopt.h>
#include <subfd.h>
#include <stralloc.h>
#include <getln.h>
#include <open.h>
#include <error.h>
#include <strerr.h>
#include <byte.h>
#include <scan.h>
#include <fmt.h>

stralloc        target = { 0 };
char           *user, *group, *to;
const char      FATAL[] = "installer: fatal: ";
const char      WARN[] = "installer: warning: ";
static char     strnum[FMT_ULONG];
char            inbuf[SUBSTDIO_INSIZE], outbuf[SUBSTDIO_OUTSIZE];
substdio        ssin, ssout;
uid_t           my_uid;

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
print_info(char *str, char *source, char *dest, int mode, int uid, int gid)
{
	int             i, d, count;
	int             a[4];
	char            octal[5];
	struct passwd  *pw;
	struct group   *gr;

	substdio_puts(subfdout, str);
	substdio_puts(subfdout, " ");
	substdio_puts(subfdout, dest);
	if (gid != -1) {
		if (!(gr = getgrgid(gid))) {
			strnum[fmt_ulong(strnum, gid)] = 0;
			strerr_die4sys(111, FATAL, "unable to get gid entry for ", strnum, ": ");
		}
		group = gr->gr_name;
	}
	if (uid != -1) {
		if (!(pw = getpwuid(uid))) {
			strnum[fmt_ulong(strnum, uid)] = 0;
			strerr_die4sys(111, FATAL, "unable to get uid entry for ", strnum, ": ");
		}
		user = pw->pw_name;
		substdio_puts(subfdout, " -user ");
		substdio_puts(subfdout, user);
	}
	if (gid != -1) {
		substdio_puts(subfdout, " -group ");
		substdio_puts(subfdout, group);
	}
	if (mode != -1) {
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
	}
	if (source) {
		substdio_puts(subfdout, " -source ");
		substdio_puts(subfdout, source);
	}
	substdio_puts(subfdout, "\n");
	substdio_flush(subfdout);
}

/*
 * anyline starting with #, newline or spaace is a  comment
 * d:owner:group:mode:target_dir::
 * l:owner:group:mode:link:target_dir:
 * f:owner:group:mode:target_file:name:
 * p:ower:group:mode:target_fifo::
 * c:ower:group:mode:target_device:devnum:
 */
void
doit(stralloc *line, int uninstall, int ign_dir)
{
	char           *x;
	char           *type, *uidstr, *gidstr, *modestr, *mid, *name;
	int             fdin, fdout, opt;
	unsigned long   m;
	unsigned int    xlen, i;
	int             uid, gid, mode;
	dev_t           dev;
	struct stat     st;
	struct passwd  *pw;
	struct group   *gr;

	switch (line->s[0])
	{
		case '#':
		case '\n':
		case ' ':
			return;
	}
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

	if (uninstall) {
		switch (*type)
		{
		case 'l':
			print_info("unlink", 0, target.s, -1, -1, -1);
			if (lstat(target.s, &st) == -1) {
				if (errno == error_noent)
					return;
				strerr_die4sys(111, FATAL, "lstat: ", target.s, ": ");
			}
			if ((st.st_mode & S_IFMT) == S_IFDIR)
				strerr_die3x(111, FATAL, target.s, ": is a directory");
			if (unlink(target.s) == -1 && errno != error_noent)
				strerr_die4sys(111, FATAL, "unable to unlink ", target.s, ": ");
			break;
		case 'd':
			if (ign_dir)
				return;
			print_info("rmdir", 0, target.s, -1, -1, -1);
			if (rmdir(target.s) == -1) {
				if (errno == error_noent)
					return;
				strerr_warn4(WARN, "rmdir ", target.s, ": ", &strerr_sys);
			}
			break;
		case 'f':
			print_info("delete", 0, target.s, -1, -1, -1);
			if (lstat(target.s, &st) == -1) {
				if (errno == error_noent)
					return;
				strerr_die4sys(111, FATAL, "lstat: ", name, ": ");
			}
			if ((st.st_mode & S_IFMT) == S_IFDIR)
				strerr_die3x(111, FATAL, name, ": is a directory");
			if (unlink(target.s) == -1 && errno != error_noent)
				strerr_die4sys(111, FATAL, "unable to unlink ", name, ": ");
			break;
		}
		return;
	}

	if (*uidstr) {
		if (!(pw = getpwnam(uidstr)))
			scan_uint(uidstr, (unsigned int *) &uid);
		else
			uid = pw->pw_uid;
	} else
		uid = -1;
	if (*gidstr) {
		if (!(gr = getgrnam(gidstr)))
			scan_uint(gidstr, (unsigned int *) &gid);
		else
			gid = gr->gr_gid;
	} else
		gid = -1;
	if (*modestr) {
		scan_8long(modestr, &m);
		mode = (int) m;
	} else
		mode = -1;
	switch (*type)
	{
	case 'c':
		print_info("mknod", 0, target.s, mode, uid, gid);
		scan_ulong(name, (unsigned long *) &dev);
		if (mknod(target.s, mode, dev) == -1) {
			if (errno != error_exist)
				strerr_die4sys(111, FATAL, "mknod ", target.s, ": ");
		} else {
			if (uid != -1 && gid != -1 && !my_uid && chown(target.s, (uid_t) uid, (gid_t) gid) == -1)
				strerr_die4sys(111, FATAL, "unable to chown ", target.s, ": ");
			if (mode != -1 && !my_uid && chmod(target.s, (mode_t) mode) == -1)
				strerr_die4sys(111, FATAL, "unable to chmod ", target.s, ": ");
		}
	case 'p':
		print_info("mkfifo", 0, target.s, mode, uid, gid);
		if (my_uid)
			mode = 0755;
		if (fifo_make(target.s, mode) == -1) {
			if (errno != error_exist)
				strerr_die4sys(111, FATAL, "fifo_make: ", target.s, ": ");
		} else {
			if (uid != -1 && gid != -1 && !my_uid && chown(target.s, (uid_t) uid, (gid_t) gid) == -1)
				strerr_die4sys(111, FATAL, "unable to chown ", target.s, ": ");
			if (mode != -1 && !my_uid && chmod(target.s, (mode_t) mode) == -1)
				strerr_die4sys(111, FATAL, "unable to chmod ", target.s, ": ");
		}
	case 'l': /*- here name is target */
		print_info("make link", name, target.s, -1, -1, -1);
		if (symlink(name, target.s) == -1 && errno != error_exist)
			strerr_die6sys(111, FATAL, "unable to symlink ", target.s, " to ", name, ": ");
		break;
	case 'd':
		print_info("makedir", 0, target.s, mode, uid, gid);
		if (my_uid)
			mode = 0755;
		if (mkdir(target.s, mode == -1 ? 0755 : mode) == -1) {
			if (errno != error_exist)
				strerr_die4sys(111, FATAL, "unable to mkdir ", target.s, ": ");
		} else {
			if (uid != -1 && gid != -1 && !my_uid && chown(target.s, (uid_t) uid, (gid_t) gid) == -1)
				strerr_die4sys(111, FATAL, "unable to chown ", target.s, ": ");
			if (mode != -1 && !my_uid && chmod(target.s, (mode_t) mode) == -1)
				strerr_die4sys(111, FATAL, "unable to chmod ", target.s, ": ");
		}
		break;

	case 'f':
		print_info("install file", name, target.s, mode, uid, gid);
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
		if (mode != -1 && !my_uid && chmod(target.s, (mode_t) mode) == -1)
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
	int             opt, match, uninstall = 0, ignore_dirs = 0;;

	while ((opt = getopt(argc, argv, "ui")) != opteof) {
		switch (opt)
		{
		case 'u':
			uninstall = 1;
			break;
		case 'i':
			ignore_dirs = 1;
			break;
		}
	}
	if (optind < argc)
		to = argv[optind];
	else
		to = (char *) 0;
	if ((my_uid = getuid()))
		umask(077);
	for (;;) {
		if (getln(&in, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input");
		if (line.len > 0)
			line.s[--line.len] = 0;
		doit(&line, uninstall, ignore_dirs);
		if (!match)
			_exit(0);
	}
}

void
getversion_installer_c()
{
	static char    *x = "$Id: installer.c,v 1.6 2020-10-06 11:51:26+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
