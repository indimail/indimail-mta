/*
 * $Log: supervise.c,v $
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

#define FATAL   "supervise: fatal: "
#define WARNING "supervise: warning: "

int             rename(const char *, const char *);

char           *dir;
int             selfpipe[2];
int             fdlock;
int             fdcontrolwrite;
int             fdcontrol;
int             fdok;
int             flagexit = 0;
int             flagwant = 1;
int             flagwantup = 1;
int             pid = 0;		/*- 0 means down */
int             flagpaused;		/*- defined if (pid) */
char            status[18];
#ifdef USE_RUNFS
char           *sdir;
int             fddir;
stralloc        run_service_dir = { 0 };
#endif

void
pidchange(void)
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
}

void
announce(void)
{
	int             fd;
	int             r;

	status[16] = (pid ? flagpaused : 0);
	status[17] = (flagwant ? (flagwantup ? 'u' : 'd') : 0);
	if ((fd = open_trunc("supervise/status.new")) == -1) {
		strerr_warn4(WARNING, "unable to open ", dir, "/supervise/status.new: ", &strerr_sys);
		return;
	}
	if ((r = write(fd, status, sizeof status)) == -1) {
		strerr_warn4(WARNING, "unable to write ", dir, "/supervise/status.new: ", &strerr_sys);
		close(fd);
		return;
	}
	close(fd);
	if (r < sizeof status) {
		strerr_warn4(WARNING, "unable to write ", dir, "/supervise/status.new: partial write", 0);
		return;
	}
	if (rename("supervise/status.new", "supervise/status") == -1)
		strerr_warn4(WARNING, "unable to rename ", dir, "/supervise/status.new to status: ", &strerr_sys);
}

void
trigger(void)
{
	if (write(selfpipe[1], "", 1) == -1)
		;
}

char           *run[2] =      { "./run", 0 };
char           *shutdown[3] = { "./shutdown", 0, 0 };
char           *alert[3] = { "./alert", 0, 0 };

void
trystart(void)
{
	int             f;

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
		execve(*run, run, environ);
#ifdef USE_RUNFS
		strerr_die4sys(111, FATAL, "unable to start ", sdir, "/run: ");
#else
		strerr_die4sys(111, FATAL, "unable to start ", dir, "/run: ");
#endif
	}
	flagpaused = 0;
	pid = f;
	pidchange();
	announce();
	deepsleep(1);
}

