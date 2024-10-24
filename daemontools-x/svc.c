/*
 * $Id: svc.c,v 1.12 2024-10-24 18:08:54+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <fcntl.h>
#include "sig.h"
#include "ndelay.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "sgetopt.h"
#include "substdio.h"
#include "subfd.h"
#include "qprintf.h"
#include "byte.h"
#include "sig.h"
#include "lock.h"
#include "scan.h"
#ifdef USE_RUNFS
#include "run_init.h"
#endif

#define FATAL "svc: fatal: "
#define WARN   "svc: warning: "

int             datalen = 0, fdorigdir, timeout = 0;
char            data[20], bspace[1];
substdio        b;

void
sigalrm()
{
	return;
}

int
doit(char **ptr, int *ret, int dlen, int opt)
{
	int             fd = -1, i;
	char           *dir;

	while ((dir = *ptr++)) {
		if (chdir(dir) == -1) {
			strerr_warn4(WARN, "unable to chdir to directory ", dir, ": ", &strerr_sys);
			continue;
		}
#ifdef USE_RUNFS
		if ((fd = open_write("supervise/control")) == -1) {
			if (errno == error_nodevice) {
				strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
				if (!*ret)
					*ret = 2;
				continue;
			}
			if (errno != error_noent) {
				strerr_warn4(WARN, "unable to open ", dir, "/supervise/control: ", &strerr_sys);
				if (!*ret)
					*ret = 1;
				continue;
			}
			switch ((i = run_init(dir)))
			{
			case 0: /*- cwd changed to /run/svscan/service_name */
				break;
			case 1: /*- /run, /var/run doesn't exist */
				strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
				if (!*ret)
					*ret = 2;
				continue;
			case -1:
				strerr_die2sys(111, FATAL, "unable to get current working directory: ");
				break;
			case -2:
				strerr_warn4(WARN, "No run state information for ", dir, ": ", &strerr_sys);
				continue;
			case -3:
				strerr_warn3(WARN, dir, ": name too long", 0);
				continue;
			}
			if (i) {
				if (fchdir(fdorigdir) == -1)
					strerr_die2sys(111, FATAL, "unable to revert to original directory: ");
				continue;
			}
		}
#endif
		if (dlen) {
			if (fd == -1 && (fd = open_write("supervise/control")) == -1) {
				if (errno == error_nodevice || errno == error_noent) {
					strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
					if (!*ret)
						*ret = 2;
				} else {
					if (!*ret)
						*ret = 1;
					strerr_warn4(WARN, "unable to control ", dir, ": ", &strerr_sys);
				}
			} else {
				ndelay_off(fd);
				substdio_fdbuf(&b, write, fd, bspace, sizeof bspace);
				if (substdio_putflush(&b, data, dlen) == -1)
					strerr_warn4(WARN, "error writing commands to ", dir, ": ", &strerr_sys);
				close(fd);
			}
		} else
		if (opt == 'w' || opt == 'W') {
			subprintf(subfderr, "waiting for %s to be %s ... ", dir, opt == 'w' ? "up" : "down");
			substdio_flush(subfderr);
			alarm(timeout);
			fd = open(opt == 'w' ? "supervise/up" : "supervise/dn", O_WRONLY, 0); /* this should block if service is down/up */
			alarm(0);
			if (fd == -1) {
				if (errno == error_nodevice || errno == error_noent) {
					strerr_warn4(WARN, "unable to wait for ", dir, ": ", &strerr_sys);
					sleep(1);
				} else
				if (errno = error_intr) {
					if (!*ret)
						*ret = 3;
					subprintf(subfderr, "timed out\n");
					substdio_flush(subfderr);
				} else {
					subprintf(subfderr, "failed\n");
					substdio_flush(subfderr);
					strerr_warn4(WARN, "unable to wait for ", dir, ": ", &strerr_sys);
				}
			} else { /*- service is up/down */
				subprintf(subfderr, "ok\n");
				substdio_flush(subfderr);
				close(fd);
			}
		}
		if (fchdir(fdorigdir) == -1)
			strerr_die2sys(111, FATAL, "unable to revert to original directory: ");
	} /* while ((dir = *argv++)) */
	return *ret;
}

int
main(int argc, char **argv)
{
	int             opt, sopt = 0, exit_stat = 0, do_sleep = 0;

	sig_ignore(sig_pipe);
	while ((opt = getopt(argc, argv, "udropchUaitkxq12GwWT:")) != opteof) {
		if (opt == '?')
			strerr_die1x(100,
				 "svc options: u up, d down, r restart, o once, x exit, p pause, c continue h hup\n"
				 "             a alarm, i interrupt, t term, k kill, q quit, U|1 SIGUSR1, 2 SIGUSR2\n"
				 "             G process group, w wait for up, W wait for down, T timeout");
		else
		if (datalen < sizeof data) {
			if ((opt != 'w' && opt != 'W') && byte_chr(data, datalen, opt) == datalen)
				data[datalen++] = opt;
		}
		if (opt == 'T')
			scan_int(optarg, &timeout);
		if (opt == 'u' || opt == 'd' || opt == 'r')
			do_sleep = 1;
		if (opt == 'w' || opt == 'W')
			sopt = opt;
	}
	argv += optind;
	sig_alarmcatch(sigalrm);
	if ((fdorigdir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (datalen)
		doit(argv, &exit_stat, datalen, sopt);
	if (sopt) {
		if (datalen && do_sleep)
			sleep(1);
		doit(argv, &exit_stat, 0, sopt);
	}
	_exit(exit_stat);
}

void
getversion_svc_c()
{
	const char     *x = "$Id: svc.c,v 1.12 2024-10-24 18:08:54+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: svc.c,v $
 * Revision 1.12  2024-10-24 18:08:54+05:30  Cprogrammer
 * added -w option to wait for service to be up
 * added -W option to wait for service to be down
 * added -T timeout option for -w, -W options
 *
 * Revision 1.11  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.10  2023-03-04 14:42:52+05:30  Cprogrammer
 * check for supervise/ok in original service dir before run filesystem
 *
 * Revision 1.9  2020-11-30 23:19:48+05:30  Cprogrammer
 * change warning text message for missing directory in /run
 *
 * Revision 1.8  2020-11-30 22:53:40+05:30  Cprogrammer
 * continue instead of exit on run_init() failure
 *
 * Revision 1.7  2020-10-08 18:34:02+05:30  Cprogrammer
 * use /run, /var/run if system supports it
 *
 * Revision 1.6  2020-08-29 08:41:22+05:30  Cprogrammer
 * new option 'G' to send signal to entire process group
 *
 * Revision 1.5  2020-06-10 17:37:42+05:30  Cprogrammer
 * new option restart (stop and restart) a service
 *
 * Revision 1.4  2008-06-07 19:11:45+05:30  Cprogrammer
 * added sigquit, sigusr1, sigusr2 signals
 *
 * Revision 1.3  2004-10-22 20:31:16+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-12 00:09:01+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.1  2003-12-31 18:37:19+05:30  Cprogrammer
 * Initial revision
 *
 */
