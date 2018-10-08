/*
** Copyright 2001-2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<pwd.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	"auth.h"
#include	"authmod.h"
#include	"authwait.h"
#include	"authchangepwdir.h"
#include	"numlib/numlib.h"

static const char rcsid[]="$Id: authsyschangepwd.c,v 1.8 2004/04/18 15:54:39 mrsam Exp $";

static int dochangepwd(struct passwd *, const char *, const char *);

int auth_syspasswd(const char *service,	/* Ignored */
		   const char *userid,	/* Generally ignored */
		   const char *oldpwd,	/* Old password */
		   const char *newpwd)	/* New password */
{
	char *cpy=strdup(userid);
	struct passwd *pwd;
	int rc;

	if (!cpy)
	{
		perror("malloc");
		errno=EPERM;
		return (-1);
	}

	if (strchr(cpy, '@'))
	{
		free(cpy);
		errno=EINVAL;
		return (-1);
	}

	pwd=getpwnam(cpy);

	if (!pwd)
	{
		free(cpy);
		errno=EINVAL;
		return (-1);
	}

	rc=dochangepwd(pwd, oldpwd, newpwd);

	free(cpy);
	return (rc);
}

#define EXPECT AUTHCHANGEPWDIR "/authsystem.passwd"

static int dochangepwd(struct passwd *pwd,
		       const char *oldpwd, const char *newpwd)
{
	pid_t p, p2;
	int pipefd[2];
	int waitstat;
	FILE *fp;

	signal(SIGCHLD, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	if (pipe(pipefd) < 0)
	{
		perror("CRIT: authsyschangepwd: pipe() failed");
		errno=EPERM;
		return (-1);
	}

	p=fork();

	if (p < 0)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		perror("CRIT: authsyschangepwd: fork() failed");
		errno=EPERM;
		return (-1);
	}

	if (p == 0)
	{
		char *argv[2];
		char *envp[4];

		close(0);
		dup(pipefd[0]);
		close(pipefd[0]);
		close(pipefd[1]);

		close(1);
		open("/dev/null", O_WRONLY);
		close(2);
		dup(1);

		if (pwd->pw_uid != getuid())
		{
#if HAVE_SETLOGIN
#if HAVE_SETSID
			if (setsid() < 0)
			{
				perror("setsid");
				exit(1);
			}
#endif

			setlogin(pwd->pw_name);
#endif
			libmail_changeuidgid(pwd->pw_uid, pwd->pw_gid);
		}

		argv[0]=EXPECT;
		argv[1]=0;

		envp[0]="PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/usr/local/sbin";
		envp[1]="LC_ALL=C";
		envp[2]="SHELL=/bin/sh";
		envp[3]=0;

		execve(argv[0], argv, envp);
		perror("exec");
		exit(1);
	}

	close(pipefd[0]);
	signal(SIGPIPE, SIG_IGN);

	if ((fp=fdopen(pipefd[1], "w")) == NULL)
	{
		perror("CRIT: authsyschangepwd: fdopen() failed");
		kill(p, SIGTERM);
	}
	else
	{
		fprintf(fp, "%s\n%s\n", oldpwd, newpwd);
		fclose(fp);
	}
	close(pipefd[1]);

	while ((p2=wait(&waitstat)) != p)
	{
		if (p2 < 0 && errno == ECHILD)
		{
			perror("CRIT: authsyschangepwd: wait() failed");
			errno=EPERM;
			return (-1);
		}
	}

	if (WIFEXITED(waitstat) && WEXITSTATUS(waitstat) == 0)
		return (0);
	errno=EPERM;
	return (-1);
}
