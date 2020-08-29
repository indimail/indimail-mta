/*
 * $Log: supervise.c,v $
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

#define FATAL "supervise: fatal: "
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
	if (write(selfpipe[1], "", 1) == -1) ;
}

char           *run[2] =      { "./run", 0 };
char           *shutdown[2] = { "./shutdown", 0 };

void
trystart(void)
{
	int             f;

	switch (f = fork())
	{
	case -1:
		strerr_warn4(WARNING, "unable to fork for ", dir, ", sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
		execve(*run, run, environ);
		strerr_die4sys(111, FATAL, "unable to start ", dir, "/run: ");
	}
	flagpaused = 0;
	pid = f;
	pidchange();
	announce();
	deepsleep(1);
}

void
trystop(void)
{
	int             f;
	int             wstat;

	switch (f = fork())
	{
	case -1:
		strerr_warn4(WARNING, "unable to fork for ", dir, ", sleeping 60 seconds: ", &strerr_sys);
		deepsleep(60);
		trigger();
		return;
	case 0:
		sig_uncatch(sig_child);
		sig_unblock(sig_child);
		execve(*shutdown, shutdown, environ);
		strerr_die4sys(111, FATAL, "unable to exec ", dir, "/shutdown: ");
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
				if (flagwant && flagwantup)
					trystart();
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
					if (!access(*shutdown, F_OK))
						trystop(); /*- run the shutdown command */
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
					if (!access(*shutdown, F_OK))
						trystop(); /*- run the shutdown command */
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

int
main(int argc, char **argv)
{
	struct stat     st;

	dir = argv[1];
	if (!dir || argv[2])
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
	static char    *x = "$Id: supervise.c,v 1.9 2020-08-29 11:50:27+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
