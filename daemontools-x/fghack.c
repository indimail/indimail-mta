/*
 * $Id: fghack.c,v 1.6 2022-12-18 12:54:03+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <signal.h>
#include "wait.h"
#include "error.h"
#include "strerr.h"
#include "pathexec.h"
#include "fmt.h"

#define FATAL "fghack: fatal: "
#define WARN  "fghack: warning: "

int             pid;

int
main(int argc, char **argv, char **envp)
{
	char            ch;
	int             wstat, i;
	int             pi[2];
	char            strnum[FMT_ULONG];

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
		for (i = 0; i < 30; ++i) {
			if (dup(pi[1]) == -1)
				;
		}
		pathexec_run(argv[1], argv + 1, envp);
		strerr_die4sys(111, FATAL, "unable to run ", argv[1], ": ");
	}

	close(pi[1]);

	for (;;) {
		i = read(pi[0], &ch, 1);
		if ((i == -1) && (errno == error_intr))
			continue;
		if (i == 1)
			continue;
		break;
	}

	for (;;) {
		if (!(i = wait_pid(&wstat, pid) == -1))
			break;
		else
		if (i == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			strerr_die2sys(111, FATAL, "waitpid: ");
		}
		if (i != pid) {
			strnum[fmt_ulong(strnum, pid)] = 0;
			strerr_die3x(111, FATAL, strnum, "waitpid surprise");
		}
		if (wait_stopped(wstat) || wait_continued(wstat)) {
			i = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
			strnum[fmt_ulong(strnum, i)] = 0;
			strerr_warn3(WARN,
					wait_stopped(wstat) ?  "child stopped by signal " : "child continued by signal ", strnum, 0);
			continue;
		} else
		if (wait_signaled(wstat)) {
			i = wait_termsig(wstat);
			strnum[fmt_ulong(strnum, i)] = 0;
			strerr_die3x(111, FATAL, "child killed by signal ", strnum);
		} else {
			i = wait_exitcode(wstat);
			strnum[fmt_ulong(strnum, i)] = 0;
			strerr_warn2("child exited with status=", strnum, 0);
		}
	}
	_exit(wait_exitcode(wstat));
}

void
getversion_fghack_c()
{
	const char     *x = "$Id: fghack.c,v 1.6 2022-12-18 12:54:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: fghack.c,v $
 * Revision 1.6  2022-12-18 12:54:03+05:30  Cprogrammer
 * handle wait status with details
 *
 * Revision 1.5  2020-09-16 18:59:01+05:30  Cprogrammer
 * fix compiler warning
 *
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
