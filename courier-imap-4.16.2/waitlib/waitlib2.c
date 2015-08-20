/*
** Copyright 2000-2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"waitlib.h"
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>
#include	<signal.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif


static pid_t *static_pid_buf=0;

static void start_reaper(pid_t pid, int exit_stat)
{
}

static RETSIGTYPE start_reap(int signum)
{
	wait_reap(start_reaper, start_reap);

#if	RETSIGTYPE != void
	return (0);
#endif
}

void wait_forchild( void (*)(pid_t, int), /* Reaper */
        RETSIGTYPE (*)(int));   /* Signal handler stub */

int wait_startchildren(unsigned nchildren, pid_t **pidptr)
{
int	pipefd[2];
pid_t	p;
unsigned i;

	if (!pidptr)
	{
		if (static_pid_buf)	free(static_pid_buf);
		static_pid_buf=0;
		pidptr= &static_pid_buf;
	}

	if (*pidptr == 0 && (*pidptr=malloc(nchildren * sizeof(pid_t))) == 0)
		return (-1);

	if (pipe(pipefd) < 0)	return (-1);

	signal(SIGCHLD, start_reap);
	wait_block();
	for (i=0; i<nchildren; i++)
	{
		p=fork();
		if (p < 0)
		{
			while (i)
			{
				kill( (*pidptr)[--i], SIGKILL);
				wait_forchild(start_reaper, start_reap);
			}
			close(pipefd[0]);
			close(pipefd[1]);
			wait_clear(start_reap);
			signal(SIGCHLD, SIG_DFL);
			return (-1);
		}

		if (p == 0)
		{
		char	buf;

			wait_restore();
			close(pipefd[1]);
			if (read(pipefd[0], &buf, 1) != 1)
				exit(1);
			close(pipefd[0]);
			return (1);
		}

		(*pidptr)[i]=p;
	}
	wait_restore();
	close(pipefd[0]);
	for (i=0; i<nchildren; i++)
		if (write(pipefd[1], "", 1) < 0)
			; /* Shut gcc up */
	close(pipefd[1]);
	return (0);
}

int wait_reforkchild(unsigned nchildren, pid_t *pidptr, pid_t pid)
{
unsigned i;

	for (i=0; i<nchildren; i++)
		if (pidptr[i] == pid)	break;

	if (i >= nchildren)	return (0);

	switch ((pidptr[i]=fork()))	{
	case 0:
		wait_restore();	/* Just in case */
		return (1);
	case -1:
		return (-1);
	default:
		break;
	}
	return (0);
}
