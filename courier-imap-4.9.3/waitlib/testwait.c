/*
** Copyright 1998 - 2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"waitlib.h"
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<signal.h>

/* Stress test waitlib.c */

#define	NUMCHILDREN	100		/* Start 100 child processes */
#define	INITCHILDREN	10		/* Start with these many child procs */

static unsigned started, finished;

static void reap_child(pid_t p, int dummy)
{
	++finished;
}

static RETSIGTYPE sighandler(int sig)
{
	wait_reap(&reap_child, &sighandler);
#if	RETSIGTYPE != void
	return (0);
#endif
}

static pid_t start_child()
{
pid_t	p;

	wait_block();
	while ((p=fork()) == (pid_t)-1)
	{
		perror("fork");
		sleep(3);
	}
	++started;
	if (p == 0)
	{
		wait_restore();
	}
	else
		wait_clear(&sighandler);
	return (p);
}

extern void foobar();

int	main()
{
int	pipefd[2];
int	pipefd2[2];
char	c;

	if (pipe(pipefd) || pipe(pipefd2))
	{
		perror("pipe");
		exit(1);
	}

	signal(SIGCHLD, sighandler);

	started=finished=0;
	while (started < INITCHILDREN)
	{
		if (start_child() == 0)
		{
			close(pipefd2[0]);
			close(pipefd2[1]);
			close(pipefd[1]);
			if (read(pipefd[0], &c, 1) != 1)
				; /* Shut gcc up */
			close(pipefd[0]);
			_exit(0);
		}
	}
	close(pipefd2[1]);
	close(pipefd[0]);
	if (read(pipefd2[0], &c, 1) != 1)
		; /* Shut gcc up */
	close(pipefd[1]);
	close(pipefd2[0]);

	while (started < NUMCHILDREN)
		if (start_child() == 0)
			_exit(0);

	alarm(30);
	while (finished < started)
		foobar();
	exit(0);
}
