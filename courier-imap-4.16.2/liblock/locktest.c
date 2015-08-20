/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/* $Id */

#include	"liblock.h"
#if	USE_FCNTL
#include	"lockfcntl.c"
#endif
#if	USE_FLOCK
#include	"lockflock.c"
#endif
#if	USE_LOCKF
#include	"locklockf.c"
#endif
#include	<signal.h>
#include	<stdlib.h>

int main()
{
int	fd[2];
pid_t	p;
int	s;
int	f;

	signal(SIGCHLD, SIG_DFL);
	if (pipe(fd))
	{
		perror("pipe");
		return (1);
	}

	if ((p=fork()) == (pid_t)-1)
	{
		perror("fork");
		return (1);
	}

	if (p == 0)
	{
	char	c;

		close(fd[1]);
		read(fd[0], &c, 1);
		close(fd[0]);

		if ((f=open("conftest.lock", O_RDWR|O_CREAT, 0644)) < 0)
		{
			perror("open");
			exit(1);
		}
		alarm(5);

		if (ll_lockfd(f, ll_writelock, 0, 0))
		{
			close(f);
			exit(0);
		}
		close(f);
		exit(1);
	}
	
	if ((f=open("conftest.lock", O_RDWR|O_CREAT, 0644)) < 0)
	{
		perror("open");
		exit(1);
	}

	if (ll_lockfd(f, ll_writelock, 0, 0))
	{
		perror("lock");
		close(f);
		exit(1);
	}
	close(fd[1]);
	close(fd[0]);
	while (wait(&s) != p)
		;
	if (s == 0)
		exit(0);
	exit(1);
}
