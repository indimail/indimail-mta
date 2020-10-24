/*
 * $Log: instcheck.c,v $
 * Revision 1.8  2020-10-23 09:40:24+05:30  Cprogrammer
 * fixed error message
 *
 * Revision 1.7  2020-09-19 17:38:56+05:30  Cprogrammer
 * removed unused argument to perm()
 *
 * Revision 1.6  2020-09-13 17:33:18+05:30  Cprogrammer
 * skip chmod if mode is -1
 *
 * Revision 1.5  2020-08-03 17:23:52+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.4  2016-05-27 20:46:20+05:30  Cprogrammer
 * FHS compliant instcheck
 *
 * Revision 1.3  2008-07-17 23:03:18+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.2  2008-06-30 09:39:35+05:30  Cprogrammer
 * removed z()
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strerr.h>
#include <stralloc.h>
#include <error.h>
#include <str.h>
#include <alloc.h>

char           *sharedir = 0;
stralloc        dirbuf = { 0 };

extern void     hier(char *, char *);

#define FATAL "instcheck: fatal: "
#define WARNING "instcheck: warning: "

void
perm(char *prefix1, char *prefix2, char *prefix3, char *file, int type, int uid, int gid, int mode)
{
	struct stat     st;
	int             len, err = 0;
	char           *tfile = 0;

	if (stat(file, &st) != -1)
		tfile = file;
	else {
		if (errno != error_noent) {
			strerr_warn4(WARNING, "unable to stat ", file, ": ", &strerr_sys);
			return;
		}
		if (!str_diffn(prefix2, "man/", 4)) { /*- check for .gz extension */
			if (!(tfile = (char *) alloc((len = str_len(file)) + 4)))
				strerr_die2sys(111, FATAL, "unable to allocate mem: ");
			str_copy(tfile, file);
			str_copy(tfile + len, ".gz");
			if (stat(tfile, &st) == -1) {
				if (errno != error_noent)
					strerr_warn4(WARNING, "unable to stat ", tfile, ": ", &strerr_sys);
				else
#ifdef INDIMAIL
				if (!ignore_man_error)
#endif
					strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, file, " does not exist", 0);
				if (tfile != file)
					alloc_free(tfile);
				return;
			}
		} else
		if (!str_diffn(file, "lib", 3)) {
			if (stat("../lib64", &st) == -1) {
				strerr_warn4(WARNING, "unable to stat file", file, ": ", &strerr_sys);
				return;
			} else {
				if (chdir("../lib64") == -1) {
					strerr_warn4(WARNING, "unable to chdir", "../lib64", ": ", &strerr_sys);
					return;
				}
			}
			if (stat(file, &st) == -1) {
				if (errno == error_noent)
					strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, file, " does not exist", 0);
				else
					strerr_warn4(WARNING, "unable to stat ", file, ": ", &strerr_sys);
				return;
			}
			tfile = file;
		} else
		if (!str_diffn(file, "sbin", 4)) {
			if (stat("../sbin", &st) == -1) {
				strerr_warn4(WARNING, "unable to stat ", file, ": ", &strerr_sys);
				return;
			} else {
				if (chdir("../sbin") == -1) {
					strerr_warn4(WARNING, "unable to chdir", "../sbin", ": ", &strerr_sys);
					return;
				}
			}
			if (stat(file, &st) == -1) {
				if (errno == error_noent)
					strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, file, " does not exist", 0);
				else
					strerr_warn4(WARNING, "unable to stat ", file, ": ", &strerr_sys);
				return;
			}
			tfile = file;
		} else {
			if (!str_diffn(file, "man/", 4))
				strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, file, " does not exist", 0);
			else
				strerr_die7sys(111, FATAL, prefix1, "/", prefix2, prefix3, file, ": ");
			return;
		}
	}
	if ((uid != -1) && (st.st_uid != uid)) {
		err = 1;
		strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, tfile, " has wrong owner (will fix)", 0);
	}
	if ((gid != -1) && (st.st_gid != gid)) {
		err = 1;
		strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, tfile, " has wrong group (will fix)", 0);
	}
	if (err && chown(tfile, uid, gid) == -1)
		strerr_die4sys(111, FATAL, "unable to chown ", tfile, ": ");
	err = 0;
	if (mode != -1 && (st.st_mode & 07777) != mode) {
		err = 1;
		strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, tfile, " has wrong permissions (will fix)", 0);
	}
	if (err && chmod(tfile, mode) == -1)
		strerr_die4sys(111, FATAL, "unable to chmod ", tfile, ": ");
	if ((st.st_mode & S_IFMT) != type)
		strerr_warn7(WARNING, prefix1, "/", prefix2, prefix3, tfile, " has wrong type (unable to fix)", 0);
	if (tfile != file)
		alloc_free(tfile);
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
	perm("", "", "", home, S_IFDIR, uid, gid, mode);
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
	perm("", home, "/", subdir, S_IFDIR, uid, gid, mode);
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
	perm("", home, "/", fifo, S_IFIFO, uid, gid, mode);
}

void
c(home, subdir, file, uid, gid, mode)
	char           *home;
	char           *subdir;
	char           *file;
	int             uid;
	int             gid;
	int             mode;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (chdir(subdir) == -1)
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	perm(home, subdir, "/", file, S_IFREG, uid, gid, mode);
}

int
main()
{
	hier(0, FATAL);
	_exit(0);
	/*- Not reached */
}
