/*
** Copyright 2002-2006 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"liblock.h"
#include	"mail.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<errno.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if HAVE_SYSEXITS_H
#include <sysexits.h>
#endif

#ifndef EX_TEMPFAIL
#define EX_TEMPFAIL 75
#endif


#define NTRIES_DEFAULT	60
#define DELAY	5

static int sig=0;

static int catch(int signum)
{
	sig=1;
	signal(SIGHUP, (RETSIGTYPE (*)(int))catch);
	signal(SIGTERM, (RETSIGTYPE (*)(int))catch);
	signal(SIGINT, (RETSIGTYPE (*)(int))catch);
	return 0;
}

static int caught()
{
	signal(SIGHUP, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	return sig;
}

int main(int argc, char **argv)
{
	char **argvec;
	int n;
	int fd;
	pid_t pid, pid2;
	int waitstat;
	struct ll_mail *p;
	int ntries=NTRIES_DEFAULT;
	int readonly=0;
	int optchar;

	while ((optchar=getopt(argc, argv, "+rt:")) != -1)
		switch (optchar) {
		case 'r':
			readonly=1;
			break;
		case 't':
			ntries=atoi(optarg);
			if (ntries < 0)
			{
				fprintf(stderr, "%s: invalid argument to -t\n",
					argv[0]);
				exit(EX_TEMPFAIL);
			}
			ntries= (ntries / DELAY) + 1;
			break;
		default:
			exit(1);
		}

	if (argc - optind < 2)
	{
		fprintf(stderr, "Usage: %s [-r] [-t time] mailfile program [args]...\n",
			argv[0]);
		exit(1);
	}

	if ((argvec=malloc(sizeof(char *) * (argc - optind + 1))) == NULL)
	{
		perror("malloc");
		exit (1);
	}

	for (n=optind+1; n<argc; n++)
		argvec[n-optind - 1]=argv[n];

	argvec[n-optind-1]=NULL;

	/* Create the mail file, if it doesn't exist */

	if ((n=open(argv[optind], O_RDWR|O_CREAT, 0600)) >= 0)
		close(n);

	signal(SIGUSR2, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);

	p=NULL;

	for (n=0; n<ntries; sleep(DELAY), n++)
	{
		if (p)
			ll_mail_free(p);

		if ((p=ll_mail_alloc(argv[optind])) == NULL)
		{
			perror("malloc");
			exit(1);
		}

		signal(SIGHUP, (RETSIGTYPE (*)(int))catch);
		signal(SIGTERM, (RETSIGTYPE (*)(int))catch);
		signal(SIGINT, (RETSIGTYPE (*)(int))catch);

		if (ll_mail_lock(p) < 0)
		{
			if (errno == ENOENT)
				break; /* Mail file gone? */
			if (caught())
				break;
			continue;
		}

		if ((fd=ll_mail_open(p)) < 0)
		{
			if (!readonly || (fd=ll_mail_open_ro(p)) < 0)
			{
				if (caught())
					break;
				continue;
			}
		}

		if ((pid=fork()) < 0)
		{
			perror("fork");
			exit(1);
		}

		if (pid == 0)
		{
			if (setgid(getgid()) < 0 ||
			    setuid(getuid()) < 0)
			{
				perror("setuid/setgid");
				exit(1);
			}

			(void)caught();
			execvp(argvec[0], argvec);

			perror(argvec[0]);
			exit(1);
		}

		while ((pid2=wait(&waitstat)) != pid)
			;

		ll_mail_free(p);

		if (WIFEXITED(waitstat))
			exit(WEXITSTATUS(waitstat));
		exit(EX_TEMPFAIL);
	}
	if (p)
		ll_mail_free(p);
	exit(EX_TEMPFAIL);
	return (0);
}

