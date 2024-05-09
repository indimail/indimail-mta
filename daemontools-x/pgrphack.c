/*
 * $Log: pgrphack.c,v $
 * Revision 1.3  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:27:56+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:38:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "strerr.h"
#include "pathexec.h"

#define FATAL "pgrphack: fatal: "

int
main(int argc, char **argv, char **envp)
{
	if (!argv[1])
		strerr_die1x(100, "pgrphack: usage: pgrphack child");
	setsid(); /*- shouldn't fail; if it does, too bad */
	pathexec_run(argv[1], argv + 1, envp);
	strerr_die4sys(111, "pgrphack: fatal: ", "unable to run ", argv[1], ": ");
	/*- Not reached */
	return(1);
}

void
getversion_pgrphack_c()
{
	const char     *x = "$Id: pgrphack.c,v 1.3 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $";

	x++;
}
