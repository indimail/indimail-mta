/*
 * $Log: svstat.c,v $
 * Revision 1.9  2020-11-30 22:56:02+05:30  Cprogrammer
 * continue instead of exit on run_init() failure
 *
 * Revision 1.8  2020-11-10 19:13:45+05:30  Cprogrammer
 * use byte 20 from status to indicate if service is up
 *
 * Revision 1.7  2020-11-09 09:31:09+05:30  Cprogrammer
 * display wait status. print errors to stderr instead of stdout
 *
 * Revision 1.6  2020-11-07 14:23:49+05:30  Cprogrammer
 * print pid after displaying uptime
 *
 * Revision 1.5  2020-10-08 18:34:17+05:30  Cprogrammer
 * use /run, /var/run if system supports it
 *
 * Revision 1.4  2020-09-27 14:16:09+05:30  Cprogrammer
 * display status written by svwait command
 *
 * Revision 1.3  2004-10-22 20:31:20+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-09 23:35:02+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.1  2003-12-31 18:36:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "fmt.h"
#include "tai.h"
#include "substdio.h"
#ifdef USE_RUNFS
#include "run_init.h"
#endif

#define FATAL "svstat: fatal: "
#define WARN  "svstat: warning: "

char            outbuf[256], errbuf[256];
substdio        o = SUBSTDIO_FDBUF(write, 1, outbuf, sizeof outbuf);
substdio        e = SUBSTDIO_FDBUF(write, 2, errbuf, sizeof errbuf);

char            status[21];
char            strnum[FMT_ULONG];

unsigned long   pid;
unsigned char   normallyup, want, paused;
static short    waiting;

void
doit(char *dir, int *retval)
{
	struct stat     st;
	int             r, fd;
	short          *s;
	char           *x;
	struct tai      when, now;

	*retval = 111;
	if (chdir(dir) == -1) {
		x = error_str(errno);
		substdio_puts(&e, dir);
		substdio_puts(&e, ": unable to chdir: ");
		substdio_puts(&e, x);
		substdio_puts(&e, "\n");
		return;
	}

	normallyup = 0;
	if (stat("down", &st) == -1) {
		if (errno != error_noent) {
			x = error_str(errno);
			substdio_puts(&e, dir);
			substdio_puts(&e, ": unable to stat down: ");
			substdio_puts(&e, x);
			substdio_puts(&e, "\n");
			return;
		}
		normallyup = 1;
	}

#ifdef USE_RUNFS
	switch (run_init(dir))
	{
	case 0:
		break;
	case -1:
		strerr_warn2(WARN, "unable to get current working directory: ", &strerr_sys);
		return;
	case -2:
		strerr_warn4(WARN, "unable to chdir to ", dir, ": ", &strerr_sys);
		return;
	}
#endif
	if ((fd = open_write("supervise/ok")) == -1) {
		if (errno == error_nodevice) {
			substdio_puts(&e, dir);
			substdio_puts(&e, ": supervise not running\n");
			*retval = 2;
			return;
		}
		x = error_str(errno);
		substdio_puts(&e, dir);
		substdio_puts(&e, ": unable to open supervise/ok: ");
		substdio_puts(&e, x);
		substdio_puts(&e, "\n");
		return;
	}
	close(fd);
	if ((fd = open_read("supervise/status")) == -1) {
		if (errno == error_noent)
			*retval = 2;
		x = error_str(errno);
		substdio_puts(&e, dir);
		substdio_puts(&e, ": unable to open supervise/status: ");
		substdio_puts(&e, x);
		substdio_puts(&e, "\n");
		return;
	}
	r = read(fd, status, sizeof status);
	close(fd);
	if (r < sizeof status) {
		if (r == -1)
			x = error_str(errno);
		else {
			*retval = 100;
			x = "bad format";
		}
		substdio_puts(&e, dir);
		substdio_puts(&e, ": unable to read supervise/status: ");
		substdio_puts(&e, x);
		substdio_puts(&e, "\n");
		return;
	}
	pid = (unsigned char) status[15];
	pid <<= 8;
	pid += (unsigned char) status[14];
	pid <<= 8;
	pid += (unsigned char) status[13];
	pid <<= 8;
	pid += (unsigned char) status[12];
	if ((paused = status[16]))
		*retval = 3;
	want = status[17];
	s = (short *) (status + 18);
	waiting = *s;
	tai_unpack(status, &when);
	tai_now(&now);
	if (tai_less(&now, &when))
		when = now;
	tai_sub(&when, &now, &when);
	substdio_puts(&o, dir);
	substdio_puts(&o, ": ");
	if (waiting)
		substdio_puts(&o, "wait ");
	else
	if (status[20])
		substdio_puts(&o, "up ");
	else
		substdio_puts(&o, "down ");
	substdio_put(&o, strnum, fmt_ulong(strnum, tai_approx(&when)));
	substdio_puts(&o, " seconds");

	if (status[20] && !normallyup)
		substdio_puts(&o, ", normally down");
	if (!pid && normallyup)
		substdio_puts(&o, ", normally up");
	if (status[20] && paused)
		substdio_puts(&o, ", paused");
	if (!pid && (want == 'u')) {
		*retval = 4;
		substdio_puts(&o, ", want up");
	}
	if (status[20] && (want == 'd')) {
		*retval = 5;
		substdio_puts(&o, ", want down");
	}

	if (pid && status[20]) {
		if (*retval != 3 && *retval != 5)
			*retval = 0;
		substdio_puts(&o, " pid ");
		substdio_put(&o, strnum, fmt_ulong(strnum, pid));
		substdio_puts(&o, " ");
	} else
	if (pid) {
		*retval = 1;
		substdio_puts(&o, " spid ");
		substdio_put(&o, strnum, fmt_ulong(strnum, pid));
		substdio_puts(&o, " ");
	} else
		*retval = 1;
	if (waiting > 0 && (waiting - when.x) > 0) {
		substdio_puts(&o, "remaining ");
		when.x = waiting - when.x;
		substdio_put(&o, strnum, fmt_ulong(strnum, tai_approx(&when)));
		substdio_puts(&o, " seconds");
	}
	substdio_puts(&o, "\n");
}

int
main(int argc, char **argv)
{
	int             fdorigdir, retval;
	char           *dir;

	++argv;

	if ((fdorigdir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	while ((dir = *argv++)) {
		doit(dir, &retval);
		if (fchdir(fdorigdir) == -1)
			strerr_die2sys(111, FATAL, "unable to revert directory: ");
	}
	substdio_flush(&e);
	substdio_flush(&o);
	/*
	 * 111 - system error
	 * 100 - bad status
	 *   0 - service running
	 *   1 - service down
	 *   2 - supervise not running
	 *   3 - service paused
	 *   4 - down - wants up
	 *   5 - up   - wants down
	 *   6 - wait - wating for service to come up
	 */
	_exit(retval);
}

void
getversion_svstat_c()
{
	static char    *x = "$Id: svstat.c,v 1.9 2020-11-30 22:56:02+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
