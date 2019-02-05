/*
** Copyright 2000-2007 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"liblock.h"
#include	<stdio.h>
#include	<signal.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<errno.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<sys/types.h>
#include	"../numlib/numlib.h"
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#if	HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif

#define exit(_a_) _exit(_a_)


static int start1(const char *, int);

#define CONSOLE "/dev/null"

int ll_daemon_start(const char *lockfile)
{
pid_t	p;
int	pipefd[2];
char	c;
int	i;

	/*
	** Close any open file descriptors.
	*/

	for (i=3; i < 256; i++)
		close(i);

	/*
	** We fork, and set up a pipe from the child process.  If we read
	** a single 0 byte from the pipe, it means that the child has
	** successfully initialized, and will return to main, so we exit(0).
	** If we do not read a single 0 byte from the pipe, it means that
	** there was an initialization error, so we return -1 to main.
	*/

	if (pipe(pipefd) < 0)
	{
		perror("pipe");
		return (-1);
	}

	if ((p=fork()) == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		perror("fork");
		return (-1);
	}

	if (p == 0)
	{
		close(pipefd[0]);

		/*
		** We fork once more, so that the daemon process will not
		** be the child process of anyone.
		*/

		p=fork();
		if (p == -1)
		{
			perror("fork");
			exit(0);
		}
		if (p)
			exit(0);

		/*
		** Continue initialization in start1()
		*/
		return (start1(lockfile, pipefd[1]));
	}

	close(pipefd[1]);
	if (read(pipefd[0], &c, 1) <= 0)
		c=1;
	close(pipefd[0]);
	if (c == 0)
		exit (0);	/* Successful start of daemon */
	errno=EAGAIN;
	return (-1);
}

static int start1(const char *lockfile, int fd)
{
int	lockfd, maxfd;

#if     HAVE_SETPGRP
#if     SETPGRP_VOID
	setpgrp();
#else
	setpgrp(0, 0);
#endif
#endif
#ifdef  TIOCNOTTY

	{
	int fd=open("/dev/tty", O_RDWR);

		if (fd >= 0)
		{
			ioctl(fd, TIOCNOTTY, 0);
			close(fd);
		}
	}
#endif


	/* Attempt to obtain a lock */

	lockfd=open(lockfile, O_RDWR|O_CREAT, 0600);

	if (lockfd < 0)
	{
		/* Perhaps an upgraded daemon runs under new uid? */

		unlink(lockfile);
		lockfd=open(lockfile, O_RDWR|O_CREAT, 0600);
	}

#if HAVE_GETDTABLESIZE
	maxfd=getdtablesize()-1;
#elif defined(OPEN_MAX)
	maxfd=OPEN_MAX-1;
#elif HAVE_SYSCONF && defined(_SC_OPEN_MAX)
	if ((maxfd=sysconf(_SC_OPEN_MAX)) < 0)
		maxfd=63;
	else if (maxfd > 0)
		maxfd--;
#else
	maxfd=63;
#endif

	if (lockfd < 0 || dup2(lockfd, maxfd) != maxfd)
	{
		perror(lockfile);
		exit(1);
	}

	close(lockfd);
	lockfd=maxfd;

#ifdef	FD_CLOEXEC
	if (fcntl(lockfd, F_SETFD, FD_CLOEXEC) < 0)
	{
		perror("fcntl");
		close(lockfd);
		exit(1);
	}
#endif

	if (ll_lock_ex_test(lockfd))
	{
		if (write(fd, "", 1) != 1)
			exit(1); /* Shouldn't happen */

		close(fd);
		exit (0);	/* Already running, pretend success */
	}

	/*
	** Return >0 to main, so it can continue main's setup.
	*/

	return (fd);
}

int ll_daemon_resetio()
{
int	i;

	close(0);
	if (open("/dev/null", O_RDONLY) != 0)
		return (-1);

	close(1);
	i=open(CONSOLE, O_WRONLY);
	if (i < 0)	i=open("/dev/null", O_WRONLY);
	if (i != 1)	return (-1);

	close(2);
	i=open(CONSOLE, O_WRONLY);
	if (i < 0)	i=open("/dev/null", O_WRONLY);
	if (i != 2)	return (-1);
	return (0);
}

