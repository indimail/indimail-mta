/*
 * $Log: setup.c,v $
 * Revision 1.5  2016-05-27 20:45:47+05:30  Cprogrammer
 * fixed shareddir setting
 *
 * Revision 1.4  2016-05-23 16:07:40+05:30  Cprogrammer
 * fhs compliance code
 *
 * Revision 1.3  2016-05-23 04:43:17+05:30  Cprogrammer
 * fhs compliance
 *
 * Revision 1.2  2009-02-08 10:10:11+05:30  Cprogrammer
 * allow installation as non-root user
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "buffer.h"
#include "fmt.h"
#include "stralloc.h"
#include "str.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "exit.h"
#include "sgetopt.h"
#include <unistd.h>
#include <sys/stat.h>

void            dd(char *, int, int, int, char *, char *);
void            df(int, int, int, char *, char *, char *, int);
extern void     hier(char *, char *);

char           *usage = "usage: setup -d destdir -s sharedir [instdir]";
char           *destdir = 0, *sharedir = 0;
stralloc        tmpdir = { 0 };
stralloc        dirbuf = { 0 };
stralloc        dird = { 0 };

#define FATAL "install: fatal: "

int             fdsourcedir = -1;
uid_t           my_uid;

void
dd(cmd, uid, gid, mode, home, subdir)
	char           *cmd;
	int             uid;
	int             gid;
	int             mode;
	char           *home;
	char           *subdir;
{
	int             a[3];
	int             d, i, count;
	static char     strnum[FMT_ULONG];

	buffer_puts(buffer_2, cmd);
	buffer_puts(buffer_2, " -m ");
	d = mode;
	for(count = i = 0;d != 0 && i < 3;++i)
	{
		a[i] = d % 8;
		d /= 8;
		count += 1;
	}
	buffer_puts(buffer_2, "0");
	for(i = count - 1;i >= 0;--i)
	{
		strnum[fmt_ulong(strnum, a[i])] = 0;
		buffer_puts(buffer_2, strnum);
	}
	buffer_puts(buffer_2, " ");
	buffer_puts(buffer_2, home);
	if (subdir)
	{
		buffer_puts(buffer_2, "/");
		buffer_puts(buffer_2, subdir);
	}
	buffer_puts(buffer_2, "\n");
	if (uid != -1 && gid != -1)
	{
		buffer_puts(buffer_2, "/usr/bin/chown ");
		buffer_puts(buffer_2, "-1:-1 ");
		buffer_puts(buffer_2, home);
		if (subdir)
		{
			buffer_puts(buffer_2, "/");
			buffer_puts(buffer_2, subdir);
		}
		buffer_puts(buffer_2, "\n");
	}
	buffer_flush(buffer_2);
}

void
dl(cmd, home, subdir, target)
	char           *cmd;
	char           *home;
	char           *subdir;
	char           *target;
{
	buffer_puts(buffer_2, cmd);
	buffer_puts(buffer_2, " ");
	buffer_puts(buffer_2, target);
	buffer_puts(buffer_2, " ");
	buffer_puts(buffer_2, home);
	buffer_puts(buffer_2, "/");
	buffer_puts(buffer_2, subdir);
	buffer_puts(buffer_2, "\n");
	buffer_flush(buffer_2);
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
	int             a[3];
	int             d, i, count;
	static char     strnum[FMT_ULONG];

	buffer_puts(buffer_2, "/usr/bin/install -c ");
	if (uid != -1 && gid != -1)
		buffer_puts(buffer_2, "-o -1 -g -1 ");
	buffer_puts(buffer_2, "-m ");
	d = mode;
	for(count = i = 0;d != 0 && i < 3;++i)
	{
		a[i] = d % 8;
		d /= 8;
		count += 1;
	}
	buffer_puts(buffer_2, "0");
	for(i = count - 1;i >= 0;--i)
	{
		strnum[fmt_ulong(strnum, a[i])] = 0;
		buffer_puts(buffer_2, strnum);
	}
	buffer_puts(buffer_2, " ");
	if (strip)
		buffer_puts(buffer_2, "-s ");
	buffer_puts(buffer_2, file);
	buffer_puts(buffer_2, " ");
	buffer_puts(buffer_2, home);
	buffer_puts(buffer_2, "/");
	buffer_puts(buffer_2, subdir);
	buffer_puts(buffer_2, "/");
	buffer_puts(buffer_2, file);
	buffer_puts(buffer_2, "\n");
	buffer_flush(buffer_2);
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

char            inbuf[BUFFER_INSIZE];
char            outbuf[BUFFER_OUTSIZE];
buffer          ssin;
buffer          ssout;

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

	if (!str_diff(subdir, "bin"))
		is_prog = 1;
	/*- for generating qmailprog.list */
	if (is_prog) {
		buffer_puts(buffer_1, file);
		buffer_put(buffer_1, "\n", 1);
	}
	if (fchdir(fdsourcedir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to source directory: ");
	if (!str_diff(subdir, "doc") && chdir("doc") == -1)
		strerr_die2sys(111, FATAL, "unable to switch to source doc directory: ");
	if((fdin = open_read(file)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	buffer_init(&ssin, read, fdin, inbuf, sizeof inbuf);

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
	if((fdout = open_trunc(file)) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	buffer_init(&ssout, write, fdout, outbuf, sizeof outbuf);
	switch (buffer_copy(&ssout, &ssin))
	{
	case -2:
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	case -3:
		strerr_die8sys(111, FATAL, "unable to write ", tmpdir.s, "/", subd, "/", file, ": ");
	}
	close(fdin);
	if (buffer_flush(&ssout) == -1)
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
		buffer_flush(buffer_1);
	df(uid, gid, mode, file, tmpdir.s, subd, is_prog ? 1 : 0);
}

int
main(int argc, char **argv)
{
	int             opt;

	while ((opt = getopt(argc, argv, "ld:s:")) != opteof) {
		switch (opt) {
		case 'd':
			destdir = optarg;
			break;
		case 's':
			sharedir = optarg;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	my_uid = getuid();
	if ((fdsourcedir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	umask(077);
	if (optind + 1 != argc)
		hier(0, FATAL);
	else
		hier(argv[optind++], FATAL);
	_exit(0);
	/*- Not reached */
}
