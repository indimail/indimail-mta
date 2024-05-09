/*
 * $Log: envuidgid.c,v $
 * Revision 1.4  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.3  2010-06-08 21:58:33+05:30  Cprogrammer
 * pathexec now returns allocated environment on failure which should be freed
 *
 * Revision 1.2  2004-10-22 20:24:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:39:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <pwd.h>
#include <fmt.h>
#include <error.h>
#include <alloc.h>
#include <strerr.h>
#include <pathexec.h>
#include <noreturn.h>

#define FATAL "envuidgid: fatal: "

no_return void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

int
main(int argc, char **argv)
{
	char          **e;
	char            strnum[FMT_ULONG];
	char           *account;
	struct passwd  *pw;
	int             tmperrno;

	account = *++argv;
	if (!account || !*++argv)
		strerr_die1x(100, "envuidgid: usage: envuidgid account child");
	if (!(pw = getpwnam(account)))
		strerr_die3x(111, FATAL, "unknown account ", account);
	strnum[fmt_ulong(strnum, pw->pw_gid)] = 0;
	if (!pathexec_env("GID", strnum))
		nomem();
	strnum[fmt_ulong(strnum, pw->pw_uid)] = 0;
	if (!pathexec_env("UID", strnum))
		nomem();
	if ((e = pathexec(argv)))
	{
		tmperrno = errno;
		alloc_free((char *) e);
		errno = tmperrno;
	}
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_envuidgid_c()
{
	const char     *x = "$Id: envuidgid.c,v 1.4 2021-08-30 12:04:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
