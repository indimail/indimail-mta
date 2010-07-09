/*
 * $Log: setup.c,v $
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
#include "substdio.h"
#include "auto_uids.h"
#include "subfd.h"
#include "str.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "fifo.h"
#include "exit.h"
#include "hasindimail.h"

void            hier(char *);
int             uidinit(int);
char           *get_user(uid_t);
char           *get_group(gid_t);
void            dd(char *, int, int, int, char *, char *);
void            df(int, int, int, char *, char *, char *, int);

#define FATAL "install: fatal: "

int             fdsourcedir = -1;
uid_t           my_uid;
char           *mailuser = "mail";

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
		substdio_puts(subfderr, "mail");
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
		substdio_puts(subfderr, "mail");
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

void
h(home, uid, gid, mode)
	char           *home;
	int             uid;
	int             gid;
	int             mode;
{
	if (mkdir(home, my_uid ? 0755 : 0700) == -1 && errno != error_exist)
		strerr_die4sys(111, FATAL, "unable to mkdir ", home, ": ");
	if (!my_uid && chown(home, uid, gid) == -1)
		strerr_die4sys(111, FATAL, "unable to chown ", home, ": ");
	if (!my_uid && chmod(home, mode) == -1)
		strerr_die4sys(111, FATAL, "unable to chmod ", home, ": ");
	dd("/usr/bin/mkdir", uid, gid, mode, home, 0);
}

void
d(home, subdir, uid, gid, mode)
	char           *home;
	char           *subdir;
	int             uid;
	int             gid;
	int             mode;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (mkdir(subdir, my_uid ? 0755 : 0700) == -1 && errno != error_exist)
		strerr_die6sys(111, FATAL, "unable to mkdir ", home, "/", subdir, ": ");
	if (!my_uid && chown(subdir, uid, gid) == -1)
		strerr_die6sys(111, FATAL, "unable to chown ", home, "/", subdir, ": ");
	if (!my_uid && chmod(subdir, mode) == -1)
		strerr_die6sys(111, FATAL, "unable to chmod ", home, "/", subdir, ": ");
	dd("/usr/bin/mkdir", uid, gid, mode, home, subdir);
}

void
p(home, fifo, uid, gid, mode)
	char           *home;
	char           *fifo;
	int             uid;
	int             gid;
	int             mode;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (fifo_make(fifo, my_uid ? 0755 : 0700) == -1 && errno != error_exist)
		strerr_die6sys(111, FATAL, "unable to mkfifo ", home, "/", fifo, ": ");
	if (!my_uid && chown(fifo, uid, gid) == -1)
		strerr_die6sys(111, FATAL, "unable to chown ", home, "/", fifo, ": ");
	if (!my_uid && chmod(fifo, mode) == -1)
		strerr_die6sys(111, FATAL, "unable to chmod ", home, "/", fifo, ": ");
	dd("/usr/bin/mkfifo", uid, gid, mode, home, fifo);
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
	int             fdin;
	int             fdout;
	int             is_prog = 0;

	if (!str_diff(subdir, "bin"))
		is_prog = 1;
	if (is_prog)
	{
		substdio_puts(subfdout, file);
		substdio_put(subfdout, "\n", 1);
	}
	if (fchdir(fdsourcedir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to source directory: ");
	if (!str_diff(subdir, "doc") && chdir("doc") == -1)
		strerr_die2sys(111, FATAL, "unable to switch to source doc directory: ");
	if((fdin = open_read(file)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	substdio_fdbuf(&ssin, read, fdin, inbuf, sizeof inbuf);
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (chdir(subdir) == -1)
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	if((fdout = open_trunc(file)) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", home, "/", subdir, "/", file, ": ");
	substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof outbuf);
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	case -3:
		strerr_die8sys(111, FATAL, "unable to write ", home, "/", subdir, "/", file, ": ");
	}
	close(fdin);
	if (substdio_flush(&ssout) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", home, "/", subdir, "/", file, ": ");
	if (fsync(fdout) == -1)
		strerr_die8sys(111, FATAL, "unable to write ", home, "/", subdir, "/", file, ": ");
	if (close(fdout) == -1)		/*- NFS silliness */
		strerr_die8sys(111, FATAL, "unable to write ", home, "/", subdir, "/", file, ": ");
	if (!my_uid && chown(file, uid, gid) == -1)
		strerr_die8sys(111, FATAL, "unable to chown ", home, "/", subdir, "/", file, ": ");
	if (!my_uid && chmod(file, mode) == -1)
		strerr_die8sys(111, FATAL, "unable to chmod ", home, "/", subdir, "/", file, ": ");
	if (is_prog)
		substdio_flush(subfdout);
	df(uid, gid, mode, file, home, subdir, is_prog ? 1 : 0);
}

#ifdef INDIMAIL
void
ci(char *home, char *subdir, char *file, int uid, int gid, int mode)
{
	return;
}
#endif

void
z(home, file, len, uid, gid, mode)
	char           *home;
	char           *file;
	int             len;
	int             uid;
	int             gid;
	int             mode;
{
	int             fdout;

	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if((fdout = open_trunc(file)) == -1)
		strerr_die6sys(111, FATAL, "unable to write ", home, "/", file, ": ");
	substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof outbuf);
	while (len-- > 0)
	{
		if (substdio_put(&ssout, "", 1) == -1)
			strerr_die6sys(111, FATAL, "unable to write ", home, "/", file, ": ");
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die6sys(111, FATAL, "unable to write ", home, "/", file, ": ");
	if (fsync(fdout) == -1)
		strerr_die6sys(111, FATAL, "unable to write ", home, "/", file, ": ");
	if (close(fdout) == -1)		/*- NFS silliness */
		strerr_die6sys(111, FATAL, "unable to write ", home, "/", file, ": ");
	if (!my_uid && chown(file, uid, gid) == -1)
		strerr_die6sys(111, FATAL, "unable to chown ", home, "/", file, ": ");
	if (!my_uid && chmod(file, mode) == -1)
		strerr_die6sys(111, FATAL, "unable to chmod ", home, "/", file, ": ");
}

int
main(int argc, char **argv)
{
	int             i;
	struct passwd  *pw;

#ifdef DARWIN
	mailuser = "daemon";
#endif
	my_uid = getuid();
	if((fdsourcedir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (my_uid)
	{
		if (!(pw = getpwnam(mailuser)))
			strerr_die2sys(111, FATAL, "unable to get uids/gids: ");
		auto_uida = pw->pw_uid;
		auto_gidn = pw->pw_gid;
		umask(022);
	} else
	{
		if(uidinit(1) == -1)
			strerr_die2sys(111, FATAL, "unable to get uids/gids: ");
		umask(077);
	}
	if (argc == 1)
		hier(0);
	else
	for (i = 1;i < argc;i++)
		hier(argv[i]);
	return (0);
}

void
getversion_setup_c()
{
	static char    *x = "$Id: setup.c,v 1.18 2010-07-08 11:47:46+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
