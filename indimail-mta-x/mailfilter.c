/*
 * $Log: mailfilter.c,v $
 * Revision 1.1  2021-06-09 19:32:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <env.h>
#include <fmt.h>
#include <wait.h>
#include "mailfilter.h"
#include "qmulti.h"
#include "mktempfile.h"

int
mailfilter(int argc, char **argv, char *filterargs)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2];
	int             wstat, filt_exitcode;

	if (pipe(pipefd) == -1)
		_exit(60);
	switch ((filt_pid = fork()))
	{
	case -1:
		_exit(121);
	case 0: /*- Filter Program */
		/*- Mail content read from fd 0 */
		if (mktempfile(0))
			_exit(68);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(60);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER"))
			_exit(51);
		execl("/bin/sh", "qmailfilter", "-c", filterargs, (char *) 0);
		_exit(75);
	default:
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			close(pipefd[0]);
			wait_pid(&wstat, filt_pid);
			_exit(60);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mktempfile(0)) {
			close(0);
			wait_pid(&wstat, filt_pid);
			_exit(68);
		}
		break;
	}
	/*- Process message if exit code is 0, bounce if 100, else issue temp error */
	if (wait_pid(&wstat, filt_pid) != filt_pid)
		_exit(122);
	if (wait_crashed(wstat))
		_exit(123);
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0:
		return (qmulti(0, argc, argv));
	case 2:
		return (0); /*- Blackhole */
	case 88: /*- exit with custom error code with error code string from stderr */
		_exit(88);
	case 100:
		_exit(31);
	default:
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		_exit(71);
	}
	/*- Not reached */
	return (0);
}

#ifndef	lint
void
getversion_mailfilter_c()
{
	static char    *x = "$Id: mailfilter.c,v 1.1 2021-06-09 19:32:41+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmailfilterh;
	x = sccsidmktempfileh;
	x = sccsidqmultih;
	x++;
}
#endif
