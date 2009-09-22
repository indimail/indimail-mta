/*
 * $Log: exec.c,v $
 * Revision 1.4  2008-07-17 21:37:14+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.3  2008-06-09 15:30:28+05:30  Cprogrammer
 * added GPL copyright
 *
 * Revision 1.2  2002-12-21 19:08:02+05:30  Manny
 * corrected compilation warnings
 *
 * Revision 1.1  2002-12-16 01:54:58+05:30  Manny
 * Initial revision
 *
 *
 * Handles the exec functions. Does fg / bg process group stuff.
 *
 * Copyright (C) 1996  Stephen Fegan
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.

 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * please send patches or advice to: `flash@netsoc.ucd.ie'
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <syslog.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "exec.h"
#include "misc.h"
#include "screen.h"
#include "module.h"
#include "menu.h"
#include "set.h"
#include "rc.h"
#include "mystring.h"
#include "parseline.h"
#include "variables.h"

#ifdef sun
#define sys_siglist _sys_siglistp
#endif

void            pressanykey(void);

char          **pagerv = NULL;
int             pagerc = 0;

struct job_q    JH, *JobHead;

sigjmp_buf      jmpbuf;
volatile sig_atomic_t canjump = 0;

void
display_file(const char *filen)
{
	int             argc;
	char           *argv[128];
	char           *cmd_line;

	clear_close_scr();
	cmd_line = xmalloc(strlen(PAGER) + strlen(filen) + 2);
	sprintf(cmd_line, "%s %s", PAGER, filen);
	if (prep_for_exec(cmd_line, &argc, argv, 127) > 0)
		do_exec(cmd_line, argc, argv, EXEC_CLEARUP | EXEC_NOPAUSE);
	free(cmd_line);
	init_scr();
}

/*
 * Interface to screen module
 * Execute menu item
 * item == item to execute
 * *menu == menu list ptr, for use with MENU_SUBs 
 */
void
exec_item(struct menu_items *item)
{
	char            tmpargs[4096], exec[255];
	char           *args;
	char           *argv[512];
	int             argc;
	struct menu    *menu;

	/*
	 * Overload these default cases if you wish - look at screens/nc_menu.c 
	 */
	switch (item->type)
	{
	case MENU_ARGS:
		if (get_args(tmpargs, 4096, item->prompt) == 0)
		{
			args = expand(tmpargs);
			if ((*item->args != '\0') && (*(item->args + strlen(item->args) - 1) == '/'))
			{
				char           *l = args;
				while ((*l != '\0') && (*l != '/') && (!isspace((int) *l)))
					l++;
				if (*l == '/')
				{
					prep_for_exec(args, &argc, argv, 511);
					clear_close_scr();
					fprintf(stderr, "%s: '/' in command name !\n", *argv);
					pressanykey();
					init_scr();
					return;
				} else
					sprintf(exec, "%s%s", item->args, args);
			} else
				sprintf(exec, "%s %s", item->args, args);
			clear_close_scr();
			if (prep_for_exec(exec, &argc, argv, 511) > 0)
				do_exec(exec, argc, argv, EXEC_CLEARUP | (item->flags & MIF_EXEC_MASK));
			init_scr();
		}
		break;
	case MENU_EXEC:
		clear_close_scr();
		if (prep_for_exec(item->args, &argc, argv, 511) > 0)
			do_exec(item->args, argc, argv, EXEC_CLEARUP | (item->flags & MIF_EXEC_MASK));
		init_scr();
		break;
	case MENU_MODULE:
		if (prep_for_exec(item->args, &argc, argv, 511) > 0)
			RunModuleFunction(argc, argv);
		while (argc--)
		{
			free((void *) *(argv + argc));
			*(argv + argc) = NULL;
		}
		break;
	case MENU_SUB:
		menu = find_menu(item->args);
		if (menu != NULL)
			do_menu(menu);
		else
			fprintf(stderr, "No menu named %s\n", item->name);
		break;
	case MENU_TITLE:
	case MENU_NOP:
		clear_close_scr();
		fprintf(stderr, "Cannot Select This Type !!\n");
		sleep(2);
		init_scr();
		break;
	case MENU_QUIT:
		clear_close_scr();
		fprintf(stderr, "Error: QUIT menu selected in exec_item !!\n");
		sleep(2);
		init_scr();
		break;
	case MENU_EXIT:
		clear_close_scr();
		exit(0);
		break;
	default:
		clear_close_scr();
		fprintf(stderr, "Unrecognised Item !!\n");
		sleep(2);
		init_scr();
		break;
	}
}

