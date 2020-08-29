/*
 * $Log: svc.c,v $
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

#define FATAL "svc: fatal: "
#define WARNING "svc: warning: "

int             datalen = 0;
char            data[20];

substdio        b;
char            bspace[1];

int             fdorigdir;

int
main(int argc, char **argv)
{
	int             opt;
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
	fdorigdir = open_read(".");
	if (fdorigdir == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	exit_stat = 0;
	while ((dir = *argv++)) {
		if (chdir(dir) == -1)
			strerr_warn4(WARNING, "unable to chdir to ", dir, ": ", &strerr_sys);
		else {
			if((fd = open_write("supervise/control")) == -1) {
				if (errno == error_nodevice) {
					strerr_warn4(WARNING, "unable to control ", dir, ": supervise not running", 0);
					exit_stat = 2;
				} else
					strerr_warn4(WARNING, "unable to control ", dir, ": ", &strerr_sys);
			} else {
				ndelay_off(fd);
				substdio_fdbuf(&b, write, fd, bspace, sizeof bspace);
				if (substdio_putflush(&b, data, datalen) == -1)
					strerr_warn4(WARNING, "error writing commands to ", dir, ": ", &strerr_sys);
				close(fd);
			}
		}
		if (fchdir(fdorigdir) == -1)
			strerr_die2sys(111, FATAL, "unable to set directory: ");
	}
	_exit(exit_stat);
}

void
getversion_svc_c()
{
	static char    *x = "$Id: svc.c,v 1.6 2020-08-29 08:41:22+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
