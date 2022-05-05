/*
 * $Log: svscan.c,v $
 * Revision 1.24  2022-05-06 01:29:01+05:30  Cprogrammer
 * enable auto scan if AUTOSCAN env variable is set or if service startup fails
 *
 * Revision 1.23  2021-10-20 22:32:20+05:30  Cprogrammer
 * enable scan on sigchld
 *
 * Revision 1.22  2021-08-11 21:29:32+05:30  Cprogrammer
 * added handler for SIGCHLD when running as PID 1
 *
 * Revision 1.21  2021-07-27 12:33:28+05:30  Cprogrammer
 * set SERVICEDIR, PWD environment variable to service directory
 *
 * Revision 1.20  2021-04-16 12:24:13+05:30  Cprogrammer
 * disable service in run filesystem when disabled in original service directory
 *
 * Revision 1.19  2021-04-11 12:15:24+05:30  Cprogrammer
 * display parent name as argv2 for log process
 *
 * Revision 1.18  2020-10-09 11:42:28+05:30  Cprogrammer
 * renamed svscan.pid to .svscan.pid
 *
 * Revision 1.17  2020-10-08 18:29:45+05:30  Cprogrammer
 * use /run, /var/run if available
 *
 * Revision 1.16  2020-09-19 20:24:35+05:30  Cprogrammer
 * call setsid() if SETSID environment variable is set
 *
 * Revision 1.15  2020-07-11 22:08:52+05:30  Cprogrammer
 * removed STATUSFILE code
 *
 * Revision 1.14  2020-07-04 16:35:45+05:30  Cprogrammer
 * write pid as a string on the second line of .svlock
 *
 * Revision 1.13  2020-05-11 10:58:12+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.12  2020-03-22 08:03:58+05:30  Cprogrammer
 * return error on deletion of .svlock only when file exists
 *
 * Revision 1.11  2020-03-21 23:54:02+05:30  Cprogrammer
 * improved code for get_lock()
 *
 * Revision 1.10  2019-12-08 18:21:29+05:30  Cprogrammer
 * fixed svscan lock failure when run as pid 1 in docker container
 *
 * Revision 1.9  2017-05-11 14:24:27+05:30  Cprogrammer
 * run .svscan/shutdown on SIGTERM
 *
 * Revision 1.8  2017-05-11 13:30:34+05:30  Cprogrammer
 * added option to run initialization command via INITCMD env variable
 *
 * Revision 1.7  2017-04-18 08:59:53+05:30  Cprogrammer
 * lock service directory using file .svlock to prevent multiple svscan runs
 *
 * Revision 1.6  2011-08-05 14:31:05+05:30  Cprogrammer
 * create STATUSFILE on startup and delete on shutdown
 *
 * Revision 1.5  2011-05-26 23:17:52+05:30  Cprogrammer
 * open svscan log only if SCANLOG is defined
 *
 * Revision 1.4  2008-06-07 13:54:28+05:30  Cprogrammer
 * added logging of svscan through external loggers like multilog
 *
 * Revision 1.3  2004-10-22 20:31:19+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2003-10-11 09:31:58+05:30  Cprogrammer
 * added scan.h for scan_ulong() prototype
 * changed wait to unsigned long
 *
 * Revision 1.1  2003-10-11 09:28:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direntry.h>
#include <strerr.h>
#include <error.h>
#include <wait.h>
#include <coe.h>
#include <fd.h>
#include <env.h>
#include <str.h>
#include <byte.h>
#include <scan.h>
#include <sig.h>
#include <fmt.h>
#include <pathexec.h>
#include <stralloc.h>
#include "auto_sysconfdir.h"

#define SERVICES 1000
#define PIDFILE  ".svscan.pid"

#define WARNING "svscan: warning: "
#define FATAL   "svscan: fatal: "
#define INFO    "svscan: info: "

#ifndef SVSCANINFO
#define SVSCANINFO ".svscan"  /* must begin with dot ('.') */
#endif

