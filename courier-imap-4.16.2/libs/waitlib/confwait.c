/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<sys/types.h>

#if	HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#include	<stdio.h>
#include	<signal.h>

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<string.h>
#include	<time.h>

#define	INCLUDED_FROM_CONFIGURE
#include	"waitlib.c"

#define	NUMPROCS	10

static int numterminated=0;


static void cntreaped(pid_t p, int n)
{
	if ( ++numterminated == NUMPROCS )	_exit(0);
}

static RETSIGTYPE childsig(int n)
{
	n=n;

	wait_reap(cntreaped, childsig);

#if	RETSIGTYPE != void
	return (0);
#endif
}

int main()
{
int	pipefd[2];
int	pipefd2[2];
pid_t	p;
int	i;
time_t	t;
char	c;

	if (pipe(pipefd) || pipe(pipefd2))
	{
		perror("pipe");
		exit(1);
	}

	signal(SIGCHLD, childsig);

	for (i=0; i<NUMPROCS; i++)
	{
		while ((p=fork()) == -1)
		{
			perror("fork");
			sleep(5);
		}

		if (p == 0)
		{
			close(pipefd[1]);
			close(pipefd2[0]);
			close(pipefd2[1]);
			read(pipefd[0], &c, 1);
			_exit(0);
		}
	}
	close(pipefd2[1]);
	close(pipefd[0]);
	if (read(pipefd2[0], &c, 1) != 0)
	{
		perror("read");
	}

	wait_block();

	close(pipefd2[0]);
	close(pipefd[1]);
	sleep(3);
	time(&t);
	wait_clear(&childsig);

	do
	{
		sleep(3);
	} while ( time(0) < t+3);
	if (numterminated == NUMPROCS)
		exit(0);
	exit(1);
	return (0);
}
