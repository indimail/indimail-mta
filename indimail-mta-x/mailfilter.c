/*
 * $Id: mailfilter.c,v 1.5 2024-02-20 22:18:10+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <env.h>
#include <fmt.h>
#include <wait.h>
#include <mktempfile.h>
#include "mailfilter.h"
#include "qmulti.h"
#include "qmail.h"

int
mailfilter(int argc, char **argv, char *filterargs)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2];
	int             wstat, filt_exitcode;

	if (pipe(pipefd) == -1)
		_exit(QQ_PIPE_SOCKET);
	switch ((filt_pid = fork()))
	{
	case -1:
		_exit(QQ_FORK_ERR);
	case 0: /*- Filter Program */
		/*- Mail content read from fd 0 */
		if (mktempfile(0))
			_exit(QQ_TMP_FILES);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(QQ_PIPE_SOCKET);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER"))
			_exit(QQ_OUT_OF_MEMORY);
		execl("/bin/sh", "qmailfilter", "-c", filterargs, (char *) 0);
		_exit(QQ_EXEC_FAILED);
	default:
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			close(pipefd[0]);
			wait_pid(&wstat, filt_pid);
			_exit(QQ_PIPE_SOCKET);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mktempfile(0)) {
			close(0);
			wait_pid(&wstat, filt_pid);
			_exit(QQ_TMP_FILES);
		}
		break;
	}
	/*- Process message if exit code is 0, bounce if 100, else issue temp error */
	if (wait_pid(&wstat, filt_pid) != filt_pid)
		_exit(QQ_WAITPID_SURPRISE);
	if (wait_crashed(wstat))
		_exit(QQ_CRASHED);
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0:
		return (qmulti(0, argc, argv));
	case 2:
	case 99: /*- compatability with qmail-qfilter */
		return (0); /*- Blackhole */
	case 88: /*- exit with custom error code with error code string from stderr */
		_exit(QQ_CUSTOM_ERR);
	case QQ_PERM_MSG_REJECT:
	case 100:
		_exit(QQ_PERM_MSG_REJECT);
	default:
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		_exit(QQ_TEMP_MSG_REJECT);
	}
	/*- Not reached */
	return (QQ_OK);
}

#ifndef	lint
void
getversion_mailfilter_c()
{
	static char    *x = "$Id: mailfilter.c,v 1.5 2024-02-20 22:18:10+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmailfilterh;
	x = sccsidmktempfileh;
	x = sccsidqmultih;
	x++;
}
#endif

/*
 * $Log: mailfilter.c,v $
 * Revision 1.5  2024-02-20 22:18:10+05:30  Cprogrammer
 * added exit code 99 for compatibility with qmail-qfilter blackhole
 *
 * Revision 1.4  2024-02-19 22:45:39+05:30  Cprogrammer
 * return permanent rejection for both 100 and QQ_PERM_MSG_REJECT codes
 *
 * Revision 1.3  2023-10-27 16:11:14+05:30  Cprogrammer
 * replace hard-coded exit values with constants from qmail.h
 *
 * Revision 1.2  2022-10-17 19:43:40+05:30  Cprogrammer
 * use exit codes defines from qmail.h
 *
 * Revision 1.1  2021-06-09 19:32:41+05:30  Cprogrammer
 * Initial revision
 *
 */
