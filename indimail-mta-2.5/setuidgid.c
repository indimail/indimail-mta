/*
 * $Log: setuidgid.c,v $
 * Revision 1.2  2004-10-22 20:30:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:37:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <pwd.h>
#include "prot.h"
#include "strerr.h"
#include "pathexec.h"

#define FATAL "setuidgid: fatal: "

char           *account;
struct passwd  *pw;

int
main(int argc, char **argv, char **envp)
{
	account = *++argv;
	if (!account || !*++argv)
		strerr_die1x(100, "setuidgid: usage: setuidgid account child");

	pw = getpwnam(account);
	if (!pw)
		strerr_die3x(111, FATAL, "unknown account ", account);

	if (prot_gid(pw->pw_gid) == -1)
		strerr_die2sys(111, FATAL, "unable to setgid: ");
	if (prot_uid(pw->pw_uid) == -1)
		strerr_die2sys(111, FATAL, "unable to setuid: ");

	pathexec_run(*argv, argv, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_setuidgid_c()
{
	static char    *x = "$Id: setuidgid.c,v 1.2 2004-10-22 20:30:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