void
tryaction(char **action, pid_t spid)
{
	int             f;
	int             wstat;
	char            strnum[FMT_ULONG];

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
		strnum[fmt_ulong(strnum, spid)] = 0;
		action[1] = strnum;
		execve(*action, action, environ);
#ifdef USE_RUNFS
		strerr_die6sys(111, FATAL, "unable to exec ", sdir, "/", *action, ": ");
#else
		strerr_die6sys(111, FATAL, "unable to exec ", dir, "/", *action, ": ");
#endif
	default:
		wait_pid(&wstat, f);
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

	announce();

	for (;;) {
		if (flagexit && !pid)
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
			if ((r == -1) && (errno != error_intr))
				break;
			if (r == pid) {
				pid = 0;
				pidchange();
				announce();
				if (flagexit)
					return;
				if (flagwant && flagwantup) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*alert, F_OK))
						tryaction(alert, r);
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					trystart();
				}
				break;
			}
		}
		if (read(fdcontrol, &ch, 1) == 1) {
			switch (ch)
			{
			case 'G':
				g = 1;
				break;
			case 'r':
				flagwant = 1;
				flagwantup = 1;
				if (pid) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, pid); /*- run the shutdown command */
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					kill(g ? 0 - pid : pid, SIGTERM);
					kill(g ? 0 - pid : pid, SIGCONT);
					if (g) {
						kill(pid, SIGTERM);
						kill(pid, SIGCONT);
					}
					g = flagpaused = 0;
				}
				t = pid;
				while (c < 10) {
					if (kill(pid, 0) && errno == error_srch) {
						t = 0;
						break;
					}
					c++;
					usleep(100);
				}
				announce();
				if (!t)
					trystart();
				break;
			case 'd':
				flagwant = 1;
				flagwantup = 0;
				if (pid) {
#ifdef USE_RUNFS
					if (fchdir(fddir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to service directory: ");
#endif
					if (!access(*shutdown, F_OK))
						tryaction(shutdown, pid); /*- run the shutdown command */
#ifdef USE_RUNFS
					if (chdir(dir) == -1)
						strerr_die2sys(111, FATAL, "unable to switch back to run directory: ");
#endif
					kill(g ? 0 - pid : pid, SIGTERM);
					kill(g ? 0 - pid : pid, SIGCONT);
					if (g) {
						kill(pid, SIGTERM);
						kill(pid, SIGCONT);
					}
					g = flagpaused = 0;
				}
				announce();
				break;
			case 'u':
				flagwant = 1;
				flagwantup = 1;
				announce();
				if (!pid)
					trystart();
				break;
			case 'o':
				flagwant = 0;
				announce();
				if (!pid)
					trystart();
				break;
			case 'a':
				if (pid) {
					kill(g ? 0 - pid : pid, SIGALRM);
					if (g)
						kill(pid, SIGALRM);
					g = 0;
				}
				break;
			case 'h':
				if (pid) {
					kill(g ? 0 - pid : pid, SIGHUP);
					if (g)
						kill(pid, SIGHUP);
					g = 0;
				}
				break;
			case 'k':
				if (pid) {
					kill(g ? 0 - pid : pid, SIGKILL);
					if (g)
						kill(pid, SIGKILL);
					g = 0;
				}
				break;
			case 't':
				if (pid) {
					kill(g ? 0 - pid : pid, SIGTERM);
					if (g)
						kill(pid, SIGTERM);
					g = 0;
				}
				break;
			case 'i':
				if (pid) {
					kill(g ? 0 - pid : pid, SIGINT);
					if (g)
						kill(pid, SIGINT);
					g = 0;
				}
				break;
			case 'q':
				if (pid) {
					kill(g ? 0 - pid : pid,SIGQUIT);
					if (g)
						kill(pid,SIGQUIT);
					g = 0;
				}
				break;
			case 'U':
			case '1':
				if (pid) {
					kill(g ? 0 - pid : pid,SIGUSR1);
					if (g)
						kill(pid,SIGUSR1);
					g = 0;
				}
				break;
			case '2':
				if (pid) {
					kill(g ? 0 - pid : pid,SIGUSR2);
					if (g)
						kill(pid,SIGUSR2);
					g = 0;
				}
				break;
			case 'p':
				flagpaused = 1;
				announce();
				if (pid) {
					kill(g ? 0 - pid : pid, SIGSTOP);
					if (g)
						kill(pid, SIGSTOP);
					g = 0;
				}
				break;
			case 'c':
				flagpaused = 0;
				announce();
				if (pid) {
					kill(g ? 0 - pid : pid, SIGCONT);
					if (g)
						kill(pid, SIGCONT);
					g = 0;
				}
				break;
			case 'x':
				flagexit = 1;
				announce();
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
	char            buf[256];
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
		if (!(parent_dir = env_get("SV_PWD"))) {
			if (!getcwd(buf, 255))
				strerr_die2sys(111, FATAL, "unable to get current working directory: ");
			i = str_rchr(buf, '/');
			if (!buf[i])
				strerr_die2sys(111, FATAL, "unable to get current working directory: ");
			parent_dir = buf + i + 1;
		}
		if (!stralloc_cats(&run_service_dir, parent_dir))
			strerr_die2x(111, FATAL, "out of memory");
	} 
	if (!stralloc_cats(&run_service_dir, service_dir) ||
			!stralloc_0(&run_service_dir))
		strerr_die2x(111, FATAL, "out of memory");
	if (chdir(run_dir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", run_dir, ": ");
#if 0
	if (access("svscan", F_OK) && mkdir("svscan", 0755) == -1)
		strerr_die4sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan: ");
#endif
	if (chdir("svscan") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan: ");

	if (parent_dir) { /*- we were called as supervise log */
		if (access(parent_dir, F_OK)) {
			if (mkdir(parent_dir, 0755) == -1)
				strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", parent_dir, ": ");
			if (chown(parent_dir, own, grp) == -1)
				strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, ": ");
		}
		if (chdir(parent_dir) == -1)
			strerr_die6sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan/", parent_dir, ": ");
		if (access("log", F_OK)) {
			if (mkdir("log", mode) == -1)
				strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", parent_dir, "/log: ");
			if (chown("log", own, grp) == -1)
				strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", parent_dir, "/log: ");
		}
	} else
	if (access(service_dir, F_OK)) {
		if (mkdir(service_dir, mode) == -1)
			strerr_die6sys(111, FATAL, "unable to mkdir ", run_dir, "/svscan/", service_dir, ": ");
		if (chown(service_dir, own, grp) == -1)
			strerr_die6sys(111, FATAL, "unable to set permssions for ", run_dir, "/svscan/", service_dir, ": ");
	}
	if (chdir(service_dir) == -1)
		strerr_die6sys(111, FATAL, "unable to chdir to ", run_dir, "/svscan/", service_dir, ": ");
	dir = run_service_dir.s;
	return;
}
#endif

int
main(int argc, char **argv)
{
	struct stat     st;

	if (!(dir = argv[1]) || argv[2])
		strerr_die1x(100, "supervise: usage: supervise dir");
	if (pipe(selfpipe) == -1)
		strerr_die4sys(111, FATAL, "unable to create pipe for ", dir, ": ");
	coe(selfpipe[0]);
	coe(selfpipe[1]);
	ndelay_on(selfpipe[0]);
	ndelay_on(selfpipe[1]);
	sig_block(sig_child);
	sig_catch(sig_child, trigger);
	if (chdir(dir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", dir, ": ");
	if (stat("down", &st) != -1)
		flagwantup = 0;
	else
	if (errno != error_noent)
		strerr_die4sys(111, FATAL, "unable to stat ", dir, "/down: ");
#ifdef USE_RUNFS
	if ((fddir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (stat(".", &st) == -1)
		strerr_die2sys(111, FATAL, "unable to stat current directory: ");
	sdir = dir; /*- original service direcory */
	initialize_run(dir, st.st_mode, st.st_uid, st.st_gid); /*- this will set dir to new service directory in /run, /var/run */
#endif
	mkdir("supervise", 0700);
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
	pidchange();
	announce();
	fifo_make("supervise/ok", 0600);
	if ((fdok = open_read("supervise/ok")) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", dir, "/supervise/ok: ");
	coe(fdok);
	if (!flagwant || flagwantup)
		trystart();
	doit();
	announce();
	_exit(0);
}

void
getversion_supervise_c()
{
	static char    *x = "$Id: supervise.c,v 1.12 2020-11-07 14:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
