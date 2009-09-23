/*
 * $Log: runcmmd.c,v $
 * Revision 2.5  2009-09-23 14:57:02+05:30  Cprogrammer
 * added option to use execvp()
 *
 * Revision 2.4  2008-11-20 22:12:29+05:30  Cprogrammer
 * return status for the forked child only
 *
 * Revision 2.3  2008-07-13 19:46:58+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.2  2007-12-22 00:32:35+05:30  Cprogrammer
 * display error if execv() fails
 *
 * Revision 2.1  2004-09-04 00:01:44+05:30  Cprogrammer
 * system() command
 *
 */
#include "indimail.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#ifndef lint
static char     sccsid[] = "$Id: runcmmd.c,v 2.5 2009-09-23 14:57:02+05:30 Cprogrammer Stab mbhangui $";
#endif

int
runcmmd(char *cmmd, int useP)
{
	char          **argv;
	int             status, i, retval;
	pid_t           pid;
	void            (*pstat[2]) ();

	switch ((pid = fork()))
	{
	case -1:
		exit(1);
	case 0:
		if (!(argv = MakeArgs(cmmd)))
			exit(1);
		if (useP)
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		perror(*argv);
		exit(1);
	default:
		break;
	}
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	for (retval = -1;;)
	{
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (i != pid)
			continue;
		if (WIFSTOPPED(status) || WIFSIGNALED(status))
		{
			if(verbose)
				filewrt(2, "%d: killed by signal %d\n", getpid(), WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status));
			retval = -1;
		} else
		if (WIFEXITED(status))
		{
			retval = WEXITSTATUS(status);
			if(verbose)
				filewrt(2, "%d: normal exit return status %d\n", getpid(), retval);
		}
		break;
	}
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	return (retval);
}

void
getversion_runcmmd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