int
prep_for_exec(char *exec_line, int *argc, char **argv, int maxargc)
{
	int             ret;

	ret = parseline(exec_line, argc, argv, maxargc);
	if (ret >= 0)
		*(argv + *argc) = NULL;
	return ret;
}

void
init_jobq(void)
{
	JobHead = &JH;

	JH.rst = S_EXIT;
	JH.signal = 0;
	JH.exit = 0;
	JH.change = 0;
	JH.cstat = 0;
	JH.eflags = EXEC_NONE;
	JH.jflags = JOBF_NONE;
	JH.pgrp = -1;
	JH.waiton = -1;
	JH.start = 0;
	JH.cmdline = NULL;
	JH.ttystate = NULL;
	JH.origstate = NULL;
	JH.next = JobHead;
	JH.prev = JobHead;
}

void
finish_jobq(void)
{
	sigset_t        omask, bmask;
	struct job_q   *j, *jn;
	struct tm      *tm;
	int             foundrunning = 0;

	sigemptyset(&omask);
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &bmask, &omask);
	for (j = JobHead->next; j != JobHead; j = jn)
	{
		jn = j->next;
		tm = localtime(&j->start);
		fprintf(stdout, "%d/%d %d:%2.2d%2s  [%d] %s  (%s)\n", tm->tm_mday, tm->tm_mon + 1, ((tm->tm_hour + 11) % 12) + 1,
				tm->tm_min, tm->tm_hour > 11 ? "pm" : "am", (int) j->pgrp, j->cmdline, job_status(j));
		if ((j->rst == S_RUN) || (j->rst == S_STOP))
		{
			kill_job(j);
			foundrunning = 1;
		} else
			remove_job(j);
	}
	fflush(stdout);
	sigprocmask(SIG_SETMASK, &omask, NULL);
	if (foundrunning == 1)
	{
		char            message[] = "\n\nWaiting for processes to end\n\n";

		sigemptyset(&omask);
		sigemptyset(&bmask);
		sigaddset(&bmask, SIGCHLD);
		sigprocmask(SIG_BLOCK, &bmask, &omask);
		write(2, message, strlen(message));
		sleep(2);
		for (j = JobHead->next; j != JobHead; j = jn)
		{
			jn = j->next;
			if ((j->rst == S_RUN) || (j->rst == S_STOP))
				kill_job(j);
			remove_job(j);
		}
		sigprocmask(SIG_SETMASK, &omask, NULL);
	}
}

struct job_q   *
add_job(char *cmd, pid_t pgrp, pid_t waiton, int eflags, struct termios *ttystate)
{
	struct job_q   *j;
	char           *seek;

	j = xmalloc(sizeof(*j));
	j->rst = S_RUN;
	j->signal = 0;
	j->exit = 0;
	j->change = 0;
	j->cstat = 0;
	j->eflags = eflags;
	j->jflags = JOBF_NONE;
	j->pgrp = pgrp;
	j->waiton = waiton;
	time(&j->start);
	j->cmdline = xmalloc(61 * sizeof(char));
	if (ttystate != NULL)
	{
		j->ttystate = xmalloc(sizeof(*j->ttystate));
		memcpy(j->ttystate, ttystate, sizeof(*ttystate));
		j->origstate = xmalloc(sizeof(*j->origstate));
		memcpy(j->origstate, ttystate, sizeof(*ttystate));
	} else
		j->ttystate = j->origstate = NULL;
	*(j->cmdline + 60) = '\0';
	if (cmd == NULL)
		strcpy(j->cmdline, "No Name");
	else
	{
		while ((*cmd == ' ') || (*cmd == '\t'))
			cmd++;
		seek = strchr(cmd, ' ');
		if (seek == NULL)
			seek = cmd;
		else
			while ((seek > cmd) && (*seek != '/'))
				seek--;
		if (*seek == '/')
			seek++;

		strncpy(j->cmdline, seek, 60);
	}
	for (seek = j->cmdline + strlen(j->cmdline) - 1; ((*seek == ' ') || (*seek == '\t')); seek--);
	*(++seek) = '\0';
	j->next = JobHead;
	j->prev = JobHead->prev;
	JobHead->prev->next = j;
	JobHead->prev = j;
	return j;
}

void
remove_job(struct job_q *j)
{
	j->prev->next = j->next;
	j->next->prev = j->prev;

	free(j->cmdline);
	if (j->ttystate)
		free(j->ttystate);
	if (j->origstate)
		free(j->origstate);
	free(j);
	return;
}