int             rename(char *, const char *);

struct
{
	unsigned long   dev;
	unsigned long   ino;
	int             flagactive;
	int             flaglog;
	int             pid;		/*- 0 if not running */
	int             pidlog;		/*- 0 if not running */
	int             pi[2];		/*- defined if flaglog */
} x[SERVICES];
static int      numx = 0, scannow = 0;
static char     fnlog[260];
static char    *pidfile, *p_exe_name;
static stralloc tmp = {0};

#ifdef USE_RUNFS
void
initialize_run()
{
	if (!access("/run", F_OK)) {
		pidfile = "/run/svscan/"PIDFILE;
		if (access("/run/svscan", F_OK) && mkdir("/run/svscan", 0755) == -1)
			strerr_die2sys(111, FATAL, "unable to mkdir /run/svscan: ");
	} else
	if (!access("/var/run", F_OK)) {
		pidfile = "/var/run/svscan/"PIDFILE;
		if (access("/var/run/svscan", F_OK) && mkdir("/var/run/svscan", 0755) == -1)
			strerr_die2sys(111, FATAL, "unable to mkdir /var/run/svscan: ");
	} else {
		pidfile = PIDFILE;
		return;
	}
}
#endif

void
init_cmd(char *cmmd, int dowait, int shutdown)
{
	int             child, r, wstat;
	pid_t           pid;
	char           *cpath, *args[4];
	char            strnum[FMT_ULONG];

	cpath = shutdown ? SVSCANINFO"/shutdown" : cmmd && *cmmd ? cmmd : SVSCANINFO"/run";
	if (access(cpath, X_OK))
		return;
	pid = getpid();
	switch (child = fork())
	{
	case -1:
		strerr_warn4(WARNING, "unable to fork for init command ", cpath, ": ", &strerr_sys);
		return;
	case 0:
		args[0] = "/bin/sh";
		args[1] = "-c";
		args[2] = cpath;
		args[3] = 0;
		strnum[fmt_ulong(strnum, pid)] = 0;
		if (!env_put2("PPID", strnum))
			strerr_die3x(111, WARNING, cpath, "init_cmd: out of memory");
		execv(*args, args);
		strerr_die4sys(111, WARNING, "unable to run", cpath, ": ");
	default:
		break;
	}
	for (;;) {
		r = dowait ? wait_pid(&wstat, child) : wait_nohang(&wstat);
		if (!r || (dowait && r == child))
			break;
		if (r == -1) {
			if (errno == error_intr)
				continue;		/*- impossible */
			break;
		}
	}
	return;
}

