/*
 * $Id: installer.c,v 1.28 2025-01-04 08:46:05+05:30 Cprogrammer Exp mbhangui $
 * taken from ezmlm-0.54
 */
#include <sys/types.h>
#if defined(linux)
#include <sys/sysmacros.h>
#endif
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
#include <str.h>
#include <byte.h>
#include <scan.h>
#include <fmt.h>
#include <noreturn.h>
#include <alloc.h>
#ifndef NETQMAIL
#include <qprintf.h>
#endif

static stralloc target = { 0 };

static char    *user, *group, *to;
const char      FATAL[] = "installer: fatal: ";
const char      WARN[] = "installer: warn: ";
static char     strnum[FMT_ULONG], inbuf[SUBSTDIO_INSIZE], outbuf[SUBSTDIO_OUTSIZE];
static substdio ssin, ssout;
static uid_t    my_uid;
static int      dofix, missing_ok;

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

char           *
get_octal(mode_t mode)
{
	int             a[4];
	int             i, d, count;
	static char     octal[5];

	d = mode;
	for (count = i = 0; d != 0 && i < 4; ++i) {
		a[i] = (d % 8) + '0';
		d /= 8;
		count++;
	}
	d = 0;
	octal[d++] = '0';
	for (i = count - 1; i >= 0; --i)
		octal[d++] = a[i];
	octal[d] = 0;
	return octal;
}

void
print_info(const char *str, const char *source, const char *dest,
		int mode, uid_t uid, gid_t gid, unsigned long size,
		unsigned int maj, unsigned int min)
{
	struct passwd  *pw;
	struct group   *gr;

#ifdef NETQMAIL
	substdio_puts(subfdout, str);
	substdio_puts(subfdout, " ");
	substdio_puts(subfdout, dest);
#else
	subprintf(subfdout, "%s %s", str, dest);
#endif
	if (gid != -1) {
		if (!(gr = getgrgid(gid))) {
			strnum[fmt_ulong(strnum, gid)] = 0;
			strerr_warn4(WARN, "unable to get gid entry for ", strnum, ": ", &strerr_sys);
		}
		group = gr->gr_name;
	}
	if (uid != -1) {
		if (!(pw = getpwuid(uid))) {
			strnum[fmt_ulong(strnum, uid)] = 0;
			strerr_warn4(WARN, "unable to get uid entry for ", strnum, ": ", &strerr_sys);
		}
		user = pw->pw_name;
#ifdef NETQMAIL
		substdio_puts(subfdout, " -owner ");
		substdio_puts(subfdout, user);
#else
		subprintf(subfdout, " -owner %s", user);
#endif
	}
#ifdef NETQMAIL
	if (gid != -1) {
		substdio_puts(subfdout, " -group ");
		substdio_puts(subfdout, group);
	}
	if (mode != -1) {
		substdio_puts(subfdout, " -mode ");
		substdio_puts(subfdout, get_octal(mode));
	}
	if (source) {
		substdio_puts(subfdout, " -source ");
		substdio_puts(subfdout, source);
	}
#else
	if (gid != -1)
		subprintf(subfdout, " -group %s", group);
	if (mode != -1)
		subprintf(subfdout, " -mode %s", get_octal(mode));
	if (source)
		subprintf(subfdout, " -source %s", source);
#endif
#ifdef NETQMAIL
	if (size != -1) {
		substdio_puts(subfdout, " -size ");
		strnum[fmt_ulong(strnum, size)] = 0;
		substdio_puts(subfdout, strnum);
	}
	if (maj != -1) {
		substdio_puts(subfdout, " -major ");
		strnum[fmt_ulong(strnum, maj)] = 0;
		substdio_puts(subfdout, strnum);
	}
	if (min != -1) {
		substdio_puts(subfdout, " -minor ");
		strnum[fmt_ulong(strnum, min)] = 0;
		substdio_puts(subfdout, strnum);
	}
#else
	if (size != -1)
		subprintf(subfdout, " -size %lu", size);
	if (maj != -1)
		subprintf(subfdout, " -major %u", maj);
	if (min != -1)
		subprintf(subfdout, " -minor %u", min);
#endif
	substdio_puts(subfdout, "\n");
	if (substdio_flush(&ssout) == -1)
		strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
}

int
myr_mkdir(char *home, mode_t mode)
{
	static stralloc dirbuf = { 0 };
	char           *ptr;
	int             i;

	if (!stralloc_copys(&dirbuf, home) || !stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	for (ptr = dirbuf.s + 1; *ptr; ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(dirbuf.s, F_OK) && (i = mkdir(dirbuf.s, 0755)) == -1)
				return (i);
			*ptr = '/';
		}
	}
	return (mkdir(dirbuf.s, mode));
}

