/*
 * $Log: instcheck.c,v $
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
#include "strerr.h"
#include "error.h"
#include "exit.h"

extern void     hier();

#define FATAL "instcheck: fatal: "
#define WARNING "instcheck: warning: "

void
perm(prefix1, prefix2, prefix3, file, type, uid, gid, mode)
	char           *prefix1;
	char           *prefix2;
	char           *prefix3;
	char           *file;
	int             type;
	int             uid;
	int             gid;
	int             mode;
{
	struct stat     st;

	if (stat(file, &st) == -1)
	{
		if (errno == error_noent)
			strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " does not exist", 0);
		else
			strerr_warn4(WARNING, "unable to stat .../", file, ": ", &strerr_sys);
		return;
	}

	if ((uid != -1) && (st.st_uid != uid))
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " has wrong owner", 0);
	if ((gid != -1) && (st.st_gid != gid))
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " has wrong group", 0);
	if ((st.st_mode & 07777) != mode)
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " has wrong permissions", 0);
	if ((st.st_mode & S_IFMT) != type)
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " has wrong type", 0);
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
	perm(".../", subdir, "/", file, S_IFREG, uid, gid, mode);
}

int
main()
{
	hier();
	_exit(0);
	/*- Not reached */
}