void
start(char *fn, char *sdir)
{
	unsigned int    fnlen;
	struct stat     st;
	int             child, i, fdsource;
	pid_t           pid;
	char           *run_dir;
	char           *args[4];
	char            strnum[FMT_ULONG];

	if (fn[0] == '.' && str_diff(fn, SVSCANINFO)) {
		if (!fn[1] || fn[1] == '.') /*- . and .. */
			return;
		if (!access("/run", F_OK))
			run_dir = "/run/svscan";
		else
		if (!access("/var/run", F_OK))
			run_dir = "/var/run/svscan";
		else
			return;
		if ((fdsource = open(".", O_RDONLY|O_NDELAY, 0)) == -1)
			strerr_die2sys(111, FATAL, "unable to open current directory: ");
		if (chdir(run_dir) == -1) {
			strerr_warn4(WARNING, "unable to switch to ", run_dir, ": ", &strerr_sys);
			close(fdsource);
			scannow = -1;
			return;
		}
		if (!access(fn, F_OK)) { /*- no need to rename */
			if (fchdir(fdsource) == -1)
				strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
			close(fdsource);
			return;
		}
		/*
		 * if if rename a directory in orignal /service, we should rename
		 * it here.
		 * e.g. if /service/qmail-smtpd.25 is renamed to /service/.qmail-smtpd.25
		 * then rename /run/svscan/qmail-smtpd.25 to /run/svscan/.qmail-smtpd.25
		 */
		if (!access(fn + 1, F_OK) && rename(fn + 1, fn)) {
			strerr_warn6(WARNING, "unable to rename ", fn + 1, " to ", fn, ": ", &strerr_sys);
			scannow = -1;
		}
		if (fchdir(fdsource) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsource);
		return;
	}
	if (stat(fn, &st) == -1) {
		strerr_warn4(WARNING, "unable to stat ", fn, ": ", &strerr_sys);
		scannow = -1;
		return;
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR)
		return;
	for (i = 0; i < numx; ++i) {
		if (x[i].ino == st.st_ino && x[i].dev == st.st_dev)
			break; /*- existing service */
	}
	if (i == numx) { /*- we found a new service to start */
		if (numx >= SERVICES) {
			strerr_warn4(WARNING, "unable to start ", fn, ": running too many services", 0);
			return;
		}
		x[i].ino = st.st_ino;
		x[i].dev = st.st_dev;
		/*- 
		 * (fn[0] == '.' possible only if fn is SVSCANINFO.
		 * if so supervise ./log subdir only)
		 */
		x[i].pid = (fn[0] != '.') ? 0 : -1;
		x[i].pidlog = 0;
		x[i].flaglog = 0;
		fnlen = str_len(fn);
		if (fnlen <= 255) {
			byte_copy(fnlog, fnlen, fn);
			byte_copy(fnlog + fnlen, 5, "/log");
			if (stat(fnlog, &st) == 0)
				x[i].flaglog = 1;
			else
			if (errno != error_noent) {
				strerr_warn4(WARNING, "unable to stat ", fn, "/log: ", &strerr_sys);
				scannow = -1;
				return;
			}
		}
		if (x[i].flaglog) {
			if (pipe(x[i].pi) == -1) {
				strerr_warn4(WARNING, "unable to create pipe for ", fn, ": ", &strerr_sys);
				scannow = -1;
				return;
			}
			coe(x[i].pi[0]);
			coe(x[i].pi[1]);
		}
		++numx;
	}
	x[i].flagactive = 1;
	pid = getpid();
	if (!x[i].pid) { /*- exec supervise fn only if it is not .svscan/log */
		switch (child = fork())
		{
		case -1:
			strerr_warn4(WARNING, "unable to fork for ", fn, ": ", &strerr_sys);
			return;
		case 0:
			if (x[i].flaglog)
				if (fd_move(1, x[i].pi[1]) == -1)
					strerr_die4sys(111, WARNING, "unable to set up descriptors for ", fn, ": ");
			args[0] = "supervise";
			args[1] = fn;
			args[2] = 0;
			strnum[fmt_ulong(strnum, pid)] = 0;
			if (!env_put2("PPID", strnum))
				strerr_die3x(111, WARNING, fn, ": out of memory");
			pathexec_run(*args, args, environ);
			strerr_die4sys(111, WARNING, "unable to start supervise ", fn, ": ");
		default:
			x[i].pid = child;
		}
	}
	if (x[i].flaglog && !x[i].pidlog) { /*- exec supervise log */
		switch (child = fork())
		{
		case -1:
			strerr_warn4(WARNING, "unable to fork for ", fn, "/log: ", &strerr_sys);
			return;
		case 0:
			if (fd_move(0, x[i].pi[0]) == -1)
				strerr_die4sys(111, WARNING, "unable to set up descriptors for ", fn, "/log: ");
			if (chdir(fn) == -1)
				strerr_die4sys(111, WARNING, "unable to switch to ", fn, ": ");
			if (!env_put2("SV_PWD", fn))
				strerr_die4x(111, WARNING, "out of memory for ", fn, "/log");
			args[0] = "supervise";
			args[1] = "log";
			args[2] = fn;
			args[3] = 0;
			strnum[fmt_ulong(strnum, pid)] = 0;
			if (!env_put2("PPID", strnum))
				strerr_die3x(111, WARNING, fn, ": out of memory");
			pathexec_run(*args, args, environ);
			strerr_die4sys(111, FATAL, "unable to start supervise ", fn, "/log: ");
		default:
			x[i].pidlog = child;
		}
	}
}

