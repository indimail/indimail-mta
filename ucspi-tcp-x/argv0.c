/*
 * $Log: argv0.c,v $
 * Revision 1.2  2021-05-12 20:58:03+05:30  Cprogrammer
 * use upathexec_run()
 *
 * Revision 1.1  2005-01-22 01:02:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "upathexec.h"
#include "strerr.h"

int
main(int argc, char **argv, char **envp)
{
	if (argc < 3)
		strerr_die1x(100, "argv0: usage: argv0 realname program [ arg ... ]");
	upathexec_run(argv[1], argv + 2, envp);
	strerr_die4sys(111, "argv0: fatal: ", "unable to run ", argv[1], ": ");
	/*- Not reached */
	return(1);
}

void
getversion_argv0_c()
{
	static char    *x = "$Id: argv0.c,v 1.2 2021-05-12 20:58:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
