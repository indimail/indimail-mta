/*- $Id: supervise.c,v 1.47 2024-10-24 18:09:48+05:30 Cprogrammer Exp mbhangui $ */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "sig.h"
#include "strerr.h"
#include "error.h"
#include "fifo.h"
#include "open.h"
#include "lock.h"
#include "wait.h"
#include "coe.h"
#include "ndelay.h"
#include "env.h"
#include "iopause.h"
#include "taia.h"
#include "deepsleep.h"
#ifdef USE_RUNFS
#include "stralloc.h"
#include "str.h"
#endif
#include "fmt.h"
#include "substdio.h"
#include "getln.h"
#include "scan.h"
#include "seek.h"
#include "subreaper.h"

static char    *dir;
static int      selfpipe[2];
static int      fdlock = -1;
static int      fdcontrolwrite = -1;
static int      fdcontrol = -1;
static int      fdstatus = -1;
static int      fdok = -1;
static int      fdup = -1;
static int      fddn = -1;
static int      flagexit = 0;
static int      flagwantxx = 1;
static int      flagwantup = 1;
static int      flagpaused;		/*- defined if (pid) */
static int      fddir = -1;
static int      logger = 0;
static int      verbose = 0, silent = 0;
static char     flagfailed;
static unsigned long scan_interval = 60;
static char     is_subreaper = 0, do_setpgid = 0;
/*-
 * status[12-15] - pid
 * status[16]    - paused
 * status[17]    - wants up/down
 * status[18-19] - wait interval
 * status[20]    - up / down
 */
static char     status[21];
static char     pwdbuf[256];
static char     pidstr[FMT_ULONG];
static stralloc fatal, warn, info;
static stralloc wait_sv_status;
#ifdef USE_RUNFS
static stralloc run_service_dir;
int             use_runfs;
#endif
static pid_t    childpid = 0;	/*- 0 means down */
static int      grandchild = 0;
static pid_t    svpid;

void
pidchange(pid_t pid, char up)
{
	struct taia     now;
	unsigned long   u;

	taia_now(&now);
	taia_pack(status, &now);

	u = (unsigned long) pid;
	status[12] = u;
	u >>= 8;
	status[13] = u;
	u >>= 8;
	status[14] = u;
	u >>= 8;
	status[15] = u;
	status[20] = up;
}

void
announce(short sleep_interval)
{
	int             r;
	short          *s;

	status[16] = (childpid ? flagpaused : 0);
	status[17] = (flagwantxx ? (flagwantup ? 'u' : 'd') : 0);
	if (sleep_interval > 0) {
		s = (short *) (status + 18);
		*s = sleep_interval;
	} else
		status[18] = status[19] = 0;

	if (seek_begin(fdstatus)) {
		if (!silent)
			strerr_warn2(warn.s, "lseek: status: ", &strerr_sys);
		return;
	}
	if ((r = write(fdstatus, status, sizeof status)) == -1) {
		if (!silent)
			strerr_warn4(warn.s, "unable to write ", dir, "/supervise/status: ", &strerr_sys);
		return;
	}
	if (r < sizeof status) {
		if (!silent)
			strerr_warn4(warn.s, "unable to write ", dir, "/supervise/status: partial write: ", 0);
		return;
	}
}

static void
do_kill(int prgrp, int *siglist, const char **signame)
{
	int            *i;
	int             j, r;
	char            strnum[FMT_ULONG];

	for (i = siglist, j = 0; *i != -1; i++, j++) {
		if (grandchild || prgrp) {
			if ((r = killpg(childpid, *i)) == -1) {
				if (errno == error_srch && kill(childpid, *i) == -1) {
					if (errno == error_srch) {
						strnum[fmt_ulong(strnum, childpid)] = 0;
						strerr_warn8(warn.s, "pid ", pidstr, " killpg, kill ", strnum, " signal ", signame[j], ": ", &strerr_sys);
					}
				}
			}
			if (verbose && !r) {
				strnum[fmt_long(strnum, childpid)] = 0;
				strerr_warn7(info.s, "pid ", pidstr, " killed pgid ", strnum, " with signal ", signame[j], 0);
			}
		} else {
			if ((r = kill(childpid, *i)) == -1 && errno != error_srch) {
				strnum[fmt_ulong(strnum, childpid)] = 0;
				strerr_warn8(warn.s, "pid ", pidstr, " kill ", strnum, " signal ", signame[j], ": ", &strerr_sys);
			}
			if (verbose && !r) {
				strnum[fmt_ulong(strnum, childpid)] = 0;
				strerr_warn7(info.s, "pid ", pidstr, " killed pid ", strnum, " with signal ", signame[j], 0);
			}
		}
	} /*- for (i = siglist, j = 0; *i != -1; i++, j++) */
}

static void
sigterm()
{
	int             siglist[] = {-1, -1, -1};
	const char     *signame[] = {0, 0}, *p;
	static int      got_term;

	flagexit = 1;
	if (childpid) {
		p = env_get("SV_PWD");
		/*-
		 * die on the second SIGTERM if PPID is 1 and
		 * supervised service is a logger
		 */
		if (p && logger && getppid() == 1 && !got_term++)
			return;
		flagwantxx = 0;
		flagwantup = 1;
		siglist[0] = SIGTERM;
		siglist[1] = -1;
		signame[0] = "SIGTERM";
		do_kill(1, siglist, signame);
	}
}

void
trigger(void)
{
	if (write(selfpipe[1], "", 1) == -1)
		;
}

