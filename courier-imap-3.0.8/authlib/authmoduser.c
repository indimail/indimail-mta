/*
** Copyright 1998 - 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authwait.h"
#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<signal.h>

static const char rcsid[]="$Id: authmoduser.c,v 1.6 2001/11/29 02:57:15 mrsam Exp $";

void authmod(int argc, char **argv,
	const char *service,
	const char *authtype,
	const char *authdata)
{
int	pipe3fd[2];
pid_t	pid;
char	*buf;
int	waitstat;
char	*p;
int	l;

	signal(SIGCHLD, SIG_DFL);

	while (wait(&waitstat) >= 0)
		;
	alarm(0);
	close(3);
	while (open("/dev/null", O_RDWR) != 3)
		;

	if (pipe(pipe3fd))
	{
		perror("pipe");
		authexit(1);
	}

	while ((pid=fork()) == -1)
	{
		sleep(5);
	}

	if (pid)
	{
	char	*prog;
	char	**argvec=authcopyargv(argc, argv, &prog);

		if (!prog)	authexit(1);
		close(3);
		dup(pipe3fd[0]);
		close(pipe3fd[0]);
		close(pipe3fd[1]);
		execv(prog, argvec);
		perror(prog);
		authexit(1);
	}
	close(3);
	close(pipe3fd[0]);

	buf=malloc(strlen(service)+strlen(authtype)+strlen(authdata)+4);
	if (!buf)
	{
		perror("malloc");
		authexit(1);
	}
	sprintf(buf, "%s\n%s\n%s\n", service, authtype, authdata);

	p=buf;
	l=strlen(p);
	while (l)
	{
	int	n=write(pipe3fd[1], p, l);

		if (n <= 0)	break;
		p += n;
		l -= n;
	}
	free(buf);
	close(pipe3fd[0]);
	close(pipe3fd[1]);
	authexit(1);
}
