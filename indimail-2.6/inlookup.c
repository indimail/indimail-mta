/*
 * $Log: inlookup.c,v $
 * Revision 2.18  2017-12-11 15:50:33+05:30  Cprogrammer
 * added feature to precache active login records
 *
 * Revision 2.17  2011-04-03 17:59:39+05:30  Cprogrammer
 * pass instance number to ProcessInFifo()
 *
 * Revision 2.16  2010-03-28 13:46:04+05:30  Cprogrammer
 * prevent rapid spawning of inlookup children when ProcessInFifo() exits
 *
 * Revision 2.15  2008-11-21 15:08:28+05:30  Cprogrammer
 * fixed compilation on mac os
 *
 * Revision 2.14  2008-11-07 17:01:46+05:30  Cprogrammer
 * use a generic signal handler
 *
 * Revision 2.13  2008-07-13 19:44:44+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.12  2005-12-29 22:41:41+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.11  2003-01-06 20:28:50+05:30  Cprogrammer
 * added facility for reconfiguring on SIGHUP
 *
 * Revision 2.10  2002-12-13 14:35:43+05:30  Cprogrammer
 * added SIGUSR2 to toggle debugging
 *
 * Revision 2.9  2002-12-11 10:27:59+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.8  2002-12-03 03:03:37+05:30  Cprogrammer
 * removed BSD style signal()
 *
 * Revision 2.7  2002-11-18 12:44:53+05:30  Cprogrammer
 * modified signal handler to handle two signals (SIGTERM and SIGUSR)
 *
 * Revision 2.6  2002-10-18 14:55:40+05:30  Cprogrammer
 * added load balancing code
 *
 * Revision 2.5  2002-08-31 16:39:07+05:30  Cprogrammer
 * added Startup Message
 *
 * Revision 2.4  2002-08-02 00:32:00+05:30  Cprogrammer
 * use environment variable to get the default infifo file
 *
 * Revision 2.3  2002-07-07 20:55:05+05:30  Cprogrammer
 * added verbose options and option to specify the default infifo file
 *
 * Revision 2.2  2002-07-06 15:16:06+05:30  Cprogrammer
 * added facility to provide infifo path on command line
 *
 * Revision 2.1  2002-07-05 03:53:40+05:30  Cprogrammer
 * fifo server
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: inlookup.c,v 2.18 2017-12-11 15:50:33+05:30 Cprogrammer Exp mbhangui $";
#endif

#include "indimail.h"
#ifdef CLUSTERED_SITE
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static int      fork_child(char *, int);
#ifdef DARWIN
static void     sig_usr1();
static void     sig_usr2();
static void     sig_hup();
static void     sig_int();
static void     sig_term();
#else
static void     sig_hand(int, int, struct sigcontext *, char *);
static void     sig_catch(int, void(*)());
#endif
static void     sig_block(int);
int             cache_active_pwd(time_t);
extern int      btree_count;

struct pidtab
{
	int pid;
	char infifo[MAX_BUFF];
};
struct pidtab  *pid_table;
static int      inst_count = 0;

int
main(int argc, char **argv)
{
	char            envbuf[MAX_BUFF];
	int             idx, pid, wStat, tmp_stat;
	char           *infifo, *ptr, *instance = "1", *seconds_active = "0";

	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	for (idx = 1; idx < argc; idx++)
	{
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'f':
			infifo = *(argv + idx + 1);
			break;
		case 'i':
			instance = *(argv + idx + 1);
			break;
		case 'c':
			seconds_active = *(argv + idx + 1);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("USAGE: %s [-f infifo] [-i instance] [-c activeSecs] [-v]\n", ptr);
			return(1);
		}
	}
	if ((inst_count = atoi(instance)) > 1)
	{
		if (!pid_table)
		{
			if (!(pid_table = (struct pidtab *) malloc(sizeof(struct pidtab) * inst_count)))
			{
				fprintf(stderr, "inlookup: out of memory: %s\n", strerror(errno));
				return(1);
			}
			for (idx = 0; idx < inst_count; idx++)
				pid_table[idx].pid = -1;
		}
#ifdef DARWIN
		signal(SIGTERM, sig_term);
		signal(SIGUSR1, sig_usr1);
		signal(SIGUSR2, sig_usr2);
		signal(SIGHUP, sig_hup);
		signal(SIGINT, sig_int);
#else
		sig_catch(SIGTERM, sig_hand);
		sig_catch(SIGUSR1, sig_hand);
		sig_catch(SIGUSR2, sig_hand);
		sig_catch(SIGHUP, sig_hand);
		sig_catch(SIGINT, sig_hand);
#endif
		if ((tmp_stat = atoi(seconds_active)) > 0) {
			if ((wStat = cache_active_pwd(tmp_stat)) == -1)
				return (1);
			else
			if (!wStat) {
				printf("cached %d records\n", btree_count);
				fflush(stdout);
			}
		}
		for (idx = 0; idx < inst_count; idx++)
		{
			if (fork_child(infifo, idx) == -1) /*- parent returns */
				return(-1);
		}
		for (;;)
		{
			pid = wait(&wStat);
#ifdef ERESTART
			if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
			if (pid == -1 && errno == EINTR)
#endif
				continue;
			else
			if (pid == -1)
				break;
			if (WIFSTOPPED(wStat) || WIFSIGNALED(wStat))
			{
				for (idx = 0; idx < inst_count; idx++)
				{
					if (pid_table[idx].pid == pid)
					{
						fprintf(stderr, "inlookup[%d]: child [%d] died with signal %d\n", 
							idx + 1, pid, 
							WIFSIGNALED(wStat) ? WTERMSIG(wStat) : (WIFSTOPPED(wStat) ? WSTOPSIG(wStat) : -1));
						if (fork_child(infifo, idx) == -1)
						{
							fprintf(stderr, "Help!! Unable to start process %d\n", idx + 1);
							pid_table[idx].pid = -1;
						}
						break;
					}
				}
				if (idx == inst_count)
					fprintf(stderr, "inlookup: %d crashed. Unable to find slot\n", pid);
			}
			else
			if (WIFEXITED(wStat))
			{
				tmp_stat = WEXITSTATUS(wStat);
				for (idx = 0; idx < inst_count; idx++)
				{
					if (pid_table[idx].pid == pid)
					{
						fprintf(stderr, "inlookup[%d]: child [%d] died with status %d\n", idx + 1, pid, tmp_stat);
						if (fork_child(infifo, idx) == -1)
						{
							fprintf(stderr, "Help!! Unable to start process %d\n", idx + 1);
							pid_table[idx].pid = -1;
						}
						break;
					}
				}
				if (idx == inst_count)
					fprintf(stderr, "inlookup: %d crashed. Unable to find slot\n", pid);
			}
		}
		for (idx = 0; idx < inst_count; idx++)
		{
			if (pid_table[idx].pid == -1)
			{
				if (fork_child(infifo, idx) == -1)
				{
					fprintf(stderr, "inlookup[%d]: Help!! Unable to start process\n", idx + 1);
					pid_table[idx].pid = -1;
				}
			}
		}
		return(-1);
	} else
	{
		snprintf(envbuf, sizeof(envbuf), "INFIFO=%s", infifo);
		putenv(envbuf);
		printf("InLookup INFIFO=%s\n", infifo);
		fflush(stdout);
		return(ProcessInFifo(0));
	}
	return(0);
}