int
qchown(const char *name, uid_t uid, gid_t gid)
{
	int             fd;

	if ((fd = open_read(name)) == -1)
		return -1;
	if (fchown(fd, uid, gid) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int
qchmod(const char *name, mode_t mode)
{
	int             fd;

	if ((fd = open_read(name)) == -1)
		return -1;
	if (fchmod(fd, mode) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

void
set_perms(char type, char *dest, char *uidstr, char *gidstr, char *modestr,
		uid_t uid, gid_t gid, int mode, int check)
{
	struct stat     st;
	struct passwd  *pw;
	struct group   *gr;

	if (lstat(dest, &st) == -1)
		strerr_die4sys(111, FATAL, "lstat: ", dest, ": ");
	if (check) {
		if (uid != -1 && st.st_uid != uid) {
			if (!(pw = getpwuid(st.st_uid))) {
				strnum[fmt_ulong(strnum, uid)] = 0;
				strerr_warn4(WARN, "unable to get uid entry for ", strnum, ": ", &strerr_sys);
			}
			if (!dofix)
				strerr_warn7(WARN, dest, " has wrong owner [", pw->pw_name, "] should be [", uidstr, "]", 0);
		}
		if (gid != -1 && st.st_gid != gid) {
			if (!(gr = getgrgid(st.st_gid))) {
				strnum[fmt_ulong(strnum, gid)] = 0;
				strerr_warn4(WARN, "unable to get gid entry for ", strnum, ": ", &strerr_sys);
			}
			if (!dofix)
				strerr_warn7(WARN, dest, " has wrong group [", gr->gr_name, "] should be [", gidstr, "]", 0);
		}
		if (mode != -1 && (st.st_mode & 07777) != mode) {
			if (!dofix)
				strerr_warn7(WARN, dest, " has wrong mode  [", get_octal(st.st_mode & 07777), "] should be [", modestr, "]", 0);
		}
		if (!dofix)
			return;
	}
	if (!dofix && my_uid)
		return;
	if (!my_uid && ((uid != -1 && st.st_uid != uid) || (gid != -1 && st.st_gid != gid))) {
		if (dofix && uid != -1 && st.st_uid != uid) {
			if (!(pw = getpwuid(st.st_uid))) {
				strnum[fmt_ulong(strnum, uid)] = 0;
				strerr_warn4(WARN, "unable to get uid entry for ", strnum, ": ", &strerr_sys);
			}
			strerr_warn7(WARN, dest, " has wrong owner [", pw->pw_name, "], will change it to [", uidstr, "]", 0);
#ifdef NETQMAIL
			substdio_puts(subfdout, "  chown ");
			substdio_puts(subfdout, uidstr);
			substdio_puts(subfdout, " ");
			substdio_puts(subfdout, dest);
			substdio_puts(subfdout, "\n");
#else
			subprintf(subfdout, "  chown %s %s\n", uidstr, dest);
#endif
		}
		if (dofix && gid != -1 && st.st_gid != gid) {
			if (!(gr = getgrgid(st.st_gid))) {
				strnum[fmt_ulong(strnum, gid)] = 0;
				strerr_warn4(WARN, "unable to get gid entry for ", strnum, ": ", &strerr_sys);
			}
			strerr_warn7(WARN, dest, " has wrong group [", gr->gr_name, "], will change it to [", gidstr, "]", 0);
#ifdef NETQMAIL
			substdio_puts(subfdout, "  chgrp ");
			substdio_puts(subfdout, gidstr);
			substdio_puts(subfdout, " ");
			substdio_puts(subfdout, dest);
			substdio_puts(subfdout, "\n");
#else
			subprintf(subfdout, "  chgrp %s %s\n", gidstr, dest);
#endif
		}
		if ((((type == 'c' || type == 'b') ? chown : qchown) (dest, uid, gid)) == -1)
			strerr_die4sys(111, FATAL, "unable to chmod ", dest, ": ");
		if (modestr[1] == '2' || modestr[1] == '4' || modestr[1] == '6') {
			if (lstat(dest, &st) == -1)
				strerr_die4sys(111, FATAL, "lstat: ", dest, ": ");
		}
	}
	if (mode != -1 && (st.st_mode & 07777) != mode) {
		if (dofix)
			strerr_warn7(WARN, dest, " has wrong mode  [", get_octal(st.st_mode & 07777), "], will change it to [", modestr, "]", 0);
#ifdef NETQMAIL
			substdio_puts(subfdout, "  chmod ");
			substdio_puts(subfdout, modestr);
			substdio_puts(subfdout, " ");
			substdio_puts(subfdout, dest);
			substdio_puts(subfdout, "\n");
#else
		subprintf(subfdout, "  chmod %s %s\n", modestr, dest);
#endif
		if ((((type == 'c' || type == 'b') ? chmod : qchmod) (dest, mode)) == -1)
			strerr_die4sys(111, FATAL, "unable to chmod ", dest, ": ");
	}
}

/*
 * any line starting with #, newline, tab or space is a  comment
 * invalid lines are silently ignored
 *
 * d:owner:group:mode:/usr::
 * d:owner:group:mode:target_dir::
 * l:owner:group:mode:link:target_dir:
 * f:owner:group:mode:dir:target_file:source
 * p:owner:group:mode:target_fifo::
 * c:owner:group:mode:target_device:devnum:
 * z:owner:group:mode:target_file:size:
 */
void
doit(stralloc *line, int uninstall, int check)
{
	char           *x;
	char           *type, *uidstr, *gidstr, *modestr, *mid, *name, *zerobuf;
	int             fdin, fdout, opt, mode;
	unsigned long   m, size = 0;
	unsigned int    xlen, i, maj, min;
	uid_t           uid;
	gid_t           gid;
	dev_t           dev;
	struct stat     st;
	struct passwd  *pw;
	struct group   *gr;

	/*- ignore comments, blank lines, any line starting with WSP */
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

	/* type */
	type = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	/* uid */
	uidstr = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	/* gid */
	gidstr = x;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	x[i++] = 0;
	x += i;
	xlen -= i;

	/* mode */
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
	if (*type == 'z')
		scan_ulong(x, &size);
	if (*type == 'c' || *type == 'b') {
		scan_uint(x, &maj);
		i = byte_chr(x, xlen, ':');
		if (i == xlen)
			return;
		x[i++] = 0;
		x += i;
		xlen -= i;
		scan_uint(x, &min);
	} else {
		i = byte_chr(x, xlen, ':');
		if (i < xlen)
			x[i] = 0;
	}

	target.len = 0;
	if (to && !stralloc_copys(&target, to))
		nomem();
	if (!stralloc_cats(&target, mid))
		nomem();
	if (name && !stralloc_cats(&target, name))
		nomem();
	if (!stralloc_0(&target))
		nomem();
	target.len--;
	if (!target.len)
		return;
	if (xlen > 0 && *type != 'z' && *type != 'c' && *type != 'b')
		name = x;

	if (uninstall) {
		switch (*type)
		{
		case 'l':
		case 'p':
		case 'c':
		case 'b':
			print_info("unlink", 0, target.s, -1, -1, -1, -1, -1, -1);
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
			print_info("rmdir", 0, target.s, -1, -1, -1, -1, -1, -1);
			if (rmdir(target.s) == -1) {
				if (errno == error_noent)
					return;
				strerr_warn4(WARN, "rmdir ", target.s, ": ", &strerr_sys);
			}
			break;
		case 'z':
		case 'f':
			print_info("delete", 0, target.s, -1, -1, -1, -1, -1, -1);
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
		scan_int(modestr, &mode);
		if (mode != -1) {
			scan_8long(modestr, &m);
			mode = (int) m;
		}
	} else
		mode = -1;

	switch (*type)
	{
	case 'c':
	case 'b':
		if (!check || access(target.s, F_OK)) {
			dev = makedev(maj, min);
			if (mode == -1)
				mode = 0644;
			if (*type == 'c') {
				mode = mode | S_IFCHR;
				print_info("mknod char", 0, target.s, mode == -1 ? 0644 : mode, uid, gid, -1, maj, min);
			}
			if (*type == 'b') {
				mode = mode | S_IFBLK;
				print_info("mknod block", 0, target.s, mode == -1 ? 0644 : mode, uid, gid, -1, maj, min);
			}
			if (mknod(target.s, mode, dev) == -1) {
				if (errno != error_exist)
					strerr_die4sys(111, FATAL, "mknod ", target.s, ": ");
			}
		}
		if (uid != -1 || gid != -1 || mode != -1)
			set_perms(*type, target.s, uidstr, gidstr, modestr, uid, gid, mode, check);
		break;
	case 'p':
		if (!check || access(target.s, F_OK)) {
			print_info("mkfifo", 0, target.s, mode == -1 ? 0644 : mode, uid, gid, -1, -1, -1);
			if (fifo_make(target.s, mode == -1 ? 0644 : mode) == -1) {
				if (errno != error_exist)
					strerr_die4sys(111, FATAL, "fifo_make: ", target.s, ": ");
			}
		}
		if (uid != -1 || gid != -1 || mode != -1)
			set_perms(*type, target.s, uidstr, gidstr, modestr, uid, gid, mode, check);
		break;
	case 'l':/*- here name is target */
		if (!check || access(name, F_OK)) {
			print_info("make link", name, target.s, -1, -1, -1, -1, -1, -1);
			if (symlink(name, target.s) == -1 && errno != error_exist)
				strerr_die6sys(111, FATAL, "unable to symlink ", target.s, " to ", name, ": ");
		}
		break;
	case 'd':
		if (!check || access(target.s, F_OK)) {
			print_info("mkdir", 0, target.s, (my_uid || mode == -1) ? 0755 : mode, uid, gid, -1, -1, -1);
			if (myr_mkdir(target.s, (my_uid || mode == -1) ? 0755 : mode) == -1) {
				if (errno != error_exist)
					strerr_die4sys(111, FATAL, "unable to mkdir ", target.s, ": ");
			}
		}
		if (uid != -1 || gid != -1 || mode != -1)
			set_perms(*type, target.s, uidstr, gidstr, modestr, uid, gid, mode, check);
		break;

	case 'z':
	case 'f':
		if (!check) {
			if (*type == 'f') {
				if (access(name, F_OK)) {
					if (errno == error_noent) {
						if (!missing_ok)
							strerr_die3sys(111, FATAL, name, ": ");
						print_info("skipping file", name, target.s, mode == -1 ? 0644 : mode, uid, gid, -1, -1, -1);
						return;
					} else
						strerr_die3sys(111, FATAL, name, ": ");
				}
				if (lstat(name, &st) == -1)
					strerr_die4sys(111, FATAL, "lstat: ", name, ": ");
				if (mode == -1)
					mode = st.st_mode;
				size = st.st_size;
			}
			print_info("install file", *type == 'f' ? name : "/dev/zero", target.s, mode == -1 ? 0644 : mode, uid, gid, size, -1, -1);
			if ((fdin = open_read(*type == 'z' ? "/dev/zero" : name)) == -1) {
				if (opt)
					return;
				else
					strerr_die4sys(111, FATAL, "unable to read ", name, ": ");
			}
			substdio_fdbuf(&ssin, read, fdin, inbuf, sizeof (inbuf));
			if ((fdout = open_trunc(target.s)) == -1)
				strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
			substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof (outbuf));

			if (*type == 'f') {
				switch (substdio_copy(&ssout, &ssin))
				{
				case -2:
					strerr_die4sys(111, FATAL, "unable to read ", name, ": ");
				case -3:
					strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
				}
			} else
			if (*type == 'z') {
				if (!(zerobuf = (char *) alloc(sizeof(char) * size)))
					nomem();
				if (substdio_get(&ssin, zerobuf, size) == -1)
					strerr_die2sys(111, FATAL, "unable to read /dev/zero: ");
				if (substdio_put(&ssout, zerobuf, size) == -1)
					strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
				alloc_free(zerobuf);
			}
			close(fdin);
			if (substdio_flush(&ssout) == -1)
				strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
			if (fsync(fdout) == -1)
				strerr_die4sys(111, FATAL, "unable to write ", target.s, ": ");
			close(fdout);
			if (my_uid) {
#ifdef NETQMAIL
				substdio_puts(subfdout, "\tchmod ");
				substdio_puts(subfdout, modestr);
				substdio_puts(subfdout, " ");
				substdio_puts(subfdout, target.s);
				substdio_puts(subfdout, "\n");
#else
				subprintf(subfdout, "\tchmod %s %s\n", modestr, target.s);
#endif
				if (qchmod(target.s, mode == -1 ? 0644 : mode) == -1)
					strerr_die4sys(111, FATAL, "unable to chmod ", target.s, ": ");
			}
		} else {
			if (access(target.s, F_OK)) {
				if (errno == error_noent) {
					if (missing_ok)
						strerr_warn3(WARN, target.s, " is missing", 0);
					else
						strerr_die3sys(111, FATAL, target.s, ": ");
					return;
				} else
					strerr_die3sys(111, FATAL, target.s, "12: ");
			}
			if (*type == 'z') {
				if (lstat(target.s, &st) == -1)
					strerr_die4sys(111, FATAL, "lstat: ", target.s, ": ");
				if (size != st.st_size) {
					strnum[fmt_ulong(strnum, size)] = 0;
					strerr_warn5(WARN, target.s, " is not the right size (", strnum, ").", 0);
				}
			}
		}
		if (uid != -1 || gid != -1 || mode != -1)
			set_perms(*type, target.s, uidstr, gidstr, modestr, uid, gid, mode, check);
		break;

	default:
		return;
	}
	substdio_flush(subfdout);
}

no_return void
die_usage()
{
	const char     *usage_str =
		"USAGE: installer [options] dest_dir\n"
		"options\n"
		"       -c check permissions of dir/files\n"
		"       -m ignore missing files\n"
		"       -f fix permissions\n"
		"       -u uninstall";
	strerr_die2x(100, FATAL, usage_str);
}

char            buf[256];
substdio        in = SUBSTDIO_FDBUF(read, 0, buf, sizeof (buf));
stralloc        line = { 0 };

int
main(int argc, char **argv)
{
	int             opt, match, uninstall = 0, check = 0;

	while ((opt = getopt(argc, argv, "cfmu")) != opteof) {
		switch (opt)
		{
		case 'c':
			check = 1;
			break;
		case 'f':
			dofix = 1;
			check = 1;
			break;
		case 'm':
			missing_ok = 1;
			break;
		case 'u':
			uninstall = 1;
			break;
		default:
			die_usage();
		}
	}
	if (optind < argc)
		to = argv[optind];
	else
		to = (char *) 0;
	if ((my_uid = getuid()))
		umask(077);
	if (dofix && my_uid)
		strerr_warn2(WARN, "installer not running as uid 0", 0);
	for (;;) {
		if (getln(&in, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input");
		if (line.len > 0)
			line.s[--line.len] = 0;
		doit(&line, uninstall, check);
		if (!match)
			_exit(0);
	}
}

void
getversion_installer_c()
{
	static const char *x = "$Id: installer.c,v 1.28 2025-01-04 08:46:05+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: installer.c,v $
 * Revision 1.28  2025-01-04 08:46:05+05:30  Cprogrammer
 * fix for FreeBSD, MacOS missing sysmacros.h
 *
 * Revision 1.27  2025-01-03 08:28:04+05:30  Cprogrammer
 * fixed creation of links, char/block devices
 *
 * Revision 1.26  2024-03-11 17:54:44+05:30  Cprogrammer
 * fixed displaying size variable
 * fixed allocation of zerobuf variable
 *
 * Revision 1.25  2024-02-12 17:19:34+05:30  Cprogrammer
 * added 'z' option to create files with fixed number of zeros
 *
 * Revision 1.24  2024-02-11 19:52:08+05:30  Cprogrammer
 * updated for netqmail, notqmail
 *
 * Revision 1.23  2024-02-10 23:29:15+05:30  Cprogrammer
 * removed unused options
 * fixed check option doing fix
 *
 * Revision 1.22  2024-02-08 23:26:45+05:30  Cprogrammer
 * replace chown, chmod with fchown, fchmod
 *
 * Revision 1.21  2022-06-20 00:47:40+05:30  Cprogrammer
 * fix directory creation on OSX
 *
 * Revision 1.20  2022-01-30 09:51:34+05:30  Cprogrammer
 * changed datatype of mode for comparision with -1
 *
 * Revision 1.19  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.18  2021-08-05 20:52:44+05:30  Cprogrammer
 * fixed bug of skipping files in check mode
 *
 * Revision 1.17  2021-08-05 17:16:22+05:30  Cprogrammer
 * allow installation for missig source if -m is specified
 *
 * Revision 1.16  2021-08-05 14:07:45+05:30  Cprogrammer
 * added -p option to create parent directories as needed
 * display usage for wrong options/usage
 *
 * Revision 1.15  2021-08-03 15:51:43+05:30  Cprogrammer
 * added -m option to ignore missing files
 *
 * Revision 1.14  2021-08-02 13:30:11+05:30  Cprogrammer
 * set default permissions when mode=-1
 *
 * Revision 1.13  2021-08-02 09:41:42+05:30  Cprogrammer
 * added check and fix mode
 *
 * Revision 1.12  2021-07-19 07:55:39+05:30  Cprogrammer
 * fixed setuid, setguid bits getting lost by doing chmod after chown
 *
 * Revision 1.11  2021-07-01 21:03:13+05:30  Cprogrammer
 * copy mode of original file if uid is non-root
 *
 * Revision 1.10  2021-04-07 17:07:31+05:30  Cprogrammer
 * feature to update permissions for staged builds
 *
 * Revision 1.9  2020-10-23 17:54:16+05:30  Cprogrammer
 * copy the mode of the source file to target
 *
 * Revision 1.8  2020-10-23 10:01:20+05:30  Cprogrammer
 * set actual mode
 *
 * Revision 1.7  2020-10-23 07:24:36+05:30  Cprogrammer
 * set default perms for dirs, executables, fifo and character special files
 *
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
