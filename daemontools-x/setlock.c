/*
 * $Log: setlock.c,v $
 * Revision 1.3  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.2  2004-10-22 20:30:16+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:38:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <lock.h>
#include <open.h>
#include <strerr.h>
#include <pathexec.h>
#include <sgetopt.h>
#include <noreturn.h>

#define FATAL "setlock: fatal: "

no_return void
usage()
{
	strerr_die1x(100, "setlock: usage: setlock [ -nNxX ] file program [ arg ... ]");
}

int
main(int argc, char **argv, char **envp)
{
	int             opt, fd, flagndelay = 0, flagx = 0;
	char           *file;

	while ((opt = getopt(argc, argv, "nNxX")) != opteof) {
		switch (opt)
		{
		case 'n':
			flagndelay = 1;
			break;
		case 'N':
			flagndelay = 0;
			break;
		case 'x':
			flagx = 1;
			break;
		case 'X':
			flagx = 0;
			break;
		default:
			usage();
		}
	}
	argv += optind;
	if (!*argv)
		usage();
	file = *argv++;
	if (!*argv)
		usage();
	if ((fd = open_append(file)) == -1)
	{
		if (flagx)
			_exit(0);
		strerr_die4sys(111, FATAL, "unable to open ", file, ": ");
	}
	if ((flagndelay ? lock_exnb : lock_ex) (fd) == -1)
	{
		if (flagx)
			_exit(0);
		strerr_die4sys(111, FATAL, "unable to lock ", file, ": ");
	}
	pathexec_run(*argv, argv, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_setlock_c()
{
	const char     *x = "$Id: setlock.c,v 1.3 2021-08-30 12:04:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