void
doit(char *sdir, pid_t pid)
{
	DIR            *dir;
	direntry       *d;
	int             i, t, wstat;
	pid_t           r;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	for (;;) {
		if (!(r = wait_nohang(&wstat)))
			break;
		if (r == -1) {
			if (errno == error_intr)
				continue; /*- impossible */
			break;
		}
		for (i = 0; i < numx; ++i) {
			if (x[i].pid == r) {
				x[i].pid = 0;
				break;
			}
			if (x[i].pidlog == r) {
				x[i].pidlog = 0;
				break;
			}
		}
		if (pid != 1)
			continue;
		/*-
		 * this are not my children. Happens when
		 * svscan is running as pid 1 in a docker container
		 */
		strnum1[fmt_ulong(strnum1, r)] = 0;
		if (wait_stopped(wstat)) {
			t = wait_stopsig(wstat);
			strnum2[fmt_ulong(strnum2, t)] = 0;
			if (p_exe_name)
				strerr_warn7(WARNING, "pid: ", strnum1, " exe ", p_exe_name, ", stopped by signal ", strnum2, 0);
			else
				strerr_warn5(WARNING, "pid: ", strnum1, " stopped by signal ", strnum2, 0);
		} else
		if (wait_crashed(wstat)) {
			if (p_exe_name)
				strerr_warn6(WARNING, "pid: ", strnum1, " exe ", p_exe_name, ", crashed", 0);
			else
				strerr_warn4(WARNING, "pid: ", strnum1, ", crashed", 0);
		}
		else {
			t = wait_exitcode(wstat);
			strnum2[fmt_ulong(strnum2, t)] = 0;
			if (p_exe_name)
				strerr_warn7(WARNING, "pid: ", strnum1, " exe ", p_exe_name, ", exited with status=", strnum2, 0);
			else
				strerr_warn5(WARNING, "pid: ", strnum1, ", exited with status=", strnum2, 0);
		}
		if (r > 1 && i == numx)
			strerr_warn3(INFO, "completed last rites for orphan ", strnum1, 0);
	} /*- for (;;) */
	if (pid == 1 || !scannow)
		return;
	for (i = 0; i < numx; ++i)
		x[i].flagactive = 0;
	if (!(dir = opendir("."))) {
		strerr_warn2(WARNING, "unable to read directory: ", &strerr_sys);
		return;
	}
	for (;;) {
		errno = 0;
		if (!(d = readdir(dir)))
			break;
		start(d->d_name, sdir);
	}
	if (errno) {
		strerr_warn2(WARNING, "unable to read directory: ", &strerr_sys);
		closedir(dir);
		return;
	}
	closedir(dir);
	i = 0;
	while (i < numx) {
		if (!x[i].flagactive && !x[i].pid && !x[i].pidlog) {
			if (x[i].flaglog) {
				close(x[i].pi[0]);
				close(x[i].pi[1]);
				x[i].flaglog = 0;
			}
			x[i] = x[--numx];
			continue;
		}
		++i;
	}
}

static void
sighup(int i)
{
	scannow = 1;
	signal(SIGHUP, sighup);
	strerr_warn2(INFO, "got sighup", 0);
}

static void
sigterm(int i)
{
	unlink(pidfile);
	signal(SIGTERM, SIG_IGN);
	strerr_warn2(INFO, "Stopping svscan", 0);
	init_cmd(0, 1, 1); /*- run .svscan/shutdown */
	_exit(0);
}

