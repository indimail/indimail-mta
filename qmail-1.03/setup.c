/*
 * $Log: setup.c,v $
 * Revision 1.27  2016-06-17 17:26:57+05:30  Cprogrammer
 * allow linked dir to have a different basename
 *
 * Revision 1.26  2016-06-05 13:21:34+05:30  Cprogrammer
 * treat files in sbin as programs
 *
 * Revision 1.25  2016-05-29 20:12:07+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.24  2016-05-27 20:47:41+05:30  Cprogrammer
 * FHS compliant setup program
 *
 * Revision 1.23  2016-05-18 15:54:59+05:30  Cprogrammer
 * added comments for documentation
 *
 * Revision 1.22  2014-07-27 12:16:27+05:30  Cprogrammer
 * added comment for ci()
 *
 * Revision 1.21  2011-07-29 09:30:02+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.20  2011-04-29 16:12:16+05:30  Cprogrammer
 * check env variable FAKED_MODE to detect fakeroot environment
 *
 * Revision 1.19  2010-07-09 10:02:09+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.18  2010-07-08 11:47:46+05:30  Cprogrammer
 * added ci() for debian packaging
 *
 * Revision 1.17  2010-06-06 10:10:13+05:30  Cprogrammer
 * use a less restrictive umask when not running as root
 *
 * Revision 1.16  2010-05-16 16:08:18+05:30  Cprogrammer
 * use daemon as substitute for "mail" user on OS X
 *
 * Revision 1.15  2009-12-09 23:57:53+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.14  2009-04-06 16:39:14+05:30  Cprogrammer
 * removed function cd() and use function c() for docs also
 *
 * Revision 1.13  2009-02-10 09:28:43+05:30  Cprogrammer
 * allow setup to work as non-root user
 *
 * Revision 1.12  2009-02-08 10:08:43+05:30  Cprogrammer
 * allow installation through non-root user
 *
 * Revision 1.11  2009-02-01 00:08:20+05:30  Cprogrammer
 * fixed display of perms for setuid programs
 * moved get_user(), get_group() to get_uids.c
 *
 * Revision 1.10  2008-08-03 18:26:16+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.9  2008-06-25 23:16:33+05:30  Cprogrammer
 * print install style commands on subfderr
 *
 * Revision 1.8  2008-05-26 22:21:03+05:30  Cprogrammer
 * added commandline argument for configurable qmail home
 *
 * Revision 1.7  2005-01-22 01:04:03+05:30  Cprogrammer
 * renamed install to setup
 *
 * Revision 1.6  2004-10-21 21:53:26+05:30  Cprogrammer
 * display the qmail installation directory in errors
 *
 * Revision 1.5  2004-07-17 21:19:12+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "substdio.h"
#include "auto_uids.h"
#include "subfd.h"
#include "str.h"
#include "env.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "exit.h"
#include "hasindimail.h"
#include "stralloc.h"
#include "sgetopt.h"

void            hier(char *, char *);
int             uidinit(int);
char           *get_user(uid_t);
char           *get_group(gid_t);
void            dd(char *, int, int, int, char *, char *);
void            df(int, int, int, char *, char *, char *, int);

#define FATAL "install: fatal: "

int             fdsourcedir = -1;
uid_t           my_uid;
gid_t           my_gid;
char           *mailuser = "mail";
char           *mailgroup = "mail";
char           *destdir = 0, *sharedir = 0;
int             lsb = 0;
stralloc        tmpdir = { 0 };
stralloc        dirbuf = { 0 };
stralloc        dird = { 0 };

/*- gnu install style display */
void
dd(cmd, uid, gid, mode, home, subdir)
	char           *cmd;
	int             uid;
	int             gid;
	int             mode;
	char           *home;
	char           *subdir;
{
	int             a[4];
	int             d, i, count;
	char            octal[5];

	substdio_puts(subfderr, cmd);
	substdio_puts(subfderr, " -m ");
	d = mode;
	for (count = i = 0;d != 0 && i < 4;++i)
	{
		a[i] = (d % 8) + '0';
		d /= 8;
		count++;
	}
	substdio_puts(subfderr, "0");
	d = 0;
	for (i = count - 1;i >= 0;--i)
		octal[d++] = a[i];
	octal[d] = 0;
	substdio_puts(subfderr, octal);
	substdio_puts(subfderr, " ");
	substdio_puts(subfderr, home);
	if (subdir)
	{
		substdio_puts(subfderr, "/");
		substdio_puts(subfderr, subdir);
	}
	substdio_puts(subfderr, "\n");

	substdio_puts(subfderr, "/usr/bin/chown ");
	if (my_uid)
		substdio_puts(subfderr, mailuser);
	else
		substdio_puts(subfderr, get_user(uid));
	substdio_puts(subfderr, ":");
	if (my_uid)
		substdio_puts(subfderr, mailgroup);
	else
		substdio_puts(subfderr, get_group(gid));
	substdio_puts(subfderr, " ");
	substdio_puts(subfderr, home);
	if (subdir)
	{
		substdio_puts(subfderr, "/");
		substdio_puts(subfderr, subdir);
	}
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
}

