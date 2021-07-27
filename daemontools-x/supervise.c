/*
 * $Log: supervise.c,v $
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
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

#define FATAL   "supervise: fatal: "
#define WARNING "supervise: warning: "

static char    *dir;
static int      selfpipe[2];
static int      fdlock;
static int      fdcontrolwrite;
static int      fdcontrol;
static int      fdstatus;
static int      fdok;
static int      flagexit = 0;
static int      flagwant = 1;
static int      flagwantup = 1;
static int      flagpaused;		/*- defined if (pid) */
static int      fddir;
static char     waited, flagfailed;
static char     status[21];
static char     pwdbuf[256];
static stralloc wait_sv_file = {0};
#ifdef USE_RUNFS
static char    *sdir;
static stralloc run_service_dir = { 0 };
#endif
static pid_t    childpid = 0;	/*- 0 means down */
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
	status[17] = (flagwant ? (flagwantup ? 'u' : 'd') : 0);
	if (sleep_interval) {
		s = (short *) (status + 18);
		*s = sleep_interval;
	} else
		status[18] = status[19] = 0;

	if (seek_begin(fdstatus)) {
		strerr_warn2(WARNING, "lseek: status: ", &strerr_sys);
		return;
	}
	if ((r = write(fdstatus, status, sizeof status)) == -1) {
		strerr_warn4(WARNING, "unable to write ", dir, "/supervise/status: ", &strerr_sys);
		return;
	}
	if (r < sizeof status) {
		strerr_warn4(WARNING, "unable to write ", dir, "/supervise/status: partial write: ", 0);
		return;
	}
}

void
trigger(void)
{
	if (write(selfpipe[1], "", 1) == -1)
		;
}

char           *run[2] = { "./run", 0 };
char           *shutdown[3] = { "./shutdown", 0, 0 };
char           *alert[5] = { "./alert", 0, 0, 0, 0 }; /*- alert pid chilld_exit_value signal_value */