int
get_wait_params(unsigned short *interval, char **sv_name)
{
	int             fd, match, i;
	static ushort   sleep_interval = 60;
	unsigned long   t;
	struct substdio ssin;
	static stralloc tline;
	char            inbuf[256], strnum[FMT_ULONG];

	if (tline.len) {
		*sv_name = tline.s;
		*interval = sleep_interval;
	}
	*interval = 0;
	*sv_name = (char *) 0;
	/*-
	 * first line is the sleep interval
	 * second line is the service to wait for
	 */
	if ((fd = open_read("wait")) == -1) {
		if (errno == error_noent)
			return 2;
		strerr_warn2(warn.s, "unable to open wait: ", &strerr_sys);
		return -1;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof (inbuf));
	for (i = 0;i < 2; i++) {
		if (getln(&ssin, &tline, &match, '\n') == -1) {
			strerr_warn2(warn.s, "unable to read input", &strerr_sys);
			close(fd);
			return -1;
		}
		if (!tline.len)
			break;
		if (match) {
			tline.len--;
			tline.s[tline.len] = 0;
		} else {
			if (!stralloc_0(&tline)) {
				strerr_warn2(warn.s, "out of memory", 0);
				close(fd);
				return -1;
			}
			tline.len--;
		}
		if (!i) {/*- max value 32767 */
			scan_ulong(tline.s, &t);
			sleep_interval = t > 32767 ? 60 : (short) t;
		}
	} /*- for (i = 0;; i++) */
	close(fd);
	if (i < 2 || !tline.len)
		return 1;
	*sv_name = tline.s;
	*interval = sleep_interval;
	strnum[fmt_int(strnum, sleep_interval)] = 0;
	if (verbose)
		strerr_warn5(info.s, "wait ", strnum, " seconds for service ", *sv_name, 0);
	return 0;
}

const char     *run[4] = { "./run", 0 , 0, 0};
const char     *shutdown[4] = { "./shutdown", 0, 0, 0}; /*- ./shutdown, pid, dir, parent_id, NULL */
const char     *alert[6] = { "./alert", 0, 0, 0, 0, 0 }; /*- ./alert, alert pid, child_exit_value, signal_value, dir, parent_id, NULL */

void
do_wait()
{
	int             fd, fd_depend, i, r, len;
	unsigned short  sleep_interval = 60;
	unsigned long   wpid;
	time_t          t1, t2;
	char            wstatus[20], strnum[FMT_ULONG];
	char           *service_name;

#ifdef USE_RUNFS
	if (use_runfs && fchdir(fddir) == -1)
		strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
	/* get sleep interval and the service name for which we are waiting */
	if (!(i = get_wait_params(&sleep_interval, &service_name))) {
		if (!stralloc_copyb(&wait_sv_status, "../", 3) ||
				!stralloc_cats(&wait_sv_status, service_name) ||
				!stralloc_0(&wait_sv_status)) {
			strerr_warn2(warn.s, "out of memory", 0);
#ifdef USE_RUNFS
			if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
				strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
			return;
		}
		wait_sv_status.len--;
		/*-
		 * if the service for which we are supposed to
		 * wait doesn't exist, dont wait
		 */
		if (access(wait_sv_status.s, F_OK))
			i = 1;
	}
#ifdef USE_RUNFS
	if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
		strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
	if (i)
		return;
	pidchange(svpid, 0);
	if (!stralloc_catb(&wait_sv_status, "/supervise/", 11)) {
		strerr_warn2(warn.s, "out of memory", 0);
		return;
	}
	len = wait_sv_status.len;
	if (!stralloc_catb(&wait_sv_status, "up", 2) || !stralloc_0(&wait_sv_status)) {
		strerr_warn2(warn.s, "out of memory", 0);
		return;
	}

	/*-
	 * supervise/up is a FIFO
	 * if the service service_name is running
	 *   opening supervise/up with O_WRONLY should return
	 * if the service service_name is down
	 *   opening supervise/up with O_WRONLY should block
	 *
	 * supervise/dn is a FIFO
	 * if the service service_name is running
	 *   opening supervise/dn with O_WRONLY should block
	 * if the service service_name is down
	 *   opening supervise/dn with O_WRONLY should return
	 */
	t1 = time(0);
	for (i = 0;;) {
		if ((fd_depend = open(wait_sv_status.s, O_WRONLY)) == -1) {
			if (errno == error_noent) {/*- supervise for service_name is not running */
				if (!silent)
					strerr_warn3(warn.s, "supervise not running for service ", service_name, 0);
				i++;
				sleep(1); /* prevent high cpu utilization */
				continue;
			}
			if (errno == error_intr)
				continue;
			strerr_die4sys(111, fatal.s, "unable to write ", wait_sv_status.s, ": ");
		} else {
			close(fd_depend);
			break;
		}
	}
	t2 = time(0);
	if (!silent && i) {
		strnum[fmt_int(strnum, t2 - t1)] = 0;
		strerr_warn6(warn.s, "service ", service_name, " up after ~", strnum, " seconds", 0);
	}

	/*- now open the status file of service_name */
	wait_sv_status.len = len;
	if (!stralloc_catb(&wait_sv_status, "status", 6) || !stralloc_0(&wait_sv_status)) {
		strerr_warn2(warn.s, "out of memory", 0);
		return;
	}
	announce(-1);
	for (;;) {
		if ((fd = open_read(wait_sv_status.s)) == -1) {
			if (errno != error_noent) {
				strerr_warn4(warn.s, "unable to open ", wait_sv_status.s, ": ", &strerr_sys);
				return;
			}
			sleep(1);
			continue;
		}
		if ((r = read(fd, wstatus, sizeof wstatus)) == -1) {
			if (!silent)
				strerr_warn4(warn.s, "unable to read ", wait_sv_status.s, ": ", &strerr_sys);
			close(fd);
			sleep(1);
			continue;
		} else
		if (!r) {
			if (!silent)
				strerr_warn4(warn.s, "file ", wait_sv_status.s, " is of zero bytes: ", &strerr_sys);
			close(fd);
			sleep(1);
			continue;
		}
		close(fd);
		wpid = (unsigned char) wstatus[15];
		wpid <<= 8;
		wpid += (unsigned char) wstatus[14];
		wpid <<= 8;
		wpid += (unsigned char) wstatus[13];
		wpid <<= 8;
		wpid += (unsigned char) wstatus[12];
		/*- supervise started and not in paused and not in want down */
		if (wpid && (!wstatus[16] && wstatus[17] != 'd'))
			break;
	} /*- for (;;) */
	pidchange(svpid, 0); /*- update start time */
	announce(sleep_interval);
	deepsleep(sleep_interval);
	childpid = 0;
	announce(0);
	return;
}