static void
open_svscan_log(char *sdir)
{
	const int       i = numx;
	struct stat     st;
	static char     fn[] = SVSCANINFO;	/*- avoid compiler warning on const string */

	/*- (semi-paranoid; could be more so) */
	if (fstat(STDIN_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_RDONLY);
	if (fstat(STDOUT_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_WRONLY);
	if (fstat(STDERR_FILENO, &st) != 0 && errno == EBADF)
		(void) open("/dev/null", O_WRONLY);
	if (stat(fn, &st) == 0) {
		start(fn, sdir);
		if (i + 1 == numx && x[i].pidlog != 0) {
			(void) dup2(x[i].pi[1], STDOUT_FILENO);
			(void) dup2(x[i].pi[1], STDERR_FILENO);
			strerr_warn2(INFO, "Starting svscan", 0);
		}
	}
}

int
get_lock(char *sdir)
{
	int             fd, n, fdsource;
	pid_t           pid;
	char            strnum[FMT_ULONG], buf[8];

	if (1 == getpid()) { /*- we are running under a docker container as init */
		if (unlink(pidfile) == -1 && errno != error_noent)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
	}
	pid = -1;
	if ((fd = open(pidfile, O_CREAT|O_WRONLY|O_EXCL, 0644)) >= 0) {
		pid = getpid();
		strnum[n = fmt_ulong(strnum, pid)] = 0;
		if (write(fd, (char *) &pid, sizeof(pid_t)) == -1 || 
				write(fd, "\n", 1) == -1 || write(fd, strnum, n) == -1 ||
				write(fd, "\n", 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write pid to lock file: ");
		close(fd);
		return (0);
	}
	if (errno != error_exist)
		strerr_die4sys(111, FATAL, "unable to obtain lock for ", pidfile, ": ");
	/* .svlock exists */
	if ((fd = open(pidfile, O_RDONLY, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", pidfile, ": ");
	if (read(fd, (char *) &pid, sizeof(pid)) == -1)
		strerr_die2sys(111, FATAL, "unable to get pid from lock: ");
	close(fd);
	if (pid == getpid()) /*- we again got the same pid */
		return (0);
	errno = 0;
	if (pid == -1 || (kill(pid, 0) == -1 && errno == error_srch)) { /*- process does not exist */
		if (unlink(pidfile) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (errno)
		strerr_die4sys(111, FATAL, "unable to get status of pid ", strnum, ": ");

	if ((fdsource = open(".", O_RDONLY|O_NDELAY, 0)) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	/*- 
	 * let us find out if the process is svscan we will
	 * use the /proc filesystem to figure out command name
	 */
	if (chdir("/proc") == -1) /*- on systems without /proc filesystem, give up */
		strerr_die2sys(111, FATAL, "chdir: /proc: ");
	if (chdir(strnum) == -1) { /*- process is now dead */
		if (fchdir(fdsource) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsource);
		if (unlink(pidfile) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	/*- open 'comm' to get the command name */
	if ((fd = open("comm", O_RDONLY, 0)) == -1) { /*- process is now dead */
		if (fchdir(fdsource) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsource);
		if (unlink(pidfile) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	if ((n = read(fd, buf, 7)) != 7) { /*- non-svcan process is running with this pid */
		close(fd);
		if (fchdir(fdsource) == -1)
			strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
		close(fdsource);
		if (unlink(pidfile) == -1)
			strerr_die2sys(111, FATAL, "unable to delete lock: ");
		return (1);
	}
	close(fd);
	if (buf[0] == 's' && buf[1] == 'v' && buf[2] == 's' && 
		buf[3] == 'c' && buf[4] == 'a' && buf[5] == 'n' && buf[6] == '\n') { /*- indeed pid is svscan process */
		buf[6] = 0;
		strerr_warn5(FATAL, "[", buf, "] ", "already running", 0);
		_exit (111);
	}
	/*- some non-svscan process is running with pid */
	if (fchdir(fdsource) == -1)
		strerr_die4sys(111, FATAL, "unable to switch back to ", sdir, ": ");
	close(fdsource);
	if (unlink(pidfile) == -1)
		strerr_die2sys(111, FATAL, "unable to delete lock: ");
	return (1);
}

char *
get_proc_exename(pid_t pid)
{
	char            strnum[FMT_ULONG];
	static char     buf[128];
	int             n, fd;

	strnum[n = fmt_ulong(strnum, pid)] = 0;
	/*- use the /proc filesystem to figure out command name */
	if (!stralloc_copyb(&tmp, "/proc/", 6) ||
			!stralloc_catb(&tmp, strnum, n) ||
			!stralloc_catb(&tmp, "/comm", 5) ||
			!stralloc_0(&tmp))
		strerr_die2x(111, FATAL, "out of memory");
	if ((fd = open(tmp.s, O_RDONLY, 0)) == -1) /*- process is now dead */
		return ((char *) NULL);
	if ((n = read(fd, buf, sizeof(buf) - 1)) == -1) {
		close(fd);
		return ((char *) NULL);
	}
	close(fd);
	buf[n - 1] = 0; /*- remove newline */
	return buf;
}

static void
sigchld(int signum, siginfo_t *si, void *data)
{
	p_exe_name = get_proc_exename(si->si_pid);
	scannow = 2;
}

int
main(int argc, char **argv)
{
	unsigned long   scan_interval = 60;
	int             auto_scan;
	char           *s, *sdir;
	char            dirbuf[256];
	struct sigaction sa;
	pid_t           pid;

	/*- setup handler for sigchild if running as pid 1 */
	if (1 == (pid = getpid())) {
		byte_zero((char *) &sa, sizeof(struct sigaction));
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = sigchld;
		if (sigaction(sig_child, &sa, 0) == -1)
			strerr_die2sys(111, FATAL, "sigaction: unable to set signal handler for SIGCHLD");
	}
	/*- save the current dir */
#ifdef USE_RUNFS
	initialize_run();
#else
	pidfile = PIDFILE;
#endif
	if (argc > 1 && argv[1]) {
		if (chdir(argv[1]) == -1)
			strerr_die4sys(111, FATAL, "unable to chdir to ", argv[1], ": ");
		if (!((sdir = getcwd(dirbuf, 255))))
			strerr_die2sys(111, FATAL, "unable to get current working directory: ");
		while (get_lock(sdir)) ;
	} else {
#ifdef USE_RUNFS
		pidfile = PIDFILE;
#endif
		if (!((sdir = getcwd(dirbuf, 255))))
			strerr_die2sys(111, FATAL, "unable to get current working directory: ");
		while (get_lock(sdir)) ;
	}
	if (!env_put2("SERVICEDIR", sdir))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!env_put2("PWD", sdir))
		strerr_die2x(111, FATAL, "out of memory");
	if (env_get("SETSID"))
		(void) setsid();
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);
	if ((s = env_get("SCANINTERVAL")))
		scan_ulong(s, &scan_interval);
	if (env_get("SCANLOG"))
		open_svscan_log(sdir);
	if ((s = env_get("INITCMD")))
		init_cmd(s, env_get("WAIT_INITCMD") ? 1 : 0, 0);
	auto_scan = env_get("AUTOSCAN") ? 1 : 0;
	for (scannow = 1;;) {
		doit(sdir, pid);
		/*-
		 * we do not scan service directory unless we get a sighup,
		 * sigchld, auto_scan is set or we failed to start supervise
		 */
		if (scannow == -1) {
			sleep(scan_interval ? scan_interval : 60);
			continue;
		}
		scannow = 0;
		while (!scannow) {
			sleep(scan_interval ? scan_interval : 60);
			/* 
			 * on interruption by SIGCHLD or SIGHUP we either
			 * 1. reap the child
			 * or
			 * 2. scan the /service directory
			 */
			if (errno == EINTR)
				continue;
			if (auto_scan)
				break;
		}
	}
}

void
getversion_svscan_c()
{
	static char    *y = "$Id: svscan.c,v 1.24 2022-05-06 01:29:01+05:30 Cprogrammer Exp mbhangui $";

	y++;
}