void
dl(cmd, home, subdir, target)
	char           *cmd;
	char           *home;
	char           *subdir;
	char           *target;
{
	substdio_puts(subfderr, cmd);
	substdio_puts(subfderr, " ");
	substdio_puts(subfderr, target);
	substdio_puts(subfderr, " ");
	substdio_puts(subfderr, home);
	substdio_puts(subfderr, "/");
	substdio_puts(subfderr, subdir);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
}

void
df(uid, gid, mode, file, home, subdir, strip)
	int             uid;
	int             gid;
	int             mode;
	char           *file;
	char           *home;
	char           *subdir;
	int             strip;
{
	int             a[4];
	int             d, i, count;
	char            octal[5];

	substdio_puts(subfderr, "/usr/bin/install -c -o ");
	if (my_uid)
		substdio_puts(subfderr, mailuser);
	else
		substdio_puts(subfderr, get_user(uid));
	substdio_puts(subfderr, " -g ");
	if (my_uid)
		substdio_puts(subfderr, mailgroup);
	else
		substdio_puts(subfderr, get_group(gid));
	substdio_puts(subfderr, " -m ");
	d = mode;
	for (count = i = 0;d != 0 && i < 4;++i)
	{
		a[i] = (d % 8) + '0';
		d /= 8;
		count++;
	}
	substdio_puts(subfderr, "0");
	d = 0;
	for (i = count - 1;i >= 0;--i)
		octal[d++] = a[i];
	octal[d] = 0;
	substdio_puts(subfderr, octal);
	substdio_puts(subfderr, " ");
	if (strip)
		substdio_puts(subfderr, "-s ");
	substdio_puts(subfderr, file);
	substdio_puts(subfderr, " ");
	substdio_puts(subfderr, home);
	substdio_puts(subfderr, "/");
	substdio_puts(subfderr, subdir);
	substdio_puts(subfderr, "/");
	substdio_puts(subfderr, file);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
}

int
myr_mkdir(home, mode)
	char           *home;
	int             mode;
{
	char           *ptr;
	int             i;

	if (!stralloc_copys(&dirbuf, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	for (ptr = dirbuf.s + 1;*ptr;ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(dirbuf.s, F_OK) && (i = mkdir(dirbuf.s, mode)) == -1)
				return (i);
			*ptr = '/';
		}
	}
	return (mkdir(dirbuf.s, mode));
}

char    *
getdirname(char *dir, char **basedir)
{
	char           *ptr;
	int             len;

	for (ptr = dir, len = 0;*ptr; ptr++, len++);
	ptr--;
	for (;ptr != dir && *ptr != '/';ptr--, len--);
	if (basedir)
		*basedir = ptr;
	while (len > 1 && *ptr == '/')
		ptr--,len--;
	if (!stralloc_copyb(&dirbuf, dir, len))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	return (dirbuf.s);
}

