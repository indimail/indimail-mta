/*
 * $Log: pgrphack.c,v $
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
	const char     *x = "$Id: pgrphack.c,v 1.2 2004-10-22 20:27:56+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