void
trystart(const char *how)
{
	pid_t           f, pgid;
	struct stat     st;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	do_wait();
#ifdef USE_RUNFS
	if (use_runfs && fchdir(fddir) == -1)
		strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
	if (!is_subreaper) {
		if (stat(*run, &st) == -1)
			strerr_die4sys(111, fatal.s, "unable to stat ", *run, ": ");
		if (st.st_mode & S_ISVTX) /*- sticky bit on run file */
			is_subreaper = subreaper() == 0 ? 1 : 0;
	}
#ifdef USE_RUNFS
	if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
		strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
	switch (f = fork())
	{
	case -1:
		if (!silent)
			strerr_warn2(warn.s, "unable to fork, sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		flagfailed = 1;
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
#ifdef USE_RUNFS
		if (use_runfs && fchdir(fddir) == -1)
			strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
		if ((do_setpgid || is_subreaper) && setpgid(0, 0) == -1)
			strerr_die2sys(111, fatal.s, "unable to set process group id: ");
		run[1] = dir;
		run[2] = how;
		execve(*run, (char **) run, environ);
		strerr_die2sys(111, fatal.s, "unable to start run: ");
	}
	grandchild = 0;
	flagpaused = 0;
	if (verbose) {
		pgid = getpgid(f);
		strnum1[fmt_ulong(strnum1, f)] = 0;
		strnum2[fmt_ulong(strnum2, pgid)] = 0;
		strerr_warn8(info.s, "pid ", pidstr, " started child pid ", strnum1,
				" pgid ", strnum2,
				is_subreaper ? " in subreaper mode" : " in non-subreaper mode", 0);
	}
	pidchange((childpid = f), 1);
	announce(0);
	if ((fdup = open_read("supervise/up")) == -1) /*- open O_RDONLY|O_NDELAY */
		strerr_die2sys(111, fatal.s, "read: supervise/up: ");
	coe(fdup);
	if (fddn != -1) {
		close(fddn);
		fddn = -1;
	}
	deepsleep(1);
}

void
tryaction(const char **action, pid_t cpid, int wstat, int do_alert)
{
	pid_t           f;
	int             t, i;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG + 1];

	switch (f = fork())
	{
	case -1:
		if (!silent)
			strerr_warn2(warn.s, "unable to fork, sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
#ifdef USE_RUNFS
		if (use_runfs && fchdir(fddir) == -1)
			strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
		strnum1[fmt_ulong(strnum1, cpid)] = 0;
		action[1] = dir;
		action[2] = strnum1;
		if (do_alert) {
			if (WIFSTOPPED(wstat) || WIFSIGNALED(wstat)) {
				strnum2[fmt_uint(strnum2, WIFSTOPPED(wstat) ? WSTOPSIG(wstat) : WTERMSIG(wstat))] = 0;
				action[3] = strnum2;
				action[4] = "stopped/signalled";
			} else
			if (WIFEXITED(wstat)) {
				if ((t = WEXITSTATUS(wstat)) < 0) {
					i = fmt_uint(strnum2 + 1, 0 - t);
					*strnum2 = '-';
					strnum2[i + 1] = 0;
				} else
					strnum2[fmt_uint(strnum2, t)] = 0;
				action[3] = strnum2;
				action[4] = "exited";
			} else {
				action[3] = "-1";
				action[4] = "unknown";
			}
		} else {
			action[2] = dir;
			action[3] = NULL;
		}
		execve(*action, (char **) action, environ);
		strerr_die4sys(111, fatal.s, "unable to exec ", *action, ": ");
	default:
		wait_pid(&t, f);
	}
}

static void
postmortem(pid_t pid, int wstat)
{
	int             t;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if (silent)
		return;
	strnum1[fmt_ulong(strnum1, pid)] = 0;
	if (wait_stopped(wstat) || wait_continued(wstat)) {
		t = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
		strnum2[fmt_ulong(strnum2, t)] = 0;
		strerr_warn5(warn.s, "pid ", strnum1,
			wait_stopped(wstat) ? " stopped by signal " : " started by signal ", strnum2, 0);
	} else
	if (wait_signaled(wstat)) {
		t = wait_termsig(wstat);
		strnum2[fmt_ulong(strnum2, t)] = 0;
		strerr_warn5(warn.s, "pid ", strnum1, " got signal ", strnum2, 0);
	} else {
		t = wait_exitcode(wstat);
		strnum2[fmt_ulong(strnum2, t)] = 0;
		strerr_warn5(warn.s, "pid ", strnum1, " exited with status=", strnum2, 0);
	}
	return;
}

void
doit()
{
	iopause_fd      x[2];
	int             siglist[] = {-1, -1, -1};
	const char     *signame[] = {0, 0};
	struct taia     deadline;
	struct taia     stamp;
	static int      double_fork_flag;
	pid_t           r;
	int             wstat, wstat_reap, t, g = 0, do_break;
	char            ch, c = 0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	announce(0);

	for (;;) {
		if (flagexit && !childpid) {
			if (verbose)
				strerr_warn4(info.s, "supervise pid ", pidstr, " exiting...", 0);
			return;
		}

		sig_unblock(sig_child);

		x[0].fd = selfpipe[0];
		x[0].events = IOPAUSE_READ;
		x[1].fd = fdcontrol;
		x[1].events = IOPAUSE_READ;
		taia_now(&stamp); /*- current timestamp */
		taia_uint(&deadline, 3600); /*- one hour deadline */
		taia_add(&deadline, &stamp, &deadline); /*- current timestamp + 1 hour */
		/*-
		 * handle events on
		 * 1. selfpipe -> when child dies
		 * 2. supervise/control
		 * This is where supervise will be, waiting
		 * for sigchild to happen or waiting for
		 * something written to supervise/control
		 */
		iopause(x, 2, &deadline, &stamp);

		/*-
		 * block sigchld till we handle
		 * current death of a child without
		 * any interruption by another sigchld
		 */
		sig_block(sig_child);

		/*- empty the pipe and come out when empty */
		while (read(selfpipe[0], &ch, 1) == 1) ;

		/*- handle child exits */
		for (do_break = 0;;) {
			if (!(r = waitpid(-1, &wstat, WNOHANG|WUNTRACED|WCONTINUED)))
				break;
			if (r == -1) {
				if (errno != error_intr) {
					do_break = 1;
					break;
				} else
					continue;
			}
			if (wait_stopped(wstat) || wait_continued(wstat)) {
				strnum1[fmt_ulong(strnum1, childpid)] = 0;
				t = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
				strnum2[fmt_ulong(strnum2, t)] = 0;
				if (!silent)
					strerr_warn5(warn.s, "pid ", strnum1,
						wait_stopped(wstat) ? " stopped by signal " : " continued by signal ", strnum2, 0);
				break;
			}
			if (r == childpid || (r != -1 && grandchild)) { /*- process exited */
				if (r != childpid) {
					strnum1[fmt_ulong(strnum1, childpid)] = 0;
					strnum2[fmt_ulong(strnum2, r)] = 0;
					if (!silent)
						strerr_warn6(warn.s, "pid ", strnum2, " double fork by ppid ", strnum1, " died", 0);
					childpid = r;
				}
				if (is_subreaper) {
					for (do_break = 0;;) {
						if (!(r = wait_nohang(&wstat_reap))) { /*- children but no zombies */
							strnum1[fmt_ulong(strnum1, childpid)] = 0;
							if (!double_fork_flag++)
								if (!silent)
									strerr_warn4(warn.s, "pid ", strnum1, " did double fork", 0);
							postmortem(childpid, wstat);
							grandchild = 1;
							do_break = 1;
							break;
						}
						if (r == -1) {
							if (errno == error_child) /*- no child left */
								break;
							if (errno != error_intr) {
								do_break = 1;
								break;
							}
						}
					} /*- for (do_break = 0;;) */
				}
				if (do_break)
					break;
				double_fork_flag = 0;
				postmortem(childpid, wstat);
				childpid = 0;
				pidchange(svpid, 0);
				close(fdup);
				announce(0);
				if (flagexit) {
					if (verbose)
						strerr_warn4(info.s, "supervise pid ", pidstr, " exiting...", 0);
					return;
				}
				if (flagwantxx && flagwantup) {
#ifdef USE_RUNFS
					if (use_runfs && fchdir(fddir) == -1)
						strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
					if (!access(*alert, F_OK))
						tryaction(alert, r, wstat, 1);
#ifdef USE_RUNFS
					if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
						strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
					trystart("abnormal startup"); /* restart because of abnormal exit */
				}
				break;
			} /*- if (r == childpid || (r != -1 && grandchild)) */
		} /*- for (;;) */
		if (flagfailed && flagwantxx && flagwantup) {
			flagfailed = 0;
			trystart("system failure"); /*- fork failed */
		}
		if (read(fdcontrol, &ch, 1) == 1) {
			switch (ch)
			{
			case 'G': /*- send signal to process group */
				g = 1;
				break;
			case 'r': /*- restart */
				flagwantxx = 1;
				flagwantup = 1;
				if (childpid) {
#ifdef USE_RUNFS
					if (use_runfs && fchdir(fddir) == -1)
						strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, childpid, 0, 0); /*- run the shutdown command */
#ifdef USE_RUNFS
					if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
						strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
					siglist[0] = SIGTERM;
					siglist[1] = SIGCONT;
					signame[0] = "SIGTERM";
					signame[1] = "SIGCONT";
					do_kill(g, siglist, signame);
					g = flagpaused = 0;
				}
				t = childpid;
				while (c < 10) {
					if (kill(childpid, 0) && errno == error_srch) {
						t = 0;
						break;
					}
					c++;
					usleep(100);
				}
				announce(0);
				if (!t)
					trystart("manual restart"); /* normal startup */
				break;
			case 'd': /*- down */
				flagwantxx = 1;
				flagwantup = 0;
				if (childpid) {
#ifdef USE_RUNFS
					if (use_runfs && fchdir(fddir) == -1)
						strerr_die2sys(111, fatal.s, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, childpid, 0, 0); /*- run the shutdown command */
					siglist[0] = SIGTERM;
					siglist[1] = SIGCONT;
					signame[0] = "SIGTERM";
					signame[1] = "SIGCONT";
					do_kill(g, siglist, signame);
					g = flagpaused = 0;
#ifdef USE_RUNFS
					if (use_runfs && chdir(dir) == -1) /*- switch back to /run/svscan */
						strerr_die4sys(111, fatal.s, "unable to switch back to ", dir, ": ");
#endif
					if ((fddn = open_read("supervise/dn")) == -1)
						strerr_die4sys(111, fatal.s, "unable to read ", dir, "/supervise/dn: ");
					coe(fddn);
				}
				announce(0);
				break;
			case 'u': /*- up */
				flagwantxx = 1;
				flagwantup = 1;
				announce(0);
				if (!childpid)
					trystart("manual startup"); /* normal startup */
				break;
			case 'o': /*- run once */
				flagwantxx = 0;
				announce(0);
				if (!childpid)
					trystart("one-time startup"); /* normal startup */
				break;
			case 'a': /*- send ALRM */
				if (childpid) {
					siglist[0] = SIGALRM;
					siglist[1] = -1;
					signame[0] = "SIGALRM";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'h': /*- send HUP */
				if (childpid) {
					siglist[0] = SIGHUP;
					siglist[1] = -1;
					signame[0] = "SIGHUP";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'k': /*- send kill */
				if (childpid) {
					siglist[0] = SIGKILL;
					siglist[1] = -1;
					signame[0] = "SIGKILL";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 't': /*- send TERM */
				if (childpid) {
					siglist[0] = SIGTERM;
					siglist[1] = -1;
					signame[0] = "SIGTERM";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'i': /*- send INT */
				if (childpid) {
					siglist[0] = SIGINT;
					siglist[1] = -1;
					signame[0] = "SIGINT";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'q': /*- send SIGQUIT */
				if (childpid) {
					siglist[0] = SIGQUIT;
					siglist[1] = -1;
					signame[0] = "SIGQUIT";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'U': /*- send USR1 */
			case '1':
				if (childpid) {
					siglist[0] = SIGUSR1;
					siglist[1] = -1;
					signame[0] = "SIGUSR1";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case '2': /*- send USR2 */
				if (childpid) {
					siglist[0] = SIGUSR2;
					siglist[1] = -1;
					signame[0] = "SIGUSR2";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'p': /*- send STOP */
				flagpaused = 1;
				announce(0);
				if (childpid) {
					siglist[0] = SIGSTOP;
					siglist[1] = -1;
					signame[0] = "SIGSTOP";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'c': /*- send CONT */
				flagpaused = 0;
				announce(0);
				if (childpid) {
					siglist[0] = SIGCONT;
					siglist[1] = -1;
					signame[0] = "SIGCONT";
					do_kill(g, siglist, signame);
					g = 0;
				}
				break;
			case 'x': /*- exit supervise */
				flagexit = 1;
				announce(0);
				break;
			} /*- switch (ch) */
		} /*- if (read(fdcontrol, &ch, 1) == 1) */
	} /* for (;;) */
}

#ifdef USE_RUNFS
void
cleanup(const char *d)
{
	const char     *sv_files[] = { "control", "lock", "ok", "status", "up", 0};
	const char     *sv_dirs[] = {"supervise", "log/supervise", 0};
	const char    **p, **q;
	int             l;
	stralloc        tmp = {0};

	for (p = sv_dirs; *p; p++) {
		if (!access(*p, W_OK)) {
			if (!stralloc_copys(&tmp, *p) ||
					!stralloc_append(&tmp, "/"))
				strerr_die2x(111, fatal.s, "out of memory");
			l = tmp.len;
			for (q = sv_files; *q; q++) {
				if (!stralloc_cats(&tmp, *q) ||
						!stralloc_0(&tmp))
					strerr_die2x(111, fatal.s, "out of memory");
				if (access(tmp.s, W_OK) || unlink(tmp.s))
					strerr_warn6(warn.s, "unable to remove file ", d, "/", tmp.s, ": ", &strerr_sys);
				tmp.len = l;
			}
			if (rmdir(*p))
				strerr_warn6(warn.s, "unable to remove dir ", d, "/", *p, ": ", &strerr_sys);
		}
	}
}

int
qchown(const char *name, uid_t uid, gid_t gid)
{
	int             fd;

	if ((fd = open_read(name)) == -1)
		return -1;
	if (fchown(fd, uid, gid) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

void
initialize_run(const char *service_dir, mode_t mode, uid_t own, gid_t grp)
{
	const char     *run_dir, *parent_dir = (char *) 0;
	int             i;

	if (env_get("DISABLE_RUN")) {
		use_runfs = 0;
		return;
	}
	if (!access("/run/svscan", F_OK)) {
		run_dir = "/run";
		i = 4;
		use_runfs = 1;
	} else
	if (!access("/var/run/svscan", F_OK)) {
		run_dir = "/var/run";
		i = 8;
		use_runfs = 1;
	} else
		return;
	cleanup(service_dir);
	if (!stralloc_copyb(&run_service_dir, run_dir, i) ||
			!stralloc_catb(&run_service_dir, "/svscan/", 8))
		strerr_die2x(111, fatal.s, "out of memory");

	if (!str_diff(service_dir, "log")) {
		/*-
		 * SV_PWD is the dirname of directory in /service e.g.
		 * for qmail-smtpd.25/log it will be qmail-smtpd.25
		 * supervise invocation will be
		 * supervise log qmail-stmpd.25
		 * supervise cwd will be
		 * /service/qmail-stmpd.25 or /run/svscan/qmail-smtpd.25
		 */
		if (!(parent_dir = env_get("SV_PWD"))) {
			if (!getcwd(pwdbuf, 255))
				strerr_die2sys(111, fatal.s, "log: unable to get current working directory: ");
			i = str_rchr(pwdbuf, '/');
			if (!pwdbuf[i])
				strerr_die2sys(111, fatal.s, "unable to get current working directory: ");
			parent_dir = pwdbuf + i + 1;
		}
		if (!stralloc_cats(&run_service_dir, parent_dir) ||
				!stralloc_append(&run_service_dir, "/"))
			strerr_die2x(111, fatal.s, "out of memory");
	}
	/*- full path /run/svscan/service/qmail-smtpd.25 or /run/svscan/service/qmail-smtpd.25/log */
	if (!stralloc_cats(&run_service_dir, service_dir) ||
			!stralloc_0(&run_service_dir))
		strerr_die2x(111, fatal.s, "out of memory");
	if (chdir(run_dir) == -1)
		strerr_die4sys(111, fatal.s, "unable to chdir to ", run_dir, ": ");
	if (access("svscan", F_OK) && mkdir("svscan", 0755) == -1)
		strerr_die4sys(111, fatal.s, "unable to mkdir ", run_dir, "/svscan: ");
	if (chdir("svscan") == -1)
		strerr_die4sys(111, fatal.s, "unable to chdir to ", run_dir, "/svscan: ");

	if (parent_dir) { /*- we were called as supervise log */
		if (access(parent_dir, F_OK)) {
			if (mkdir(parent_dir, 0755) == -1 && errno != error_exist)
				strerr_die6sys(111, fatal.s, "unable to mkdir ", run_dir, "/svscan/", parent_dir, ": ");
			if (qchown(parent_dir, own, grp) == -1)
				strerr_die6sys(111, fatal.s, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, ": ");
		}
		if (chdir(parent_dir) == -1)
			strerr_die6sys(111, fatal.s, "unable to chdir to ", run_dir, "/svscan/", parent_dir, ": ");
		if (access("log", F_OK)) {
			if (mkdir("log", mode) == -1 && errno != error_exist)
				strerr_die6sys(111, fatal.s, "unable to mkdir ", run_dir, "/svscan/", parent_dir, "/log: ");
			if (qchown("log", own, grp) == -1)
				strerr_die6sys(111, fatal.s, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, "/log: ");
		}
	} else
	if (access(service_dir, F_OK)) {
		if (mkdir(service_dir, mode) == -1 && errno != error_exist)
			strerr_die6sys(111, fatal.s, "unable to mkdir ", run_dir, "/svscan/", service_dir, ": ");
		if (qchown(service_dir, own, grp) == -1)
			strerr_die6sys(111, fatal.s, "unable to set permssions for ", run_dir, "/svscan/", service_dir, ": ");
	}
	if (chdir(service_dir) == -1) /*- e.g. qmail-smtpd.25 or log */
		strerr_die6sys(111, fatal.s, "unable to chdir to ", run_dir, "/svscan/", service_dir, ": ");
	dir = run_service_dir.s;
	return;
}
#endif

char           *init[2] = { 0, 0 };
static stralloc init_sv_path = {0};

int
do_init()
{
	int             t, f;

	switch (f = fork())
	{
	case -1:
		if (!silent)
			strerr_warn4(warn.s, "unable to fork to run ", *init, ", sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		flagfailed = 1;
		return -1;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
		execve(*init, init, environ);
		strerr_die4sys(111, fatal.s, "unable to start ", *init, ": ");
	}
	if (wait_pid(&t, f) == -1) {
		if (!silent)
			strerr_warn4(warn.s, "wait failed ", dir, "/init: ", &strerr_sys);
		return -1;
	}
	if (WIFSTOPPED(t) || WIFSIGNALED(t))
		return -1;
	else
	if (WIFEXITED(t))
		return (WEXITSTATUS(t));
	return -1;
}

int
main(int argc, char **argv)
{
	struct stat     st;
	char           *ptr;

	verbose = env_get("VERBOSE") ? 1 : 0;
	silent = env_get("SILENT") ? 1 : 0;
	do_setpgid = env_get("SETPGID") ? 1 : 0;
	if (!(dir = argv[1]) || argc > 3)
		strerr_die1x(100, "supervise: usage: supervise dir [parent_ident]");
	if (*dir == '/' || *dir == '.')
		strerr_die1x(100, "supervise: dir cannot start with '/' or '.'");
	if (argc == 2) {
		if (!stralloc_copys(&fatal, dir) || !stralloc_catb(&fatal, ": supervise: fatal: ", 20))
			strerr_die1x(111, "supervise: out of memory");
		if (!stralloc_copys(&warn, dir) || !stralloc_catb(&warn, ": supervise: warning: ", 22))
			strerr_die1x(111, "supervise: out of memory");
		if (!stralloc_copys(&info, dir) || !stralloc_catb(&info, ": supervise: info: ", 19))
			strerr_die1x(111, "supervise: out of memory");
	} else
	if (argc == 3) {
		logger = 1; /*- passed by svscan for the log supervise process */
		if (!stralloc_copys(&fatal, argv[2]) || !stralloc_catb(&fatal, "/log: supervise: fatal: ", 24))
			strerr_die1x(111, "supervise: out of memory");
		if (!stralloc_copys(&warn, argv[2]) || !stralloc_catb(&warn, "/log: supervise: warning: ", 26))
			strerr_die1x(111, "supervise: out of memory");
		if (!stralloc_copys(&info, argv[2]) || !stralloc_catb(&info, "/log: supervise: info: ", 23))
			strerr_die1x(111, "supervise: out of memory");
	}
	if (!stralloc_0(&fatal) || !stralloc_0(&warn) || !stralloc_0(&info))
		strerr_die2x(111, fatal.s, "out of memory");
	if (pipe(selfpipe) == -1)
		strerr_die4sys(111, fatal.s, "unable to create pipe for ", dir, ": ");
	/*- close selfpipe on execve */
	coe(selfpipe[0]);
	coe(selfpipe[1]);
	ndelay_on(selfpipe[0]);
	ndelay_on(selfpipe[1]);
	sig_termcatch(sigterm);
	sig_block(sig_child);
	sig_catch(sig_child, trigger);

	svpid = getpid();
	pidstr[fmt_ulong(pidstr, svpid)] = 0;
	if (!env_put2("PPID", pidstr))
		strerr_die2x(111, fatal.s, "out of memory");
	if (!(ptr = env_get("SCANDIR"))) {
		if (!getcwd(pwdbuf, 255))
			strerr_die2sys(111, fatal.s, "unable to get current working directory: ");
		ptr = pwdbuf;
	}
	/*-
	 * allocate now so that we don't fail later
	 * ../ + str_len(ptr) + /supervise/status\0
	 */
	if (!stralloc_ready(&wait_sv_status, str_len(ptr) + 21))
		strerr_die2x(111, fatal.s, "out of memory");
	if (!stralloc_copys(&init_sv_path, ptr) ||
			!stralloc_append(&init_sv_path, "/") ||
			!stralloc_cats(&init_sv_path, dir) ||
			!stralloc_append(&init_sv_path, "/") ||
			!stralloc_catb(&init_sv_path, "init", 4) ||
			!stralloc_0(&init_sv_path))
		strerr_die2x(111, fatal.s, "out of memory");
	init[0] = init_sv_path.s;


	if ((ptr = env_get("SCANINTERVAL")))
		scan_ulong(ptr, &scan_interval);
	if (chdir(dir) == -1) /*- chdir to /service/service_name */
		strerr_die4sys(111, fatal.s, "unable to chdir to ", dir, ": ");
	if (stat("down", &st) != -1)
		flagwantup = 0;
	else
	if (errno != error_noent)
		strerr_die4sys(111, fatal.s, "unable to stat ", dir, "/down: ");

	if ((fddir = open_read(".")) == -1) /*- save dir for /service */
		strerr_die2sys(111, fatal.s, "unable to open current directory: ");
	if (stat("run", &st) == -1)
		strerr_die4sys(111, fatal.s, "unable to stat ", dir, "/run: ");
	if (st.st_mode & S_ISVTX) /*- sticky bit on run file */
		is_subreaper = subreaper() == 0 ? 1 : 0;
#ifdef USE_RUNFS
	if (stat(".", &st) == -1)
		strerr_die2sys(111, fatal.s, "unable to stat current directory: ");
	initialize_run(dir, st.st_mode, st.st_uid, st.st_gid); /*- this will set dir to new service directory in /run, /var/run */
#endif
	if (mkdir("supervise", 0700) && errno != error_exist)
		strerr_die2sys(111, fatal.s, "unable to create supervise directory: ");

	fdlock = open_append("supervise/lock");
	if ((fdlock == -1) || (lock_exnb(fdlock) == -1))
		strerr_die4sys(100, fatal.s, "unable to acquire ", dir, "/supervise/lock: ");
	coe(fdlock);

	fifo_make("supervise/control", 0600);
	if ((fdcontrol = open_read("supervise/control")) == -1)
		strerr_die4sys(111, fatal.s, "unable to read ", dir, "/supervise/control: ");
	coe(fdcontrol);
	ndelay_on(fdcontrol); /*- shouldn't be necessary */
	if ((fdcontrolwrite = open_write("supervise/control")) == -1)
		strerr_die4sys(111, fatal.s, "unable to write ", dir, "/supervise/control: ");
	coe(fdcontrolwrite);

	pidchange(svpid, 0);
	if ((fdstatus = open_trunc("supervise/status")) == -1)
		strerr_die4sys(111, fatal.s, "unable to create ", dir, "/supervise/status: ");
	coe(fdstatus);
	announce(0);

	fifo_make("supervise/ok", 0600);
	if ((fdok = open_read("supervise/ok")) == -1)
		strerr_die4sys(111, fatal.s, "unable to read ", dir, "/supervise/ok: ");
	coe(fdok);
	fifo_make("supervise/up", 0600);
	fifo_make("supervise/dn", 0600);
	if (!flagwantup) {
		if ((fddn = open_read("supervise/dn")) == -1)
			strerr_die4sys(111, fatal.s, "unable to read ", dir, "/supervise/dn: ");
		coe(fddn);
	}

	/*-
	 * By now we have finished initialization of /run. We
	 * can now run scripts that will not fail due to absence of
	 * directories/files in /run
	 */
	while (1) {
		if (!access(*init, X_OK)) {
			if (do_init()) {
				if (!silent)
					strerr_warn4(warn.s, "initialization failed for ", dir, ", sleeping 60 seconds: ", 0);
				deepsleep(60);
			} else
				break;
		} else
			break;
	}

	if (!flagwantxx || flagwantup)
		trystart("auto startup"); /*- normal startup */
	doit();
	announce(0);
	_exit(0);
}

void
getversion_supervise_c()
{
	const char     *x = "$Id: supervise.c,v 1.47 2024-10-24 18:09:48+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: supervise.c,v $
 * Revision 1.47  2024-10-24 18:09:48+05:30  Cprogrammer
 * fix status file getting truncated before announce()
 * open supervise/dn fifo in read mode after service goes down
 * renamed SERVICEDIR to SCANDIR
 * do not exit in do_wait()
 * do not exit in get_wait_params()
 *
 * Revision 1.46  2024-10-22 23:02:31+05:30  Cprogrammer
 * renamed SERVICEDIR to SCANDIR for svscan
 * open supervise/dn after service is brought down
 *
 * Revision 1.45  2024-10-21 17:47:39+05:30  Cprogrammer
 * BUG: status file truncated before announce
 *
 * Revision 1.44  2024-10-20 20:44:45+05:30  Cprogrammer
 * fixed comment
 *
 * Revision 1.43  2024-08-29 18:51:11+05:30  Cprogrammer
 * use servicedir as first argument for exec of ./run, ./alert and ./shutdown
 *
 * Revision 1.42  2024-08-14 11:22:05+05:30  Cprogrammer
 * pass additional "startup type" argument to ./run
 *
 * Revision 1.41  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.40  2024-04-01 18:21:40+05:30  Cprogrammer
 * set PPID env variable for child
 * added comments, updated variable name for flagwant, spid arg
 *
 * Revision 1.39  2024-03-01 15:27:07+05:30  Cprogrammer
 * die on two SIGTERM if PPID is 1 and supervise is a logger
 * Fixed termination of processes with -G option when no processes found in a process group
 *
 * Revision 1.38  2024-02-09 00:31:40+05:30  Cprogrammer
 * replaced chown with fchown
 *
 * Revision 1.37  2023-06-05 18:17:16+05:30  Cprogrammer
 * disable warning message if pid does not exist
 *
 * Revision 1.36  2023-06-05 17:53:16+05:30  Cprogrammer
 * Fix waitpid returning EINVAL on OSX
 *
 * Revision 1.35  2023-04-27 13:25:24+05:30  Cprogrammer
 * ignore wait if service for which supervise should wait doesn't exit
 * reduce sleep to 1 when service being waited for hasn't started
 *
 * Revision 1.34  2023-04-02 23:16:34+05:30  Cprogrammer
 * pass directory as the last argument to ./run, ./shutdown, ./alert scripts
 *
 * Revision 1.33  2023-03-07 00:06:34+05:30  Cprogrammer
 * check for sticky bit on run on every restart
 *
 * Revision 1.32  2023-03-06 23:13:54+05:30  Cprogrammer
 * fix termination by svc in subreaper mode
 * handle SIGTERM to exit and terminate child
 * set Process Group ID of child when SETPGID env variable is set
 *
 * Revision 1.31  2023-03-05 22:59:57+05:30  Cprogrammer
 * added exit informational message
 *
 * Revision 1.30  2023-03-04 18:14:42+05:30  Cprogrammer
 * cleanup supervise directory in service directory when using run fs
 *
 * Revision 1.29  2023-03-04 13:38:09+05:30  Cprogrammer
 * disable using /run if DISABLE_RUN env variable is set
 *
 * Revision 1.28  2022-12-14 13:35:44+05:30  Cprogrammer
 * become subreaper if run file has sticky bit set
 * display exit status and termination signal
 * use VERBOSE, SILENT env variables to control logging of info, warn messages
 *
 * Revision 1.27  2022-12-02 09:05:07+05:30  Cprogrammer
 * sleep SCANINTERVAL seconds if supervise for waited service is not running
 *
 * Revision 1.26  2022-07-03 11:32:00+05:30  Cprogrammer
 * open supervise/up in O_WRONLY when waiting for service
 *
 * Revision 1.25  2022-05-25 08:27:08+05:30  Cprogrammer
 * new variable use_runfs to indicate if svscan is using /run
 *
 * Revision 1.24  2021-08-11 18:12:15+05:30  Cprogrammer
 * fix for using of uninitialized value of signal in error log
 *
 * Revision 1.23  2021-07-27 12:36:03+05:30  Cprogrammer
 * add feature to run ./init once before ./run
 *
 * Revision 1.22  2021-07-24 20:27:00+05:30  Cprogrammer
 * display in logs if child is stopped
 *
 * Revision 1.21  2021-06-06 10:14:57+05:30  Cprogrammer
 * indicate service name in logs when supervised service exits/crashes
 *
 * Revision 1.20  2021-04-27 14:16:23+05:30  Cprogrammer
 * do not treat error_exist as error for mkdir
 *
 * Revision 1.19  2021-04-07 22:37:08+05:30  Cprogrammer
 * allow extra parent_dir argument for log process
 *
 * Revision 1.18  2020-11-12 11:27:38+05:30  Cprogrammer
 * initialize svpid in main()
 *
 * Revision 1.17  2020-11-11 09:27:23+05:30  Cprogrammer
 * pass exit/signal of exited child to alert
 *
 * Revision 1.16  2020-11-10 19:11:24+05:30  Cprogrammer
 * maintain pid of supervise in down state and status of down state in byte 20
 *
 * Revision 1.15  2020-11-09 21:32:09+05:30  Cprogrammer
 * avoid recreating status file with every invocation of supervise.
 *
 * Revision 1.14  2020-11-09 18:17:57+05:30  Cprogrammer
 * recover from fork error - http://cr-yp-to.996295.n3.nabble.com/posible-bug-in-daemontools-td16964.html
 *
 * Revision 1.13  2020-11-09 09:30:24+05:30  Cprogrammer
 * add wait for service feature
 *
 * Revision 1.12  2020-11-07 14:30:35+05:30  Cprogrammer
 * run alert script on abnormal exit of service
 *
 * Revision 1.11  2020-10-08 18:33:24+05:30  Cprogrammer
 * use /run, /var/run if system supports it
 *
 * Revision 1.10  2020-09-16 19:07:40+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.9  2020-08-29 11:50:27+05:30  Cprogrammer
 * send signal to process id after sending signal to process group id
 *
 * Revision 1.8  2020-08-29 08:38:51+05:30  Cprogrammer
 * new option 'G' to send signal to entire process group
 *
 * Revision 1.7  2020-06-10 17:37:21+05:30  Cprogrammer
 * new option 'r' to restart (stop & start) a service
 *
 * Revision 1.6  2020-06-08 22:52:16+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.5  2008-06-07 13:53:31+05:30  Cprogrammer
 * added signals SIGQUIT, SIGUSR2
 *
 * Revision 1.4  2008-05-28 11:19:58+05:30  Cprogrammer
 * added wait for shutdown script
 *
 * Revision 1.3  2004-10-24 21:40:06+05:30  Cprogrammer
 * added prototype for rename()
 *
 * Revision 1.2  2004-10-22 20:31:13+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-25 23:49:19+05:30  Cprogrammer
 * Initial revision
 *
 */
