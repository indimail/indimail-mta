/*
 * $Log: svstat.c,v $
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

#define FATAL "svstat: fatal: "
#define WARNING "svstat: warning: "

char            bspace[1024];
substdio        b = SUBSTDIO_FDBUF(write, 1, bspace, sizeof bspace);

char            status[18];
char            strnum[FMT_ULONG];

unsigned long   pid;
unsigned char   normallyup;
unsigned char   want;
unsigned char   paused;

void
doit(char *dir, int *retval)
{
	struct stat     st;
	int             r, fd;
	char           *x;
	struct tai      when, now;
	char            buf[80];

	*retval = 1;
	substdio_puts(&b, dir);
	substdio_puts(&b, ": ");
	if (chdir(dir) == -1) {
		x = error_str(errno);
		substdio_puts(&b, "unable to chdir: ");
		substdio_puts(&b, x);
		return;
	}

	normallyup = 0;
	if (stat("down", &st) == -1) {
		if (errno != error_noent) {
			x = error_str(errno);
			substdio_puts(&b, "unable to stat down: ");
			substdio_puts(&b, x);
			return;
		}
		normallyup = 1;
	}

	if ((fd = open_write("supervise/ok")) == -1) {
		if (errno == error_nodevice) {
			substdio_puts(&b, "supervise not running");
			return;
		}
		x = error_str(errno);
		substdio_puts(&b, "unable to open supervise/ok: ");
		substdio_puts(&b, x);
		return;
	}
	close(fd);
	if ((fd = open_read("supervise/status")) == -1) {
		x = error_str(errno);
		substdio_puts(&b, "unable to open supervise/status: ");
		substdio_puts(&b, x);
		return;
	}
	r = read(fd, status, sizeof status);
	close(fd);
	if (r < sizeof status) {
		if (r == -1)
			x = error_str(errno);
		else
			x = "bad format";
		substdio_puts(&b, "unable to read supervise/status: ");
		substdio_puts(&b, x);
		return;
	}
	pid = (unsigned char) status[15];
	pid <<= 8;
	pid += (unsigned char) status[14];
	pid <<= 8;
	pid += (unsigned char) status[13];
	pid <<= 8;
	pid += (unsigned char) status[12];
	paused = status[16];
	want = status[17];
	tai_unpack(status, &when);
	tai_now(&now);
	if (tai_less(&now, &when))
		when = now;
	tai_sub(&when, &now, &when);
	if (pid) {
		substdio_puts(&b, "up (pid ");
		substdio_put(&b, strnum, fmt_ulong(strnum, pid));
		substdio_puts(&b, ") ");
		*retval = 0;
	} else
		substdio_puts(&b, "down ");
	substdio_put(&b, strnum, fmt_ulong(strnum, tai_approx(&when)));
	substdio_puts(&b, " seconds");

	if (pid && !normallyup)
		substdio_puts(&b, ", normally down");
	if (!pid && normallyup)
		substdio_puts(&b, ", normally up");
	if (pid && paused)
		substdio_puts(&b, ", paused");
	if (!pid && (want == 'u'))
		substdio_puts(&b, ", want up");
	if (pid && (want == 'd'))
		substdio_puts(&b, ", want down");
/*--------------------------------------------------------*/
	if (stat("supervise/wait", &st) == -1) {
		if (errno != error_noent) {
			x = error_str(errno);
			substdio_puts(&b, "unable to stat supervise/wait: ");
			substdio_puts(&b, x);
			return;
		}
	} else {
		if ((fd = open_read("supervise/wait")) == -1) {
			x = error_str(errno);
			substdio_puts(&b, "unable to open supervise/wait: ");
			substdio_puts(&b, x);
			return;
		}
		substdio_puts(&b, "\n");
		for (;;) {
			if ((r = read(fd, buf, sizeof(buf))) == -1) {
				x = error_str(errno);
				close(fd);
				substdio_puts(&b, "unable to read supervise/wait: ");
				substdio_puts(&b, x);
				return;
			} else
			if (!r)
				break;
			substdio_put(&b, buf, r);
		}
		close(fd);
	}
/*--------------------------------------------------------*/
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
		substdio_puts(&b, "\n");
		if (fchdir(fdorigdir) == -1)
			strerr_die2sys(111, FATAL, "unable to set directory: ");
	}
	substdio_flush(&b);
	_exit(retval);
}

void
getversion_svstat_c()
{
	static char    *x = "$Id: svstat.c,v 1.4 2020-09-27 14:16:09+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
