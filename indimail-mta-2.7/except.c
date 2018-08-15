/*
 * $Log: except.c,v $
 * Revision 1.6  2004-10-22 20:24:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-07-17 21:18:40+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "strerr.h"
#include "wait.h"
#include "error.h"
#include "exit.h"

#define FATAL "except: fatal: "

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             pid;
	int             wstat;

	if (!argv[1])
		strerr_die1x(100, "except: usage: except program [ arg ... ]");
	pid = fork();
	if (pid == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0)
	{
		execvp(argv[1], argv + 1);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		_exit(100);
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		_exit(0);
	}
	/*- Not reached */
	return(0);
}

void
getversion_except_c()
{
	static char    *x = "$Id: except.c,v 1.6 2004-10-22 20:24:59+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
