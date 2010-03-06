/*
 * $Log: pipe_exec.c,v $
 * Revision 2.2  2002-09-02 22:04:37+05:30  Cprogrammer
 * added SIGPIPE handling
 *
 * Revision 2.1  2002-09-01 20:52:53+05:30  Cprogrammer
 * function for executing smtp auth programs
 *
 */

#ifndef lint
static char     sccsid[] = "$Id: pipe_exec.c,v 2.2 2002-09-02 22:04:37+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int
pipe_exec(char **argv, char *tmpbuf, int len)
{
	int             pipe_fd[2];
	void            (*pstat) ();

	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
	{
		fprintf(stderr, "pipe_exec: signal: %s\n", strerror(errno));
		return (-1);
	}
	if (pipe(pipe_fd) == -1)
	{
		fprintf(stderr, "pipe_exec: pipe: %s\n", strerror(errno));
		signal(SIGPIPE, pstat);
		return(-1);
	}
	if (dup2(pipe_fd[0], 3) == -1 || dup2(pipe_fd[1], 4) == -1)
	{
		fprintf(stderr, "pipe_exec: dup2: %s\n", strerror(errno));
		signal(SIGPIPE, pstat);
		return(-1);
	}
	if (pipe_fd[0] != 3 && pipe_fd[0] != 4)
		close(pipe_fd[0]);
	if (pipe_fd[1] != 3 && pipe_fd[1] != 4)
		close(pipe_fd[1]);
	if (write(4, tmpbuf, len) != len)
	{
		fprintf(stderr, "pipe_exec: %s: %s\n", argv[1], strerror(errno));
		signal(SIGPIPE, pstat);
		return(-1);
	}
	close(4);
	signal(SIGPIPE, pstat);
	execvp(argv[1], argv + 1);
	fprintf(stderr, "pipe_exec: %s: %s\n", argv[1], strerror(errno));
	return(-1);
}

void
getversion_pipe_exec_c()
{
	printf("%s\n", sccsid);
}
