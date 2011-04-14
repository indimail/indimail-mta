/*
** Copyright 1998 - 2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<time.h>
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/wait.h>

#define	MD5_INTERNAL
#include	"md5/md5.h"

#include	"random128.h"


const char *random128()
{
static char randombuf[sizeof(MD5_DIGEST)*2+1];

#ifdef	RANDOM
	{
	int	fd=open(RANDOM, O_RDONLY);
	char	buf2[sizeof(MD5_DIGEST)];
	int	i;

		if (fd >= 0)
		{
			if (read(fd, buf2, sizeof(buf2)) == sizeof(buf2))
			{
				for (i=0; i<sizeof(buf2); i++)
					sprintf(randombuf+i*2,
						"%02X",
						(int)(unsigned char)buf2[i]);
				close(fd);
				return (randombuf);
			}
			close(fd);
		}
	}
#endif

	/* /dev/urandom not available or broken?  Create some noise */

	{
	int pipefd[2];
	int s;
	char	buf[512];
	struct MD5_CONTEXT context;
	MD5_DIGEST	digest;
	int	n;
	time_t	t;
	pid_t	p, p2;
	unsigned long l;

		time(&t);
		p=getpid();

		if (pipe(pipefd))	return (0);
		while ((p=fork()) == -1)
		{
			sleep (5);
		}
		if (p == 0)
		{
			dup2(pipefd[1], 1);
			dup2(pipefd[1], 2);
			close(pipefd[0]);
			close(pipefd[1]);

#ifdef	W
			while ((p=fork()) == -1)
			{
				sleep (5);
			}
			if (p == 0)
			{
				execl(W, W, (char *)0);
				perror(W);
				_exit(0);
			}
			while (wait(&s) >= 0)
				;
#endif

			execl(PS, PS, PS_OPTIONS, (char *)0);
			perror(PS);
			_exit(0);
		}
		close(pipefd[1]);
		md5_context_init(&context);
		md5_context_hashstream(&context, &t, sizeof(t));
		md5_context_hashstream(&context, &p, sizeof(p));
		l=sizeof(t)+sizeof(p);

		while ((n=read(pipefd[0], buf, sizeof(buf))) > 0)
		{
			md5_context_hashstream(&context, buf, n);
			l += n;
		}
		md5_context_endstream(&context, l);
		md5_context_digest(&context, digest);
		close(pipefd[0]);
		while ((p2=wait(&s)) >= 0 && p != p2)
			;

		for (n=0; n<sizeof(digest); n++)
			sprintf(randombuf+n*2,
				"%02X", (int)(unsigned char)digest[n]);
	}

	return (randombuf);
}
