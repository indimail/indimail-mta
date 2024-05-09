/*
 * $Log: svc.c,v $
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
#include <unistd.h>
#include "ndelay.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "sgetopt.h"
#include "substdio.h"
#include "byte.h"
#include "sig.h"
#ifdef USE_RUNFS
#include "run_init.h"
#endif

#define FATAL "svc: fatal: "
#define WARN   "svc: warning: "

int             datalen = 0;
char            data[20];

substdio        b;
char            bspace[1];

int             fdorigdir;

int
main(int argc, char **argv)
{
	int             opt, i;
	int             fd, exit_stat;
	char           *dir;

	sig_ignore(sig_pipe);
	while ((opt = getopt(argc, argv, "udropchUaitkxq12G")) != opteof) {
		if (opt == '?')
			strerr_die1x(100,
				 "svc options: u up, d down, r restart, o once, x exit, p pause, c continue h hup\n"
				 "             a alarm, i interrupt, t term, k kill, q quit, U|1 SIGUSR1, 2 SIGUSR2\n"
				 "             G process group");
		else
		if (datalen < sizeof data) {
			if (byte_chr(data, datalen, opt) == datalen)
				data[datalen++] = opt;
		}
	}
	argv += optind;
	if ((fdorigdir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	exit_stat = 0;
	while ((dir = *argv++)) {
		if (chdir(dir) == -1) {
			strerr_warn4(WARN, "unable to chdir to ", dir, ": ", &strerr_sys);
			continue;
		}
		fd = -1;
#ifdef USE_RUNFS
		if ((fd = open_write("supervise/control")) == -1) {
			if (errno == error_nodevice) {
				strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
				exit_stat = 2;
				continue;
			}
			if (errno != error_noent) {
				strerr_warn4(WARN, "unable to open ", dir, "/supervise/control: ", &strerr_sys);
				exit_stat = 1;
				continue;
			}
			switch ((i = run_init(dir)))
			{
			case 0: /*- cwd changed to /run/svscan/service_name */
				break;
			case 1: /*- /run, /var/run doesn't exist */
				strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
				exit_stat = 2;
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
					strerr_die2sys(111, FATAL, "unable to revert directory: ");
				continue;
			}
		}
#endif
		if (fd == -1 && (fd = open_write("supervise/control")) == -1) {
			if (errno == error_nodevice || errno == error_noent) {
				strerr_warn4(WARN, "unable to control ", dir, ": supervise not running: ", 0);
				exit_stat = 2;
			} else
				strerr_warn4(WARN, "unable to control ", dir, ": ", &strerr_sys);
		} else {
			ndelay_off(fd);
			substdio_fdbuf(&b, write, fd, bspace, sizeof bspace);
			if (substdio_putflush(&b, data, datalen) == -1)
				strerr_warn4(WARN, "error writing commands to ", dir, ": ", &strerr_sys);
			close(fd);
		}
		if (fchdir(fdorigdir) == -1)
			strerr_die2sys(111, FATAL, "unable to revert directory: ");
	} /* while ((dir = *argv++)) */
	_exit(exit_stat);
}

void
getversion_svc_c()
{
	const char     *x = "$Id: svc.c,v 1.10 2023-03-04 14:42:52+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