void ll_daemon_started(const char *pidfile, int fd)
{
char	buf[NUMBUFSIZE+1];
char	*p=strcat(libmail_str_pid_t(getpid(), buf), "\n");
FILE	*fp;

	unlink(pidfile); 
	if ((fp=fopen(pidfile, "w")) == NULL ||
		fprintf(fp, "%s", p) < 0 || fflush(fp) < 0 || fclose(fp))
	{
		perror(pidfile);
		exit(1);
	}

	if (write(fd, "", 1) != 1)	/* Signal waiting parent */
		exit(1); /* Shouldn't happen */
	close(fd);
}

static void stop1(const char *, const char *);

int ll_daemon_stop(const char *lockfile, const char *pidfile)
{
pid_t	p, p2;
int	waitstat;

	/*
	** We fork, and the child process attempts to stop the daemon,
	** then communicates the success to us, via its exit code.
	*/

	signal(SIGCHLD, SIG_DFL);
	if ((p=fork()) == -1)
	{
		perror("fork");
		return (1);
	}
	if (p == 0)	stop1(lockfile, pidfile);

	while ((p2=wait(&waitstat)) != p)
		;

	if (WIFEXITED(waitstat))
		return (WEXITSTATUS(waitstat));
	return (0);
}

/*
**	The child process forks too.  The parent process goes in a loop,
**	trying to kill the daemon process.
**
**	The child process attempts to lock the lock file.  When it
**	succeeds, it exits.  When the child process exits, the parent
**	process kills itself.
*/

static RETSIGTYPE sigexit(int signum)
{
	kill(getpid(), SIGKILL);
#if     RETSIGTYPE != void
	return (0);
#endif
}

static void stop1(const char *lockfile, const char *pidfile)
{
int	lockfd;
pid_t	p;

	if ((lockfd=open(lockfile, O_RDWR|O_CREAT, 0600)) < 0)
	{
		perror(lockfile);
		exit(1);
	}

	if ( ll_lock_ex_test(lockfd) == 0)	/* No daemon process running */
	{
		close(lockfd);
		exit (0);	/* That was easy! */
	}

	signal(SIGCHLD, sigexit);

	if ((p=fork()) == -1)
	{
		perror("fork");
		exit(1);
	}

	if (p)	/* Parent - first sends a SIGTERM, then a SIGKILL */
	{
	int	signum=SIGTERM;

		close(lockfd);
		for (;; sleep(10))
		{
		FILE	*fp;
		int	c;

			if ((fp=fopen(pidfile, "r")) == NULL)
				continue;

			p=0;

			while ((c=getc(fp)) != EOF && c != '\n')
			{
				if (isdigit(c))
					p=p*10+(c-'0');
			}

			fclose(fp);
			if (p)
				kill(p, signum);
			signum=SIGKILL;
		}
	}

	if (ll_lock_ex(lockfd))
		perror("lock");
	close(lockfd);
	exit(0);
}

int ll_daemon_restart(const char *lockfile, const char *pidfile)
{
int	lockfd;
pid_t	p;
FILE	*fp;
int	c;

	if ((lockfd=open(lockfile, O_RDWR|O_CREAT, 0600)) < 0)
	{
		perror(lockfile);
		return (1);
	}

	if ( ll_lock_ex_test(lockfd) == 0)	/* No daemon process running */
	{
		close(lockfd);
		return (0);	/* That was easy! */
	}
	close(lockfd);

	if ((fp=fopen(pidfile, "r")) == NULL)
		return (0);

	p=0;

	while ((c=getc(fp)) != EOF && c != '\n')
	{
		if (isdigit(c))
			p=p*10+(c-'0');
	}

	fclose(fp);
	if (p)
		kill(p, SIGHUP);
	return (0);
}