void
clean_jobq(int volume)
{
	sigset_t        omask, bmask;
	struct job_q   *j, *jn;
	struct tm      *tm;

	sigemptyset(&omask);
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &bmask, &omask);
	for (j = JobHead->next; j != JobHead; j = jn)
	{
		jn = j->next;
		if (((j->jflags & JOBF_KEEPJOBONQ) == 0) && ((j->rst == S_EXIT) || (j->rst == S_KILL)))
		{
			if (volume != 0)
			{
				tm = localtime(&j->start);
				fprintf(stdout, "%d/%d %d:%2.2d%2s  [%d] %s  (%s)\n", tm->tm_mday, tm->tm_mon + 1,
					((tm->tm_hour + 11) % 12) + 1, tm->tm_min, tm->tm_hour > 11 ? "pm" : "am", (int) j->pgrp, j->cmdline,
					job_status(j));
			}
			remove_job(j);
		}
	}
	sigprocmask(SIG_SETMASK, &omask, NULL);
}

void
promote_job(struct job_q *j)
{
	j->prev->next = j->next;
	j->next->prev = j->prev;
	j->next = JobHead;
	j->prev = JobHead->prev;
	JobHead->prev->next = j;
	JobHead->prev = j;
}

int
wait_fg(struct job_q *j, sigset_t * mask)
{
	int             ret = EXIT_SUCCESS;

	while (j->rst == S_RUN)
		sigsuspend(mask);
	if ((j->ttystate != NULL) && (tcgetattr(0, j->ttystate) == -1))
	{
		free(j->ttystate);
		j->ttystate = NULL;
	}
	if (j->origstate != NULL)
		tcsetattr(0, TCSADRAIN, j->origstate);
	GRAB_BACK_TTY;
	switch (j->rst)
	{
	case S_KILL:
		fprintf(stderr, "Process Killed by Signal ");
		if ((j->eflags & EXEC_NEVERPAUSE) == 0)
			pressanykey();
		ret = -1;
		remove_job(j);
		break;
	case S_EXIT:
		if ((j->eflags & EXEC_NEVERPAUSE) == 0)
		{
			if ((j->eflags & EXEC_NOPAUSE) == 0)
				pressanykey();
			else
			if (j->exit != EXIT_SUCCESS)
				pressanykey();
		}
		ret = (j->exit == EXIT_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
		remove_job(j);
		break;
	case S_STOP:
		if (j->signal == SIGTSTP)
			fprintf(stderr, "Process Stopped by user ");
		else
			fprintf(stderr, "Process Stopped by signal ");
		if ((j->eflags & EXEC_NEVERPAUSE) == 0)
			pressanykey();
		ret = -1;
		break;
	case S_RUN:
		/*
		 * Humm 
		 */
		break;
	}
	return ret;
}

int
runnable_jobs(void)
{
	struct job_q   *j = JobHead->prev;

	while (j != JobHead)
	{
		if ((j->rst == S_RUN) || (j->rst == S_STOP))
			break;
		j = j->prev;
	}
	if (j != JobHead)
		return 1;
	else
		return 0;
}

int
run_in_fg(struct job_q *j)
{
	sigset_t        wmask, block;
	int             ret;

	sigemptyset(&wmask);
	sigemptyset(&block);
	sigaddset(&block, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block, &wmask);
	if ((j->rst == S_EXIT) || (j->rst == S_KILL))
	{
		sigprocmask(SIG_SETMASK, &wmask, NULL);
		return (j->rst == S_EXIT) ? ((j->exit == EXIT_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE) : -1;
	}
	j->rst = S_RUN;
	if (j->ttystate != NULL)
		tcsetattr(0, TCSADRAIN, j->ttystate);
	tcsetpgrp(0, j->pgrp);
	kill(j->pgrp * (-1), SIGCONT);
	promote_job(j);
	ret = wait_fg(j, &wmask);
	sigprocmask(SIG_SETMASK, &wmask, NULL);
	return ret;
}

void
kill_job(struct job_q *j)
{
	if (j->jflags & JOBF_HUPSENT)
		kill(-j->pgrp, SIGKILL);
	else
	{
		kill(-j->pgrp, SIGHUP);
		run_in_bg(j);
	}
	j->jflags |= JOBF_HUPSENT;
	return;
}

void
stop_job(struct job_q *j)
{
	kill(j->pgrp * (-1), SIGSTOP);
	return;
}

void
run_in_bg(struct job_q *j)
{
	sigset_t        wmask, block;

	sigemptyset(&wmask);
	sigemptyset(&block);
	sigaddset(&block, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block, &wmask);

	if ((j->rst == S_EXIT) || (j->rst == S_KILL))
	{
		sigprocmask(SIG_SETMASK, &wmask, NULL);
		return;
	}
	j->rst = S_RUN;
	kill(j->pgrp * (-1), SIGCONT);
	sigprocmask(SIG_SETMASK, &wmask, NULL);
	return;
}

char           *
job_status(struct job_q *j)
{
	switch (j->rst)
	{
	case S_RUN:
		return ("Running");
		break;
	case S_STOP:
		return ((char *) sys_siglist[j->signal]);
		break;
	case S_EXIT:
		return ("Exited");
		break;
	case S_KILL:
		return ((char *) sys_siglist[j->signal]);
		break;
	}
	return NULL;
}

static void
setup_pager(void)
{
	char           *raw_pager = NULL;

	pagerv = xmalloc(sizeof(char *) * 16);
	find_variable("execpager", &raw_pager);
	if (raw_pager == NULL)
		raw_pager = getenv("PAGER");
#ifdef PAGER
	if (raw_pager == NULL)
		raw_pager = PAGER;
#endif
	if (raw_pager != NULL)
	{
		if (prep_for_exec(raw_pager, &pagerc, pagerv, 15) <= 0)
		{
			free(pagerv);
			pagerv = NULL;
		}
	}
}

static void
exec_prog(char **argv)
{
	int             n;
	char           *path;

	path = *argv;
	*argv = (char *) strrchr(path, '/');
	if (*argv == NULL)
		*argv = path;
	else
		(*argv)++;
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	for (n = 3; n < 20; n++)
		close(n);
	execv(path, argv);
}

int
do_exec(char *exec_line, int argc, char **argv, int eflags)
{
	pid_t           pid_child, pid_pager = -1, pidtowait;
	int             pipefd[2], page;
	int             exitstat = EXIT_SUCCESS;
	sigset_t        wmask, block;
	struct job_q   *j;
	struct termios  Ttystate, *ttystate;

	page = (eflags & EXEC_PAGE);
	if (page)
	{
		if (pagerv == NULL)
			setup_pager();
		if ((pagerv == NULL) || (pipe(pipefd) < 0))
			page = 0;
	}
	sigemptyset(&wmask);
	sigemptyset(&block);
	sigaddset(&block, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block, &wmask);
	if (tcgetattr(0, &Ttystate) != -1)
		ttystate = &Ttystate;
	else
		ttystate = NULL;
	fflush(NULL);
	if ((pid_child = fork()) == -1)
		error("No more pids");
	else
	if (pid_child == 0)
	{
	  /***********************\
      |*  The Child Process  *|
      \***********************/
		if (page)
		{
			close(pipefd[0]);	/*- Read end */
			if (pipefd[1] != 1)
			{
				if (dup2(pipefd[1], 1) != 1)
					error("Could not dup pipe fd");
				else
					close(pipefd[1]);
			}
		}
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		setpgid(0, 0);
		if ((eflags & EXEC_BACKGROUND) == 0)
		{
			tcsetpgrp(0, getpgrp());
			tcsetpgrp(1, getpgrp());
		}
		sigprocmask(SIG_SETMASK, &wmask, NULL);
#ifdef DEBUG
		fprintf(stdout, "do exec: [%d] %s\n", getpid(), *argv);
#endif
		exec_prog(argv);
		fprintf(stderr, "%s: %s\n", *argv, strerror(errno));
		_exit(EXIT_FAILURE);
	} else
	{
	  /************************\
      |*  The parent process  *|
      \************************/
		if ((find_variable("logging", NULL) == 1) && (exec_line != (char *) NULL))
		{
			if (!page)
				syslog(LOG_LOCAL1 | LOG_INFO, "%d: %s (%d)", getpid(), exec_line, pid_child);
			else
				syslog(LOG_LOCAL1 | LOG_INFO, "%d: %s (paged) (%d)", getpid(), exec_line, pid_child);
		}
		setpgid(pid_child, pid_child);
		if ((eflags & EXEC_BACKGROUND) == 0)
		{
			tcsetpgrp(0, pid_child);
			tcsetpgrp(0, pid_child);
			/*
			 * kill ( -pid_child, SIGCONT);
			 */
		}

		if (page)
		{
			close(pipefd[1]);	/*- Write end */
			pid_pager = fork();
			if (pid_pager == -1)
			{
				close(pipefd[0]);
				error("No more pids");
			} else
			if (pid_pager == 0)
			{
		  		/***********************\
	      		|*  The Pager Process  *|
	      		\***********************/
				if (pipefd[0] != 0)
				{
					if (dup2(pipefd[0], 0) != 0)
						error("Could not dup pipe fd");
					else
						close(pipefd[0]);
				}
				signal(SIGTTIN, SIG_IGN);
				signal(SIGTTOU, SIG_IGN);
				setpgid(0, pid_child);
				sigprocmask(SIG_SETMASK, &wmask, NULL);
				exec_prog(pagerv);
				fprintf(stderr, "Could not exec pager: %s\n", *pagerv);
				fprintf(stderr, "%s: %s\n", *pagerv, strerror(errno));
				_exit(EXIT_FAILURE);
			} else
			{
				setpgid(pid_pager, pid_child);
				close(pipefd[0]);
			}
		}
		pidtowait = page ? pid_pager : pid_child;
		/*
		 * Wait for processes in the child group and store the 
		 * status of the child process 
		 */
		j = add_job(exec_line, pid_child, pidtowait, eflags, ttystate);
		if ((eflags & EXEC_BACKGROUND) == 0)
		{
			exitstat = wait_fg(j, &wmask);
			sigprocmask(SIG_SETMASK, &wmask, NULL);
			while (((eflags & EXEC_NONBGPROC) != 0) && (exitstat == -1) && (j->rst == S_STOP))
			{
				fprintf(stderr, "\n\n\007Cannot suspend this process !!\n");
				sleep(1);
				exitstat = run_in_fg(j);
			}
		} else
		if ((eflags & (EXEC_NEVERPAUSE | EXEC_NOPAUSE)) == 0)
		{
			sleep(1);
			fprintf(stderr, "\n\nProcess run in background - ");
			pressanykey();
			sigprocmask(SIG_SETMASK, &wmask, NULL);
		}
		if (eflags & EXEC_CLEARUP)
			while (argc--)
			{
				free((void *) *(argv + argc));
				*(argv + argc) = NULL;
			}
	}
	return exitstat;
}

void
handle_children(void)
{
	int             pid;
	int             status;
	struct job_q   *x;
	while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
	{
		for (x = JobHead->next; x != JobHead; x = x->next)
			if ((pid == x->waiton) || (pid == x->pgrp))
				break;
		if (x == JobHead)
			continue;
		if (pid == x->waiton)
		{
			if (WIFEXITED(status))
			{
				x->rst = S_EXIT;
				x->exit = WEXITSTATUS(status);
			} else
			if (WIFSIGNALED(status))
			{
				x->rst = S_KILL;
				x->signal = WTERMSIG(status);
			} else
			if (WIFSTOPPED(status))
			{
				x->rst = S_STOP;
				x->signal = WSTOPSIG(status);
			} else
			{
				fprintf(stderr, "Child status error\n");
				exit(EXIT_FAILURE);
			}
			x->change = 1;
		}
	}
	if (canjump)
		siglongjmp(jmpbuf, 1);
	return;
}

/*
 * Handle processes than reasign the tty themselves
 */

void
handle_snarfed_tty(void)
{
	pid_t           pgrp, mypgrp;

	pgrp = tcgetpgrp(0);
	mypgrp = getpgrp();
	if (pgrp != mypgrp)
		tcsetpgrp(0, mypgrp);
}

char           *
expand(char *args)
{
	char           *ptr;
	FILE           *fp;
	static char    *result;
	char            tmpbuf[4096];
	int             shell_meta, len1, len2;

	for(shell_meta = 0, ptr = args;*ptr;ptr++)
	{
		switch(*ptr)
		{
			case '*':
			case '?':
			case '[':
			case ']':
				shell_meta = 1;
				break;
			default:
				break;
		}
	}
	if(!shell_meta)
		return(args);
	snprintf(tmpbuf, sizeof(tmpbuf), "echo %s", args);
	if(!(fp = popen(tmpbuf, "r")))
	{
		fprintf(stderr, "popen: %s: %s\n", tmpbuf, strerror(errno));
		return((char *) 0);
	}
	for(len1 = len2 = 0;;)
	{
		if(!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
		if((ptr = strrchr(tmpbuf, '\n')))
			*ptr = 0;
		len1 += strlen(tmpbuf) + 2;
		if(!(result = (char *) realloc(result, len1 * sizeof(char))))
		{
			fprintf(stderr, "expand: malloc: %s\n", strerror(errno));
			pclose(fp);
			return((char *) 0);
		}
		snprintf(result + len2, len1, "%s ", tmpbuf);
		len2 += len1;
	}
	pclose(fp);
	return(result);
}

void
die_by_signal(int s)
{
	finish_jobq();
	exit(EXIT_SUCCESS);
}
