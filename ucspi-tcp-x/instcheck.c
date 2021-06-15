/*
 * $Log: instcheck.c,v $
 * Revision 1.4  2021-06-15 08:22:08+05:30  Cprogrammer
 * replaced strerr_die2sys with sterr_die2x for memory error
 *
 * Revision 1.3  2021-04-07 18:33:32+05:30  Cprogrammer
 * fix for systems not having man pages
 * fix for running under non-root uid
 *
 * Revision 1.2  2021-04-07 10:25:27+05:30  Cprogrammer
 * do not treat missing man as error (e.g. minimial docker images)
 *
 * Revision 1.1  2020-10-23 08:17:18+05:30  Cprogrammer
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

#define FATAL   "instcheck: fatal: "
#define WARNING "instcheck: warning: "

void
perm(char *prefix1, char *prefix2, char *prefix3, char *file, int type, int uid, int gid, int mode)
{
	struct stat     st;
	int             len, err = 0;
	char           *tfile = 0, *slashd;

	slashd = (prefix1 && *prefix1) ? "/" : "";
	if (stat(file, &st) != -1)
		tfile = file;
	else {
		if (errno != error_noent) {
			if (errno != error_acces)
				strerr_die8sys(111, FATAL, "unable to stat ", prefix1, slashd, prefix2, prefix3, file, ": ");
			strerr_warn8(WARNING, "unable to stat ", prefix1, slashd, prefix2, prefix3, file, ": ", &strerr_sys);
			return;
		}
		if (!str_diffn(prefix2, "man/", 4)) { /*- check for .gz extension */
			if (!(tfile = (char *) alloc((len = str_len(file)) + 4)))
				strerr_die2sys(111, FATAL, "unable to allocate mem: ");
			str_copy(tfile, file);
			str_copy(tfile + len, ".gz");
			if (stat(tfile, &st) == -1) {
				if (errno != error_noent)
					strerr_die7sys(111, FATAL, prefix1, slashd, prefix2, prefix3, file, ": ");
				else
					strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, file, " does not exist", 0);
				if (tfile != file)
					alloc_free(tfile);
				return;
			}
		} else
		if (!str_diffn(file, "lib", 3)) {
			if (stat("../lib64", &st) == -1) {
				strerr_warn8(WARNING, "unable to stat file", prefix1, slashd, prefix2, prefix3, file, ": ", &strerr_sys);
				return;
			} else {
				if (chdir("../lib64") == -1) {
					strerr_warn4(WARNING, "unable to chdir", "../lib64", ": ", &strerr_sys);
					return;
				}
			}
			if (stat(file, &st) == -1) {
				if (errno == error_noent)
					strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, file, " does not exist", 0);
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
					strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, file, " does not exist", 0);
				else
					strerr_warn4(WARNING, "unable to stat ", file, ": ", &strerr_sys);
				return;
			}
			tfile = file;
		} else {
			if (!str_diffn(file, "man", 3))
				strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, file, " does not exist", 0);
			else
				strerr_die7sys(111, FATAL, prefix1, slashd, prefix2, prefix3, file, ": ");
			return;
		}
	}
	if ((uid != -1) && (st.st_uid != uid)) {
		err = 1;
		strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, tfile, " has wrong owner (will fix)", 0);
	}
	if ((gid != -1) && (st.st_gid != gid)) {
		err = 1;
		strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, tfile, " has wrong group (will fix)", 0);
	}
	if (err && chown(tfile, uid, gid) == -1)
		strerr_die4sys(111, FATAL, "unable to chown ", tfile, ": ");
	err = 0;
	if (mode != -1 && (st.st_mode & 07777) != mode) {
		err = 1;
		strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, tfile, " has wrong permissions (will fix)", 0);
	}
	if (err && chmod(tfile, mode) == -1)
		strerr_die4sys(111, FATAL, "unable to chmod ", tfile, ": ");
	if ((st.st_mode & S_IFMT) != type)
		strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, tfile, " has wrong type (unable to fix)", 0);
	if (tfile != file)
		alloc_free(tfile);
}

char    *
getdirname(char *dir, char **basedir)
{
	char           *ptr;
	int             len;

	for (ptr = dir, len = 0; *ptr; ptr++, len++);
	ptr--;
	for (; ptr != dir && *ptr != '/'; ptr--, len--);
	if (basedir)
		*basedir = ptr;
	while (len > 1 && *ptr == '/')
		ptr--, len--;
	if (!stralloc_copyb(&dirbuf, dir, len) ||
			!stralloc_0(&dirbuf))
		strerr_die2x(111, FATAL, "out of memory");
	return (dirbuf.s);
}

void
h(char *home, int uid, int gid, int mode)
{
	perm("", "", "", home, S_IFDIR, uid, gid, mode);
}

void
d(char *home, char *subdir, int uid, int gid, int mode)
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	perm("", home, "/", subdir, S_IFDIR, uid, gid, mode);
}

void
p(char *home, char *fifo, int uid, int gid, int mode)
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	perm("", home, "/", fifo, S_IFIFO, uid, gid, mode);
}

void
c(char *home, char *subdir, char *file, int uid, int gid, int mode)
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (chdir(subdir) == -1) {
		if (errno == error_noent && !str_diffn(subdir, "man/", 4)) {
			strerr_warn8(WARNING, "skipping ", home, "/", subdir, "/", file, ": ", &strerr_sys);
			return;
		}
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
		return;
	}
	perm(home, subdir, "/", file, S_IFREG, uid, gid, mode);
}

int
main(int argc, char **argv)
{
	hier(0, FATAL);
	_exit(0);
	/*- Not reached */
}