static int
fork_child(char *infifo, int instNum)
{
	int             pid, i;

	switch(pid = fork())
	{
		case -1:
			perror("inlookup: fork");
			return(-1);
		case 0:
#ifdef DARWIN
			signal(SIGTERM, SIG_DFL);
			signal(SIGUSR1, SIG_DFL);
#else
			sig_catch(SIGTERM, SIG_DFL);
			sig_catch(SIGUSR1, SIG_DFL);
#endif
			snprintf(pid_table[instNum].infifo, MAX_BUFF, "INFIFO=%s.%d", infifo, instNum + 1);
			putenv(pid_table[instNum].infifo);
			printf("InLookup[%d] PPID %d PID %d Ready with INFIFO=%s.%d\n", instNum + 1,
				(int) getppid(), (int) getpid(), infifo, instNum + 1);
			fflush(stdout);
			i = ProcessInFifo(instNum + 1);
			sleep(5);
			exit(i);
		default:
			pid_table[instNum].pid = pid;
			snprintf(pid_table[instNum].infifo, MAX_BUFF, "INFIFO=%s.%d", infifo, instNum + 1);
			break;
	}
	return(pid);
}

#ifdef DARWIN
static void
sig_usr1()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGUSR1);
	}
	signal(SIGUSR1, sig_usr1);
	errno = EINTR;
	return;
}

static void
sig_usr2()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGUSR2);
	}
	signal(SIGUSR2, sig_usr2);
	errno = EINTR;
	return;
}

static void
sig_hup()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGHUP);
	}
	signal(SIGHUP, sig_hup);
	errno = EINTR;
	return;
}

static void
sig_int()
{
	int             idx;

	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGINT);
	}
	signal(SIGINT, sig_int);
	errno = EINTR;
	return;
}

static void
sig_term()
{
	int             idx;

	sig_block(SIGTERM);
	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, SIGTERM);
	}
	exit(1);
}
#else
static void
sig_hand(sig, code, scp, addr)
	int             sig, code;
	struct sigcontext *scp;
	char           *addr;
{
	int             idx;

	if (sig == SIGTERM)
		sig_block(sig);
	for (idx = 0; idx < inst_count; idx++)
	{
		if (pid_table[idx].pid == -1)
			continue;
		kill(pid_table[idx].pid, sig);
	}
	if (sig != SIGTERM)
	{
		sig_catch(sig, (void(*)()) sig_hand);
		errno = EINTR;
		return;
	} else
		exit(1);
}

static void
sig_catch(sig, f)
	int             sig;
	void            (*f) ();
{
#ifdef HAVE_SIGACTION
	struct sigaction sa;
	sa.sa_handler = f;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, (struct sigaction *) 0);
#else
	signal(sig, f);	/*- won't work under System V, even nowadays---dorks */
#endif
}
#endif /*- #ifdef DARWIN */

static void
sig_block(sig)
	int             sig;
{
#ifdef HAVE_SIGPROCMASK
	sigset_t        ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	sigprocmask(SIG_BLOCK, &ss, (sigset_t *) 0);
#else
	sigblock(1 << (sig - 1));
#endif
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-user-cluster=y\n");
	return(0);
}
#endif

void
getversion_inlookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
