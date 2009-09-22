/*
 * $Log: envuidgid.c,v $
 * Revision 1.2  2004-10-22 20:24:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:39:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <pwd.h>
#include "fmt.h"
#include "strerr.h"
#include "pathexec.h"

#define FATAL "envuidgid: fatal: "

void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

char            strnum[FMT_ULONG];
char           *account;
struct passwd  *pw;

int
main(int argc, char **argv)
{
	account = *++argv;
	if (!account || !*++argv)
		strerr_die1x(100, "envuidgid: usage: envuidgid account child");

	pw = getpwnam(account);
	if (!pw)
		strerr_die3x(111, FATAL, "unknown account ", account);

	strnum[fmt_ulong(strnum, pw->pw_gid)] = 0;
	if (!pathexec_env("GID", strnum))
		nomem();
	strnum[fmt_ulong(strnum, pw->pw_uid)] = 0;
	if (!pathexec_env("UID", strnum))
		nomem();

	pathexec(argv);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_envuidgid_c()
{
	static char    *x = "$Id: envuidgid.c,v 1.2 2004-10-22 20:24:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
