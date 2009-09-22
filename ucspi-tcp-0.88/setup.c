/*
 * $Log: setup.c,v $
 * Revision 1.2  2009-02-08 10:10:11+05:30  Cprogrammer
 * allow installation as non-root user
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "buffer.h"
#include "fmt.h"
#include "str.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "exit.h"
#include <unistd.h>
#include <sys/stat.h>

extern void     hier();

#define FATAL "install: fatal: "

int             fdsourcedir = -1;
uid_t           my_uid;

void            dd(char *, int, int, int, char *, char *);
void            df(int, int, int, char *, char *, char *, int);

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
	int             fdin, fdout;
	int             is_prog = 0;

	if (fchdir(fdsourcedir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to source directory: ");
	if ((fdin = open_read(file)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	buffer_init(&ssin, read, fdin, inbuf, sizeof inbuf);
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (chdir(subdir) == -1)
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	if ((fdout = open_trunc(file)) == -1)
		strerr_die6sys(111, FATAL, "unable to write .../", subdir, "/", file, ": ");
	buffer_init(&ssout, write, fdout, outbuf, sizeof outbuf);
	switch (buffer_copy(&ssout, &ssin))
	{
	case -2:
		strerr_die4sys(111, FATAL, "unable to read ", file, ": ");
	case -3:
		strerr_die6sys(111, FATAL, "unable to write .../", subdir, "/", file, ": ");
	}
	close(fdin);
	if (buffer_flush(&ssout) == -1)
		strerr_die6sys(111, FATAL, "unable to write .../", subdir, "/", file, ": ");
	if (fsync(fdout) == -1)
		strerr_die6sys(111, FATAL, "unable to write .../", subdir, "/", file, ": ");
	if (close(fdout) == -1)		/*- NFS silliness */
		strerr_die6sys(111, FATAL, "unable to write .../", subdir, "/", file, ": ");
	if (!my_uid && chown(file, uid, gid) == -1)
		strerr_die6sys(111, FATAL, "unable to chown .../", subdir, "/", file, ": ");
	if (!my_uid && chmod(file, mode) == -1)
		strerr_die6sys(111, FATAL, "unable to chmod .../", subdir, "/", file, ": ");
	if (!str_diffn(subdir, "bin", 3))
		is_prog = 1;
	df(uid, gid, mode, file, home, subdir, is_prog ? 1 : 0);
}

int
main(int argc, char **argv)
{
	int             i;

	my_uid = getuid();
	if ((fdsourcedir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	umask(077);
	if (argc == 1)
		hier(0);
	else
	for (i = 1;i < argc;i++)
		hier(argv[i]);
	_exit(0);
	/*- Not reached */
}