void
l(home, subdir, target, relative)
	char           *home;
	char           *subdir;
	char           *target;
	int             relative;
{
	if (destdir) {
		if (!stralloc_copys(&tmpdir, destdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
	} else
	if (!stralloc_copys(&tmpdir, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&tmpdir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (chdir(tmpdir.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", tmpdir.s, ": ");
	if (!stralloc_copys(&dird, target))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (relative) {
		if (!stralloc_append(&dird, "/"))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&dird, subdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
	}
	if (!stralloc_0(&dird))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (symlink(dird.s, subdir) == -1 && errno != error_exist)
		strerr_die6sys(111, FATAL, "unable to symlink ", subdir, " to ", dird.s, ": ");
	/*- dl(cmd, home, subdir, target) -*/
	dl("/usr/bin/ln -s", tmpdir.s, subdir, dird.s);
}

void
h(home, uid, gid, mode)
	char           *home;
	int             uid;
	int             gid;
	int             mode;
{
	if (destdir) {
		if (!stralloc_copys(&tmpdir, destdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
	} else
	if (!stralloc_copys(&tmpdir, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&tmpdir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (myr_mkdir(tmpdir.s, my_uid ? 0755 : 0700) == -1 && errno != error_exist)
		strerr_die4sys(111, FATAL, "unable to mkdir ", tmpdir.s, ": ");
	if (!my_uid && chown(tmpdir.s, uid, gid) == -1)
		strerr_die4sys(111, FATAL, "unable to chown ", tmpdir.s, ": ");
	if (!my_uid && chmod(tmpdir.s, mode) == -1)
		strerr_die4sys(111, FATAL, "unable to chmod ", tmpdir.s, ": ");
	dd("/usr/bin/mkdir", uid, gid, mode, tmpdir.s, 0);
}

void
d(home, subdir, uid, gid, mode)
	char           *home;
	char           *subdir;
	int             uid;
	int             gid;
	int             mode;
{
	if (destdir) {
		if (!stralloc_copys(&tmpdir, destdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_copy(&dird, &tmpdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_append(&tmpdir, "/"))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, subdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
	} else {
		if (!stralloc_copys(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_copy(&dird, &tmpdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_append(&tmpdir, "/"))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, subdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
	}
	if (!stralloc_0(&tmpdir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dird))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (myr_mkdir(tmpdir.s, my_uid ? 0755 : 0700) == -1 && errno != error_exist)
		strerr_die6sys(111, FATAL, "unable to mkdir ", tmpdir.s, "/", subdir, ": ");
	if (chdir(dird.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", dird.s, ": ");
	if (!my_uid && chown(subdir, uid, gid) == -1)
		strerr_die6sys(111, FATAL, "unable to chown ", dird.s, "/", subdir, ": ");
	if (!my_uid && chmod(subdir, mode) == -1)
		strerr_die6sys(111, FATAL, "unable to chmod ", dird.s, "/", subdir, ": ");
	dd("/usr/bin/mkdir", uid, gid, mode, dird.s, subdir);
}

char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[SUBSTDIO_OUTSIZE];
substdio        ssin;
substdio        ssout;

/*
 * Copy binaries, boot, man, cat
 */
void
c(home, subdir, file, uid, gid, mode)
	char           *home;
	char           *subdir;
	char           *file;
	int             uid;
	int             gid;
	int             mode;
{
	int             fdin, fdout, is_prog = 0, i, j;
	char           *subd;
	struct stat     st;

	if (!str_diff(subdir, "bin") || !str_diff(subdir, "sbin"))
		is_prog = 1;
	/*- for generating qmailprog.list */
	if (is_prog) {
		substdio_puts(subfdout, file);
		substdio_put(subfdout, "\n", 1);
	}
	if (fchdir(fdsourcedir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to source directory: ");
	if (!str_diff(subdir, "doc") && chdir("doc") == -1)
		strerr_die2sys(111, FATAL, "unable to switch to source doc directory: ");
	if ((fdin = open_read(file)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	substdio_fdbuf(&ssin, read, fdin, inbuf, sizeof inbuf);

	if (destdir) {
		if (!stralloc_copys(&tmpdir, destdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
	} else
	if (!stralloc_copys(&tmpdir, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&tmpdir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (chdir(tmpdir.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", tmpdir.s, ": ");

	if (lstat(subdir, &st) == -1)
		strerr_die6sys(111, FATAL, "unable to stat", tmpdir.s, "/", subdir, ": ");
	subd = subdir;
	if (st.st_mode & S_IFLNK) {
		if (destdir) {
			if (!stralloc_copys(&tmpdir, destdir))
				strerr_die2sys(111, FATAL, "out of memory: ");
			for (i = 28;;)
			{
				if (!stralloc_ready(&dird, i + 1))
					strerr_die2sys(111, FATAL, "out of memory: ");
				if ((j = readlink(subdir, dird.s, i)) == i) {
					i += 28;
				} else {
					dird.s[j] = 0;
					break;
				}
			}
			if (!stralloc_copys(&tmpdir, destdir))
				strerr_die2sys(111, FATAL, "out of memory: ");
			if (!stralloc_cat(&tmpdir, &dird))
				strerr_die2sys(111, FATAL, "out of memory: ");
			if (chdir(tmpdir.s) == -1)
				strerr_die6sys(111, FATAL, "unable to switch to ", destdir, "/", dird.s, ": ");
			if (!stralloc_copys(&tmpdir, destdir))
				strerr_die2sys(111, FATAL, "out of memory: ");
			if (!stralloc_cats(&tmpdir, home))
				strerr_die2sys(111, FATAL, "out of memory: ");
			if (!stralloc_0(&tmpdir))
				strerr_die2sys(111, FATAL, "out of memory: ");
			subd = subdir;
		} else
		if (chdir(subdir) == -1)
			strerr_die6sys(111, FATAL, "unable to switch to ", tmpdir.s, "/", subdir, ": ");
	} else
	if (chdir(subdir) == -1)
		strerr_die6sys(111, FATAL, "unable to switch to ", tmpdir.s, "/", subdir, ": ");
	if ((fdout = open_trunc(file)) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof outbuf);
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	case -3:
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	}
	close(fdin);
	if (substdio_flush(&ssout) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	if (fsync(fdout) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	if (close(fdout) == -1)		/*- NFS silliness */
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	if (!my_uid && chown(file, uid, gid) == -1)
		strerr_die8sys(111, FATAL, "unable to chown ", tmpdir.s, "/", subd, "/", file, ": ");
	if (!my_uid && chmod(file, mode) == -1)
		strerr_die8sys(111, FATAL, "unable to chmod ", tmpdir.s, "/", subd, "/", file, ": ");
	if (is_prog)
		substdio_flush(subfdout);
	df(uid, gid, mode, file, tmpdir.s, subd, is_prog ? 1 : 0);
}

#ifdef INDIMAIL
/* dummy function for setup. It works only in instcheck. */
void
ci(char *home, char *subdir, char *file, int uid, int gid, int mode)
{
	return;
}
#endif

char           *usage = "usage: setup -d destdir [-s sharedir] [instdir]";

int
main(int argc, char **argv)
{
	int             opt;
	struct passwd  *pw;
	struct group   *gr;

#ifdef DARWIN
	mailuser = "daemon";
	mailgroup = "daemon";
#endif
	while ((opt = getopt(argc, argv, "ld:s:")) != opteof) {
		switch (opt) {
		case 'd':
			destdir = optarg;
			break;
		case 's':
			sharedir = optarg;
			break;
		case 'l':
			lsb = 1;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (env_get("FAKED_MODE")) { /*-fix for debian generation */
		my_uid = 1;
		my_gid = 1;
	} else {
		my_uid = getuid();
		my_gid = getgid();
	}
	if ((fdsourcedir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (my_uid) {
		if (!(pw = getpwuid(my_uid)) && !(pw = getpwnam(mailuser))) /*- we expect this user to be present on all linux distros */
			strerr_die2sys(111, FATAL, "unable to get uids: ");
		if ((gr = getgrgid(my_gid)))
			mailgroup = gr->gr_name;
		mailuser = pw->pw_name;
		auto_uida = pw->pw_uid;
		auto_gidn = pw->pw_gid;
		umask(022);
	} else {
		if (uidinit(1) == -1)
			strerr_die2sys(111, FATAL, "unable to get uids/gids: ");
		umask(077);
	}
	if (optind + 1 != argc)
		hier(0, FATAL);
	else
		hier(argv[optind++], FATAL);
	return (0);
}

void
getversion_setup_c()
{
	static char    *x = "$Id: setup.c,v 1.27 2016-06-17 17:26:57+05:30 Cprogrammer Exp mbhangui $";
#ifdef INDIMAIL
	if (x)
		x = sccsidh;
#else
	if (x)
		x++;
#endif
}
