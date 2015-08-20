/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif

#include	"auth.h"

static const char rcsid[]="$Id: chain.c,v 1.4 2000/05/14 17:39:10 mrsam Exp $";

void authchain(int argc, char **argv, const char *buf)
{
int	pipes[2];
pid_t	p;
int	l, n;
char	**vec;
char	*prog;

	vec=authcopyargv(argc, argv, &prog);
	close(3);
	if (!prog || open("/dev/null", O_RDONLY) != 3)	authexit(1);

	if (pipe(pipes))
	{
		perror("pipe");
		authexit(1);
	}
	while ((p=fork()) < 0)
	{
		perror("fork");
		sleep(3);
	}
	close(3);

	if (p)
	{
		dup(pipes[0]);
		close(pipes[0]);
		close(pipes[1]);
		execv(prog, vec);
		perror(prog);
		authexit(1);
	}
	l=strlen(buf);
	close(pipes[0]);
	while (l)
	{
		n=write(pipes[1], buf, l);
		if (n <= 0)	break;
		buf += n;
		l -= n;
	}
	close(pipes[1]);
	authexit(1);
}
