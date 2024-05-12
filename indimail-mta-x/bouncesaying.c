/*
 * $Log: bouncesaying.c,v $
 * Revision 1.9  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2020-11-24 13:44:08+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 20:22:09+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-07-17 21:16:31+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "strerr.h"
#include "error.h"
#include "wait.h"
#include "sig.h"

#define FATAL "bouncesaying: fatal: "

int
main(int argc, char **argv)
{
	int             pid;
	int             wstat;

	if (!argv[1])
		strerr_die1x(100, "bouncesaying: usage: bouncesaying error [ program [ arg ... ] ]");
	if (argv[2])
	{
		pid = fork();
		if (pid == -1)
			strerr_die2sys(111, FATAL, "unable to fork: ");
		if (pid == 0)
		{
			execvp(argv[2], argv + 2);
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
			break;
		case 111:
			strerr_die2x(111, FATAL, "temporary child error");
		default:
			_exit(0);
		}
	}
	strerr_die1x(100, argv[1]);
	/*- Not reached */
	return(0);
}

void
getversion_bouncesaying_c()
{
	const char     *x = "$Id: bouncesaying.c,v 1.9 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
