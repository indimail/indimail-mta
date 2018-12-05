/*
** Copyright 1998 - 2014 Double Precision, Inc.  See COPYING for
** distribution information.
*/

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
#include	<string.h>
#include	<stdio.h>

int main()
{
#define FILENAME	"courier-imap.locktest.XXXXXXXXXX"
int	fd[2];
pid_t	p;
int	s;
int	f;

	char *name;
	const char *tmpdir;
	if ((tmpdir = (char *)getenv("TMPDIR")) == NULL || !*tmpdir)
		tmpdir = "/tmp";

	if ((name=malloc(strlen(tmpdir)+sizeof(FILENAME)+1)) == NULL)
	{
		perror("get filename");
		exit(1);
	}

	(void)sprintf(name, "%s/%s", tmpdir, FILENAME);

	signal(SIGCHLD, SIG_DFL);
	if (pipe(fd))
	{
		perror("pipe");
		return (1);
	}

	if ((f=mkstemp(name)) < 0)
	{
		perror("open");
		exit(1);
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

		if ((f=open(name, O_RDWR)) < 0)
		{
			perror("open");
			exit(1);
		}
		alarm(5);

		if (ll_lockfd(f, ll_writelock, 0, 0))
		{
			close(f);
			unlink(name);
			exit(0);
		}
		close(f);
		exit(1);
	}

	if (ll_lockfd(f, ll_writelock, 0, 0))
	{
		perror("lock");
		close(f);
		unlink(name);
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