int
get_wait_params(unsigned short *interval, char **sv_name)
{
	int             fd, match, i;
	static ushort   sleep_interval = 60;
	unsigned long   t;
	struct substdio ssin;
	static stralloc tline = { 0 };
	char            inbuf[256];

	if (tline.len) {
		*sv_name = tline.s;
		*interval = sleep_interval;
	}
	*interval = 0;
	*sv_name = (char *) 0;
	if ((fd = open_read("wait")) == -1) {
		if (errno == error_noent)
			return 2;
		strerr_die2sys(111, FATAL, "unable to open wait: ");
		return -1;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof (inbuf));
	for (i = 0;i < 2; i++) {
		if (getln(&ssin, &tline, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input");
		if (!tline.len)
			break;
		if (match) {
			tline.len--;
			tline.s[tline.len] = 0;
		} else {
			if (!stralloc_0(&tline))
				strerr_die2x(111, FATAL, "out of memory");
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
	return 0;
}

void
do_wait()
{
	int             fd, i, r;
	unsigned short  sleep_interval = 60;
	unsigned long   wpid;
	char            wstatus[20];
	char           *service_name;

#ifdef USE_RUNFS
	if (fchdir(fddir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
	i = get_wait_params(&sleep_interval, &service_name);
#ifdef USE_RUNFS
	if (chdir(dir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
	if (i)
		return;
	if (!stralloc_copyb(&wait_sv_file, "../", 3) ||
			!stralloc_cats(&wait_sv_file, service_name) ||
			!stralloc_catb(&wait_sv_file, "/supervise/status", 17) ||
			!stralloc_0(&wait_sv_file))
		strerr_die2x(111, FATAL, "out of memory");
	pidchange(svpid, 0);
	for (;;) {
		announce(-1);
		if ((fd = open_read(wait_sv_file.s)) == -1) {
			if (errno != error_noent)
				strerr_warn4(WARNING, "unable to open ", wait_sv_file.s, ": ", &strerr_sys);
			sleep(1);
			waited = 0;
			continue;
		}
		if ((r = read(fd, wstatus, sizeof wstatus)) == -1) {
			strerr_warn4(WARNING, "unable to read ", wait_sv_file.s, ": ", &strerr_sys);
			close(fd);
			sleep(1);
			waited = 0;
			continue;
		} else
		if (!r) {
			strerr_warn4(WARNING, "file ", wait_sv_file.s, " is of zero bytes: ", &strerr_sys);
			close(fd);
			sleep(1);
			waited = 0;
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
		waited = 0;
	} /*- for (i = -1;;) */
	if (!waited) {
		waited = 1;
		pidchange(svpid, 0); /*- update start time */
		announce(sleep_interval);
		deepsleep(sleep_interval);
		childpid = 0;
		announce(0);
	}
	return;
}

void
trystart(void)
{
	int             f;

	do_wait();
	switch (f = fork())
	{
	case -1:
#ifdef USE_RUNFS
		strerr_warn4(WARNING, "unable to fork for ", sdir, ", sleeping 60 seconds: ", &strerr_sys);
#else
		strerr_warn4(WARNING, "unable to fork for ", dir, ", sleeping 60 seconds: ", &strerr_sys);
#endif
		deepsleep(60);
		trigger();
		flagfailed = 1;
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
#ifdef USE_RUNFS
		if (fchdir(fddir) == -1)
			strerr_die2sys(111, FATAL, "unable to set current directory: ");
#endif
		execve(*run, run, environ);
#ifdef USE_RUNFS
		strerr_die4sys(111, FATAL, "unable to start ", sdir, "/run: ");
#else
		strerr_die4sys(111, FATAL, "unable to start ", dir, "/run: ");
#endif
	}
	flagpaused = 0;
	pidchange((childpid = f), 1);
	announce(0);
	deepsleep(1);
}

void
tryaction(char **action, pid_t spid, int wstat, int do_alert)
{
	int             f, t, i;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG + 1];

	switch (f = fork())
	{
	case -1:
#ifdef USE_RUNFS
		strerr_warn4(WARNING, "unable to fork for ", sdir, ", sleeping 60 seconds: ", &strerr_sys);
#else
		strerr_warn4(WARNING, "unable to fork for ", dir, ", sleeping 60 seconds: ", &strerr_sys);
#endif
		deepsleep(60);
		trigger();
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
#ifdef USE_RUNFS
		if (fchdir(fddir) == -1)
			strerr_die2sys(111, FATAL, "unable to set current directory: ");
#endif
		strnum1[fmt_ulong(strnum1, spid)] = 0;
		action[1] = strnum1;
		if (do_alert) {
			if (WIFSTOPPED(wstat) || WIFSIGNALED(wstat)) {
				action[3] = "stopped/signalled";
				strnum2[fmt_uint(strnum2, WIFSTOPPED(wstat) ? WSTOPSIG(wstat) : WTERMSIG(wstat))] = 0;
			} else
			if (WIFEXITED(wstat)) {
				action[3] = "exited";
				if ((t = WEXITSTATUS(wstat)) < 0) {
					i = fmt_uint(strnum2 + 1, 0 - t);
					*strnum2 = '-';
					strnum2[i + 1] = 0;
				} else
					strnum2[fmt_uint(strnum2, t)] = 0;
			}
			action[2] = strnum2;
		} else
			action[2] = NULL;
		execve(*action, action, environ);
#ifdef USE_RUNFS
		strerr_die6sys(111, FATAL, "unable to exec ", sdir, "/", *action, ": ");
#else
		strerr_die6sys(111, FATAL, "unable to exec ", dir, "/", *action, ": ");
#endif
	default:
		wait_pid(&t, f);
	}
}

void
doit(void)
{
	iopause_fd      x[2];
	struct taia     deadline;
	struct taia     stamp;
	int             wstat, r, t, g = 0;
	char            ch, c = 0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *ptr;

	announce(0);

	for (;;) {
		if (flagexit && !childpid)
			return;

		sig_unblock(sig_child);

		x[0].fd = selfpipe[0];
		x[0].events = IOPAUSE_READ;
		x[1].fd = fdcontrol;
		x[1].events = IOPAUSE_READ;
		taia_now(&stamp);
		taia_uint(&deadline, 3600);
		taia_add(&deadline, &stamp, &deadline);
		iopause(x, 2, &deadline, &stamp);

		sig_block(sig_child);

		while (read(selfpipe[0], &ch, 1) == 1) ;
		for (;;) {
			if (!(r = wait_nohang(&wstat)))
				break;
			if (r == -1 && errno != error_intr)
				break;
			if (r == childpid) { /*- process exited */
				strnum1[fmt_ulong(strnum1, childpid)] = 0;
				childpid = 0;
				pidchange(svpid, 0);
				announce(0);
				t = str_rchr(dir, '/');
				ptr = dir[t] ? dir + t + 1 : dir;
				if (wait_stopped(wstat)) {
					t = wait_stopsig(wstat);
					strerr_warn7(WARNING, "pid: ", strnum1, " service ", ptr, " stopped by signal ", strnum2, 0);
				} else
				if (wait_crashed(wstat)) {
					strerr_warn6(WARNING, "pid: ", strnum1, " service ", ptr, " crashed", 0);
				} else {
					t = wait_exitcode(wstat);
					strnum2[fmt_ulong(strnum2, t)] = 0;
					strerr_warn7(WARNING, "pid: ", strnum1, " service ", ptr, " exited with status=", strnum2, 0);
				}
				if (flagexit)
					return;
				if (flagwant && flagwantup) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*alert, F_OK))
						tryaction(alert, r, wstat, 1);
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					trystart();
				}
				break;
			}
		} /*- for (;;) */
		if (flagfailed && flagwant && flagwantup) {
			flagfailed = 0;
			trystart();
		}
		if (read(fdcontrol, &ch, 1) == 1) {
			switch (ch)
			{
			case 'G': /*- send signal to process group */
				g = 1;
				break;
			case 'r': /*- restart */
				flagwant = 1;
				flagwantup = 1;
				if (childpid) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, childpid, 0, 0); /*- run the shutdown command */
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					kill(g ? 0 - childpid : childpid, SIGTERM);
					kill(g ? 0 - childpid : childpid, SIGCONT);
					if (g) {
						kill(childpid, SIGTERM);
						kill(childpid, SIGCONT);
					}
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
					trystart();
				break;
			case 'd': /*- down */
				flagwant = 1;
				flagwantup = 0;
				if (childpid) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, childpid, 0, 0); /*- run the shutdown command */
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					kill(g ? 0 - childpid : childpid, SIGTERM);
					kill(g ? 0 - childpid : childpid, SIGCONT);
					if (g) {
						kill(childpid, SIGTERM);
						kill(childpid, SIGCONT);
					}
					g = flagpaused = 0;
				}
				announce(0);
				break;
			case 'u': /*- up */
				flagwant = 1;
				flagwantup = 1;
				announce(0);
				if (!childpid)
					trystart();
				break;
			case 'o': /*- run once */
				flagwant = 0;
				announce(0);
				if (!childpid)
					trystart();
				break;
			case 'a': /*- send ALRM */
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGALRM);
					if (g)
						kill(childpid, SIGALRM);
					g = 0;
				}
				break;
			case 'h': /*- send HUP */
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGHUP);
					if (g)
						kill(childpid, SIGHUP);
					g = 0;
				}
				break;
			case 'k': /*- send kill */
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGKILL);
					if (g)
						kill(childpid, SIGKILL);
					g = 0;
				}
				break;
			case 't': /*- send TERM */
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGTERM);
					if (g)
						kill(childpid, SIGTERM);
					g = 0;
				}
				break;
			case 'i': /*- send INT */
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGINT);
					if (g)
						kill(childpid, SIGINT);
					g = 0;
				}
				break;
			case 'q': /*- send SIGQUIT */
				if (childpid) {
					kill(g ? 0 - childpid : childpid,SIGQUIT);
					if (g)
						kill(childpid,SIGQUIT);
					g = 0;
				}
				break;
			case 'U': /*- send USR1 */
			case '1':
				if (childpid) {
					kill(g ? 0 - childpid : childpid,SIGUSR1);
					if (g)
						kill(childpid,SIGUSR1);
					g = 0;
				}
				break;
			case '2': /*- send USR2 */
				if (childpid) {
					kill(g ? 0 - childpid : childpid,SIGUSR2);
					if (g)
						kill(childpid,SIGUSR2);
					g = 0;
				}
				break;
			case 'p': /*- send STOP */
				flagpaused = 1;
				announce(0);
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGSTOP);
					if (g)
						kill(childpid, SIGSTOP);
					g = 0;
				}
				break;
			case 'c': /*- send CONT */
				flagpaused = 0;
				announce(0);
				if (childpid) {
					kill(g ? 0 - childpid : childpid, SIGCONT);
					if (g)
						kill(childpid, SIGCONT);
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
initialize_run(char *service_dir, mode_t mode, uid_t own, gid_t grp)
{
	char           *run_dir, *parent_dir = (char *) 0;
	int             i;

	if (!access("/run", F_OK)) {
		run_dir = "/run";
		i = 4;
	} else
	if (!access("/var/run", F_OK)) {
		run_dir = "/var/run";
		i = 8;
	} else
		return;
	if (!stralloc_copyb(&run_service_dir, run_dir, i) ||
			!stralloc_catb(&run_service_dir, "/svscan/", 8))
		strerr_die2x(111, FATAL, "out of memory");

	if (!str_diff(service_dir, "log")) {
		/*- SV_PWD is the dirname of directory in /service e.g. qmail-smtpd.25 or log */
		if (!(parent_dir = env_get("SV_PWD"))) {
			if (!getcwd(pwdbuf, 255))
				strerr_die2sys(111, FATAL, "unable to get current working directory: ");
			i = str_rchr(pwdbuf, '/');
			if (!pwdbuf[i])
				strerr_die2sys(111, FATAL, "unable to get current working directory: ");
			parent_dir = pwdbuf + i + 1;
		}
		if (!stralloc_cats(&run_service_dir, parent_dir) ||
				!stralloc_append(&run_service_dir, "/"))
			strerr_die2x(111, FATAL, "out of memory");
	} 
	/*- full path /run/svscan/service/qmail-smtpd.25 or /run/svscan/service/qmail-smtpd.25/log */
	if (!stralloc_cats(&run_service_dir, service_dir) ||
			!stralloc_0(&run_service_dir))
		strerr_die2x(111, FATAL, "out of memory");
	if (chdir(run_dir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", run_dir, ": ");
	if (access("svscan", F_OK) && mkdir("svscan", 0755) == -1)
		strerr_die4sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan: ");
	if (chdir("svscan") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan: ");

	if (parent_dir) { /*- we were called as supervise log */
		if (access(parent_dir, F_OK)) {
			if (mkdir(parent_dir, 0755) == -1 && errno != error_exist)
				strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", parent_dir, ": ");
			if (chown(parent_dir, own, grp) == -1)
				strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, ": ");
		}
		if (chdir(parent_dir) == -1)
			strerr_die6sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan/", parent_dir, ": ");
		if (access("log", F_OK)) {
			if (mkdir("log", mode) == -1 && errno != error_exist)
				strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", parent_dir, "/log: ");
			if (chown("log", own, grp) == -1)
				strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, "/log: ");
		}
	} else
	if (access(service_dir, F_OK)) {
		if (mkdir(service_dir, mode) == -1 && errno != error_exist)
			strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", service_dir, ": ");
		if (chown(service_dir, own, grp) == -1)
			strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", service_dir, ": ");
	}
	if (chdir(service_dir) == -1) /*- e.g. qmail-smtpd.25 or log */
		strerr_die6sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan/", service_dir, ": ");
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
		strerr_warn4(WARNING, "unable to fork for ", dir, ", sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		flagfailed = 1;
		return -1;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
		execve(*init, init, environ);
		strerr_die4sys(111, FATAL, "unable to start ", dir, "/init: ");
	}
	if (wait_pid(&t, f) == -1) {
		strerr_warn4(WARNING, "wait failed ", dir, "/init: ", &strerr_sys);
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

	if (!(dir = argv[1]) || argc > 3)
		strerr_die1x(100, "supervise: usage: supervise dir [log_parent]");
	if (*dir == '/' || *dir == '.')
		strerr_die1x(100, "supervise: dir cannot start with '/' or '.'");
	if (pipe(selfpipe) == -1)
		strerr_die4sys(111, FATAL, "unable to create pipe for ", dir, ": ");
	coe(selfpipe[0]);
	coe(selfpipe[1]);
	ndelay_on(selfpipe[0]);
	ndelay_on(selfpipe[1]);
	sig_block(sig_child);
	sig_catch(sig_child, trigger);

	if (!(ptr = env_get("SERVICEDIR"))) {
		if (!getcwd(pwdbuf, 255))
			strerr_die2sys(111, FATAL, "unable to get current working directory: ");
		ptr = pwdbuf;
	}
	if (!stralloc_copys(&init_sv_path, ptr) ||
			!stralloc_append(&init_sv_path, "/") ||
			!stralloc_cats(&init_sv_path, dir) ||
			!stralloc_append(&init_sv_path, "/") ||
			!stralloc_catb(&init_sv_path, "init", 4) ||
			!stralloc_0(&init_sv_path))
		strerr_die2x(111, FATAL, "out of memory");
	init[0] = init_sv_path.s;

	if (chdir(dir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", dir, ": ");
	if (stat("down", &st) != -1)
		flagwantup = 0;
	else
	if (errno != error_noent)
		strerr_die4sys(111, FATAL, "unable to stat ", dir, "/down: ");

	if ((fddir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
#ifdef USE_RUNFS
	if (stat(".", &st) == -1)
		strerr_die2sys(111, FATAL, "unable to stat current directory: ");
	/*-
	 * save original directory in sdir since
	 * dir gets modified in initialize_run()
	 */
	sdir = dir;
	initialize_run(dir, st.st_mode, st.st_uid, st.st_gid); /*- this will set dir to new service directory in /run, /var/run */
#endif
	if (mkdir("supervise", 0700) && errno != error_exist) 
		strerr_die2sys(111, FATAL, "unable to create supervise directory: ");

	if ((fdstatus = open_trunc("supervise/status")) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", dir, "/supervise/status: ");
	coe(fdstatus);

	fdlock = open_append("supervise/lock");
	if ((fdlock == -1) || (lock_exnb(fdlock) == -1))
		strerr_die4sys(111, FATAL, "unable to acquire ", dir, "/supervise/lock: ");
	coe(fdlock);

	fifo_make("supervise/control", 0600);
	if ((fdcontrol = open_read("supervise/control")) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", dir, "/supervise/control: ");
	coe(fdcontrol);
	ndelay_on(fdcontrol); /*- shouldn't be necessary */
	if ((fdcontrolwrite = open_write("supervise/control")) == -1)
		strerr_die4sys(111, FATAL, "unable to write ", dir, "/supervise/control: ");
	coe(fdcontrolwrite);

	pidchange((svpid = getpid()), 0);
	announce(0);

	fifo_make("supervise/ok", 0600);
	if ((fdok = open_read("supervise/ok")) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", dir, "/supervise/ok: ");
	coe(fdok);

	/*
	 * By now we have finished initialization of /run. We
	 * can now run scripts that will not fail due to absense of
	 * directories/files in /run
	 */
	while (1) {
		if (!access(*init, X_OK)) {
			if (do_init()) {
				strerr_warn4(WARNING, "initialization failed for ", dir, ", sleeping 60 seconds: ", 0);
				deepsleep(60);
			} else
				break;
		} else
			break;
	}

	if (!flagwant || flagwantup)
		trystart();
	doit();
	announce(0);
	_exit(0);
}

void
getversion_supervise_c()
{
	static char    *x = "$Id: supervise.c,v 1.23 2021-07-27 12:36:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
