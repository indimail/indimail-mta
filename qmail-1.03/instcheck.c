/*
 * $Log: instcheck.c,v $
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
#include "str.h"
#include "alloc.h"
#include "error.h"
#include "exit.h"
#include "hasindimail.h"

void            hier(char *);
#ifdef INDIMAIL
void            _hier(char *);
#endif
int             uidinit(int);

#define FATAL "instcheck: fatal: "
#define WARNING "instcheck: warning: "
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
	int             len, err = 0;
	char           *tfile = 0;

	if (stat(file, &st) == -1)
	{
		if (errno == error_noent)
		{
			if (!str_diffn(prefix2, "man/", 4)) /*- check for .gz extension */
			{
				if (!(tfile = (char *) alloc((len = str_len(file)) + 4)))
					strerr_die2sys(111, FATAL, "unable to allocate mem: ");
				str_copy(tfile, file);
				str_copy(tfile + len, ".gz");
				if (stat(tfile, &st) == -1)
				{
					if (errno == error_noent)
						strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " does not exist", 0);
					else
						strerr_warn4(WARNING, "unable to stat .../", tfile, ": ", &strerr_sys);
					if (tfile != file)
						alloc_free(tfile);
					return;
				}
			} else
			if (!str_diffn(file, "lib", 3))
			{
				if (stat("lib64", &st) == -1)
				{
					strerr_warn4(WARNING, "unable to stat .../", file, ": ", &strerr_sys);
					return;
				}
				tfile = file;
			} else
			{
				if (!str_diffn(file, "man/", 4))
					strerr_warn6(WARNING, prefix1, prefix2, prefix3, file, " does not exist", 0);
				else
					strerr_die6sys(111, FATAL, prefix1, prefix2, prefix3, file, ": ");
				return;
			}
		} else
		{
			strerr_warn4(WARNING, "unable to stat .../", file, ": ", &strerr_sys);
			return;
		}
	} else
		tfile = file;
	if ((uid != -1) && (st.st_uid != uid))
	{
		err = 1;
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, tfile, " has wrong owner (will fix)", 0);
	}
	if ((gid != -1) && (st.st_gid != gid))
	{
		err = 1;
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, tfile, " has wrong group (will fix)", 0);
	}
	if (err && chown(tfile, uid, gid) == -1)
		strerr_die4sys(111, FATAL, "unable to chown ", tfile, ": ");
	err = 0;
	if ((st.st_mode & 07777) != mode)
	{
		err = 1;
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, tfile, " has wrong permissions (will fix)", 0);
	}
	if (err && chmod(tfile, mode) == -1)
		strerr_die4sys(111, FATAL, "unable to chmod ", tfile, ": ");
	if ((st.st_mode & S_IFMT) != type)
		strerr_warn6(WARNING, prefix1, prefix2, prefix3, tfile, " has wrong type (unable to fix)", 0);
	if (tfile != file)
		alloc_free(tfile);
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
p(home, fifo, uid, gid, mode)
	char           *home;
	char           *fifo;
	int             uid;
	int             gid;
	int             mode;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	perm("", home, "/", fifo, S_IFIFO, uid, gid, mode, 1);
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
	{
		if (errno == error_noent && !str_diffn(subdir, "man/", 4))
			return;
		strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	}
	perm(".../", subdir, "/", file, S_IFREG, uid, gid, mode, 1);
}

#ifdef INDIMAIL
void
ci(home, subdir, file, uid, gid, mode)
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
	{
		if (errno == error_noent)
		{
			if (!str_diff(subdir, "lib"))
			{
				if (chdir("lib64") == -1)
				{
					if (errno == error_noent)
						return;
					strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
				} /*- fall through */
			} else
				return;
		} else
			strerr_die6sys(111, FATAL, "unable to switch to ", home, "/", subdir, ": ");
	}
	perm(".../", subdir, "/", file, S_IFREG, uid, gid, mode, 0);
}
#endif

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
	perm(".../", subdir, "/", file, S_IFREG, uid, gid, mode, 1);
}

void
z(home, file, len, uid, gid, mode)
	char           *home;
	char           *file;
	int             len;
	int             uid;
	int             gid;
	int             mode;
{
	if (chdir(home) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
	perm("", home, "/", file, S_IFREG, uid, gid, mode, 1);
}

int
main(int argc, char **argv)
{
	int             i;

	if(uidinit(1) == -1)
		strerr_die2sys(111, FATAL, "unable to get uids/gids: ");
	if (argc == 1)
	{
		hier(0);
#ifdef INDIMAIL
		_hier(0);
#endif
	} else
	for (i = 1;i < argc;i++)
	{
		hier(argv[i]);
#ifdef INDIMAIL
		_hier(argv[i]);
#endif
	}
	return (0);
}

void
getversion_instcheck_c()
{
	static char    *x = "$Id: instcheck.c,v 1.16 2011-07-29 09:28:28+05:30 Cprogrammer Stab mbhangui $";
#ifdef INDIMAIL
	if (x)
		x = sccsidh;
#else
	if (x)
		x++;
#endif
}
