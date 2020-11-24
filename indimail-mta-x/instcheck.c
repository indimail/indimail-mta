/*
 * $Log: instcheck.c,v $
 * Revision 1.30  2020-11-24 13:48:50+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.29  2020-10-21 20:33:45+05:30  Cprogrammer
 * removed unused argument to prerm
 *
 * Revision 1.27  2018-06-25 13:45:26+05:30  Cprogrammer
 * ignore permission denied errors when running as non-root
 *
 * Revision 1.26  2018-01-09 11:43:43+05:30  Cprogrammer
 * removed unused function ci()
 *
 * Revision 1.25  2017-05-05 20:19:20+05:30  Cprogrammer
 * added logdir
 *
 * Revision 1.24  2017-01-08 19:04:07+05:30  Cprogrammer
 * added option to skip devel man pages
 *
 * Revision 1.23  2016-06-17 17:22:00+05:30  Cprogrammer
 * allow linked dir to have a different basename
 *
 * Revision 1.22  2016-05-30 20:25:14+05:30  Cprogrammer
 * removed call to _hier() indimail function
 *
 * Revision 1.21  2016-05-29 20:03:24+05:30  Cprogrammer
 * fixed display of double slash "//"
 *
 * Revision 1.20  2016-05-27 20:47:15+05:30  Cprogrammer
 * FHS compliance
 *
 * Revision 1.19  2014-07-27 12:50:53+05:30  Cprogrammer
 * fixed check for executables in sbin or bin directory
 *
 * Revision 1.18  2014-04-17 12:01:40+05:30  Cprogrammer
 * ignore man pages error
 *
 * Revision 1.17  2014-01-23 19:03:18+05:30  Cprogrammer
 * fixed locating files in lib64
 *
 * Revision 1.16  2011-07-29 09:28:28+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.15  2010-10-06 22:30:06+05:30  Cprogrammer
 * fix for 64 bit systems
 *
 * Revision 1.14  2010-07-08 11:23:03+05:30  Cprogrammer
 * fix perms for all indimail programs for debian stupidity
 *
 * Revision 1.13  2009-12-09 23:58:36+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.12  2009-11-17 09:38:11+05:30  Cprogrammer
 * treat errors with man directories as warnings
 *
 * Revision 1.11  2009-04-30 21:12:20+05:30  Cprogrammer
 * check for compressed man page if uncompressed is missing
 *
 * Revision 1.10  2009-02-15 21:46:25+05:30  Cprogrammer
 * added uidinit() to initialize uids
 *
 * Revision 1.9  2008-08-03 18:25:50+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.8  2008-05-27 19:32:12+05:30  Cprogrammer
 * fix perms if not proper rather than just complain
 *
 * Revision 1.7  2008-05-26 22:19:59+05:30  Cprogrammer
 * added commandline argument for configurable qmail home
 *
 * Revision 1.6  2004-10-22 20:25:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:35:27+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-07-17 21:19:13+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include "strerr.h"
#include "auto_uids.h"
#include "str.h"
#include "alloc.h"
#include "error.h"
#include "stralloc.h"

void            hier(char *, char *, int);
int             uidinit(int);

#define FATAL "instcheck: fatal: "
#define WARNING "instcheck: warning: "

stralloc        dirbuf = { 0 };

void
perm(prefix1, prefix2, prefix3, file, type, uid, gid, mode, should_exit)
	char           *prefix1;
	char           *prefix2;
	char           *prefix3;
	char           *file;
	int             type;
	int             uid;
	int             gid;
	int             mode;
	int             should_exit;
{
	struct stat     st;
	uid_t           myuid;
	int             len, err = 0;
	char           *tfile = 0, *slashd;

	myuid = getuid();
	slashd = (prefix1 && *prefix1) ? "/" : "";
	if (stat(file, &st) != -1)
		tfile = file;
	else {
		if (errno != error_noent) {
			if (errno == error_acces)
				return;
			strerr_die4sys(111, FATAL, "unable to stat ", file, ": ");
		}
		if (!str_diffn(prefix2, "man/", 4)) {
			if (!(tfile = (char *) alloc((len = str_len(file)) + 4)))
				strerr_die2sys(111, FATAL, "unable to allocate mem: ");
			/*- check for .gz extension */
			str_copy(tfile, file);
			str_copy(tfile + len, ".gz");
			if (stat(tfile, &st) == -1) {
				if (myuid == 0 && errno != error_noent)
					strerr_die4sys(111, FATAL, "unable to stat ", tfile, ": ");
				else
					strerr_warn7(WARNING, prefix1, slashd, prefix2, prefix3, file, " does not exist", 0);
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
			if (!str_diffn(file, "man/", 4))
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
	if ((st.st_mode & 07777) != mode) {
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

void
l(home, subdir, target, dummy)
	char           *home;
	char           *subdir;
	char           *target;
	int             dummy;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	if (chdir(subdir) == -1)
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
}

void
h(home, uid, gid, mode)
	char           *home;
	int             uid;
	int             gid;
	int             mode;
{
	perm("", "", "", home, S_IFDIR, uid, gid, mode, 1);
}

char           *
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
	if (!stralloc_copyb(&dirbuf, dir, len))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	return (dirbuf.s);
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
	perm("", home, "/", subdir, S_IFDIR, uid, gid, mode, 1);
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
	if (chdir(subdir) == -1) {
		if (errno == error_noent && !str_diffn(subdir, "man/", 4))
			return;
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	}
	perm(home, subdir, "/", file, S_IFREG, uid, gid, mode, 1);
}

void
cd(home, subdir, file, uid, gid, mode)
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
	perm(home, subdir, "/", file, S_IFREG, uid, gid, mode, 1);
}

int
main(int argc, char **argv)
{
	int             i;

	if (uidinit(1) == -1)
		strerr_die2sys(111, FATAL, "unable to get uids/gids: ");
	if (argc == 1)
		hier(0, FATAL, 0);
	else
		for (i = 1; i < argc; i++)
			hier(argv[i], FATAL, 0);
#ifdef LOGDIR
	h(LOGDIR, auto_uidl, auto_gidn, 0755);
#endif
	return (0);
}

void
getversion_instcheck_c()
{
	static char    *x = "$Id: instcheck.c,v 1.30 2020-11-24 13:48:50+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
