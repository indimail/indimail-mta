/*
 * $Log: fghack.c,v $
 * Revision 1.4  2020-06-08 22:51:19+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.3  2004-10-22 20:25:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-09 19:20:24+05:30  Cprogrammer
 * removed buffer.h
 *
 * Revision 1.1  2003-12-31 19:30:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "wait.h"
#include "error.h"
#include "strerr.h"
#include "pathexec.h"

#define FATAL "fghack: fatal: "

int             pid;

int
main(int argc, char **argv, char **envp)
{
	char            ch;
	int             wstat;
	int             pi[2];
	int             i;

	if (!argv[1])
		strerr_die1x(100, "fghack: usage: fghack child");

	if (pipe(pi) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");

	switch (pid = fork())
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
	case 0:
		close(pi[0]);
		for (i = 0; i < 30; ++i)
			if (dup(pi[1]) == -1) ;
		pathexec_run(argv[1], argv + 1, envp);
		strerr_die4sys(111, FATAL, "unable to run ", argv[1], ": ");
	}

	close(pi[1]);

	for (;;)
	{
		i = read(pi[0], &ch, 1);
		if ((i == -1) && (errno == error_intr))
			continue;
		if (i == 1)
			continue;
		break;
	}

	if (wait_pid(&wstat, pid) == -1)
		strerr_die2sys(111, FATAL, "wait failed: ");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	_exit(wait_exitcode(wstat));
}

void
getversion_fghack_c()
{
	static char    *x = "$Id: fghack.c,v 1.4 2020-06-08 22:51:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
