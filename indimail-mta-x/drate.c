/*
 * $Log: drate.c,v $
 * Revision 1.8  2021-05-26 10:34:22+05:30  Cprogrammer
 * refactored code and added test mode
 *
 * Revision 1.7  2021-05-23 07:07:23+05:30  Cprogrammer
 * use /etc/indimail/control/ratelimit as default ratelimit directory
 *
 * Revision 1.6  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.5  2015-08-20 18:34:42+05:30  Cprogrammer
 * exit 100 for invalid option
 *
 * Revision 1.4  2013-12-09 13:08:38+05:30  Cprogrammer
 * make the domain name as the file for storing delivery rate information
 *
 * Revision 1.3  2013-09-06 20:08:20+05:30  Cprogrammer
 * use the current time for calculating rate
 *
 * Revision 1.2  2013-09-05 09:25:36+05:30  Cprogrammer
 * added consolidate and reset option
 *
 * Revision 1.1  2013-08-29 18:27:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <substdio.h>
#include <error.h>
#include <fmt.h>
#include <sgetopt.h>
#include <evaluate.h>
#include <open.h>
#include <strerr.h>
#include <env.h>
#include <lock.h>
#include <getln.h>
#include <now.h>
#include <scan.h>
#include <str.h>
#include "variables.h"
#include "myctime.h"
#include "auto_uids.h"
#include "auto_control.h"
#include "get_rate.h"
#include "control.h"
#include "getDomainToken.h"
#include "report.h"

#define FATAL     "drate: fatal: "
#define WARN      "drate: warn: "

dtype           delivery;
int             local_time;
char            strnum[FMT_ULONG];
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
int             qmailr_uid;
int             qmail_gid;
char           *usage = "usage: drate [-scR] -d domain -r deliveryRate [-D ratelimit_dir]\n";

void
report(int e, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
	strerr_die6x(e, FATAL, s2, s3, s4, s5, s6);
}

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
my_error(char *s1, int exit_val)
{
	logerr(s1);
	logerrf("\n");
	_exit(exit_val);
}

void
cleanup(char *file, char *str, int e)
{
	if (e == error_noent)
		(void) unlink(file);
	if (file)
		strerr_die5sys(111, FATAL, str, ": ", file, ": ");
	else
		strerr_die3sys(111, FATAL, str, ": ");
}

stralloc        line = { 0 }, rexpr = { 0 };

void
do_display(char *domain)
{
	int             rfd, match, line_no;
	unsigned long   email_count = 0;
	struct substdio ssfin;
	char            inbuf[2048], strdouble[FMT_DOUBLE];
	double          rate;
	datetime_sec    starttime = 0, endtime = 0;

	if ((rfd = open_read(domain)) == -1)
		strerr_die3sys(111, "unable to read: ", domain, ": ");
	substdio_fdbuf(&ssfin, read, rfd, inbuf, sizeof(inbuf));
	for (line_no = 1;;line_no++) { /*- Line Processing */
		if (getln(&ssfin, &line, &match, DELIMITER[0]) == -1)
			strerr_die3sys(111, "unable to read: ", domain, ": ");
		if (!match && line.len == 0)
			break;
		line.len--;
		if (DELIMITER[0]) {
			if (!stralloc_0(&line))
				strerr_die2x(111, FATAL, "out of memory");
			line.len--;
		}
		switch (line_no)
		{
		case 1:
			if (substdio_put(&ssout, "Conf   Rate: ", 13))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_put(&ssout, line.s, line.len) == -1)
				strerr_die1sys(111, "unable to write: ");
			get_rate(line.s, &rate);
			strdouble[fmt_double(strdouble, rate, 10)] = 0;
			if (substdio_put(&ssout, " (", 2))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_puts(&ssout, strdouble))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_put(&ssout, ")\n", 2))
				strerr_die1sys(111, "unable to write: ");
			break;
		case 2:
			scan_ulong(line.s, (unsigned long *) &email_count);
			if (substdio_put(&ssout, "Email Count: ", 13))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_put(&ssout, line.s, line.len) == -1)
				strerr_die1sys(111, "unable to write: ");
			if (substdio_put(&ssout, "\n", 2))
				strerr_die1sys(111, "unable to write: ");
			break;
		case 3:
			scan_ulong(line.s, (unsigned long *) &starttime);
			if (substdio_put(&ssout, "Start  Time: ", 13))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_puts(&ssout, myctime(starttime)))
				strerr_die1sys(111, "unable to write: ");
			break;
		case 4:
			scan_ulong(line.s, (unsigned long *) &endtime);
			if (substdio_put(&ssout, "End    Time: ", 13))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_puts(&ssout, myctime(endtime)))
				strerr_die1sys(111, "unable to write: ");
			rate = 
				(starttime == endtime) ? 0 : (double) email_count / (double) (now() - starttime);
			strdouble[fmt_double(strdouble, rate, 10)] = 0;
			if (substdio_put(&ssout, "CurrentRate: ", 13))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_puts(&ssout, strdouble))
				strerr_die1sys(111, "unable to write: ");
			if (substdio_put(&ssout, "\n", 1))
				strerr_die1sys(111, "unable to write: ");
			break;
		}
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die1sys(111, "unable to write: ");
}

void
set_mode(char *domain, char *rate_expr, int reset_mode, int consolidate, int incr)
{
	int             rfd, wfd, match, line_no, t;
	unsigned long   email_count;
	char            stime[FMT_ULONG], etime[FMT_ULONG], ecount[FMT_ULONG],
					strdouble[FMT_DOUBLE], inbuf[2048], outbuf[1024];
	struct substdio ssfin, ssfout;
	double          rate;
	datetime_sec    starttime, endtime;

	if (reset_mode) {
		starttime = endtime = now();
		ecount[fmt_ulong(ecount, 0)] = 0;
		stime[fmt_ulong(stime, starttime)] = 0;
		stime[fmt_ulong(etime, endtime)] = 0;
	}
	if ((rfd = open_read(domain)) == -1 && errno != error_noent)
		strerr_die3sys(111, "unable to read: ", domain, ": ");
	else {
		if (rfd == -1) {
			if (reset_mode && !rate_expr)
				strerr_die3sys(111, "unable to read: ", domain, ": ");
			if (!stralloc_cats(&rexpr, rate_expr))
				strerr_die1sys(111, "out of mem1: ");
			starttime = endtime = now();
			ecount[fmt_ulong(ecount, 0)] = 0;
			stime[fmt_ulong(stime, starttime)] = 0;
			stime[fmt_ulong(etime, endtime)] = 0;
			goto new;
		}
		substdio_fdbuf(&ssfin, read, rfd, inbuf, sizeof(inbuf));
		for (line_no = 1;;line_no++) { /*- Line Processing */
			if (getln(&ssfin, &line, &match, DELIMITER[0]) == -1)
				strerr_die3sys(111, "unable to read: ", domain, ": ");
			if (!match && line.len == 0)
				break;
			line.len--;
			if (DELIMITER[0]) {
				if (!stralloc_0(&line))
					strerr_die2x(111, FATAL, "out of memory");
				line.len--;
			}
			if (line_no > 1 && reset_mode)
				break;
			switch (line_no)
			{
			case 1:
				/*- we are going to overwrite this */
				if (incr) {
					if (!stralloc_cat(&rexpr, &line))
						strerr_die2x(111, FATAL, "out of memory");
					if (!stralloc_cats(&rexpr, rate_expr))
						strerr_die2x(111, FATAL, "out of memory");
				} else
				if (rate_expr) {
					if (!stralloc_cats(&rexpr, rate_expr))
						strerr_die2x(111, FATAL, "out of memory");
				} else
				if (!stralloc_copy(&rexpr, &line))
					strerr_die2x(111, FATAL, "out of memory");
				break;
			case 2:
				scan_ulong(line.s, &email_count);
				ecount[fmt_ulong(ecount, email_count)] = 0;
				break;
			case 3:
				scan_ulong(line.s, (unsigned long *) &starttime);
				stime[fmt_ulong(stime, starttime)] = 0;
				break;
			case 4:
				scan_ulong(line.s, (unsigned long *) &endtime);
				stime[fmt_ulong(etime, endtime)] = 0;
				break;
			} /*- switch (line_no) */
		} /*- for (line_no = 1;;line_no++) */
	}
new:
	t = errno;
	qmailr_uid = auto_uidr;
	qmail_gid = auto_gidq;
	if ((wfd = open(domain, O_WRONLY|O_CREAT, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to write: ", domain, ": ");
	if (lock_ex(wfd) == -1)
		cleanup(domain, "unable to lock", t);
	if (chown(domain, auto_uidr, auto_gidq))
		cleanup(domain, "unable to chown", t);
	if (ftruncate(wfd, 0) == -1)
		cleanup(domain, "unable to truncate", t);
	substdio_fdbuf(&ssfout, write, wfd, outbuf, sizeof(outbuf));
	if (consolidate) {
		if (!stralloc_0(&rexpr))
			cleanup(0, "out of mem", t);
		rexpr.len--;
		get_rate(rexpr.s, &rate);
		strdouble[fmt_double(strdouble, rate, 10)] = 0;
		if (substdio_puts(&ssfout, strdouble))
			cleanup(domain, "unable to write", t);
	} else
	if (substdio_bput(&ssfout, rexpr.s, rexpr.len) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bput(&ssfout, DELIMITER, 1) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bputs(&ssfout, ecount) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bput(&ssfout, DELIMITER, 1) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bputs(&ssfout, stime) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bput(&ssfout, DELIMITER, 1) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bputs(&ssfout, etime) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_bput(&ssfout, DELIMITER, 1) == -1)
		cleanup(domain, "unable to write", t);
	if (substdio_flush(&ssfout) == -1)
		cleanup(domain, "unable to write", t);
	if (close(wfd) == -1)
		cleanup(domain, "unable to write", t);
	return;
}

void
do_test(char *domain)
{
	char           *rate_expr = 0;
	char            strdouble1[FMT_DOUBLE], strdouble2[FMT_DOUBLE];
	unsigned long   email_count;
	double          rate, conf_rate;

	if (!access(domain, W_OK) && !is_rate_ok(domain, 0, &email_count, &conf_rate, &rate)) {
		strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
		strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
		strnum[fmt_ulong(strnum, email_count)] = 0;
		strerr_die9x(111, WARN, "high email rate [", strdouble1, "/", strdouble2, "] emails=", strnum, " for ", domain);
	} else
	if (errno != error_noent)
		strerr_die4sys(111, FATAL, "open: ", domain, ": ");
	else
	if (!access("ratecontrol", R_OK)) {
		if (control_readfile(&line, "./ratecontrol", 0) == -1)
			strerr_die2sys(111, FATAL, "Unable to read ratecontrol: ");
		rate_expr = getDomainToken(domain, &line);
		if (!is_rate_ok(domain, rate_expr, &email_count, &conf_rate, &rate)) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum[fmt_ulong(strnum, email_count)] = 0;
			strerr_die9x(111, WARN, "high email rate [", strdouble1, "/", strdouble2, "] emails=", strnum, " for ", domain);
		}
	} else
	if (errno != error_noent)
		strerr_die2sys(111, FATAL, "open: ratecontrol: ");
	else
	if (!access(".global", W_OK) && !is_rate_ok(".global", 0, &email_count, &conf_rate, &rate)) {
		strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
		strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
		strnum[fmt_ulong(strnum, email_count)] = 0;
		strerr_die9x(111, WARN, "high email rate [", strdouble1, "/", strdouble2, "] emails=", strnum, " for ", domain);
	} else
	if (errno != error_noent)
		strerr_die2sys(111, FATAL, "open: .global: ");
	strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
	strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
	strnum[fmt_ulong(strnum, email_count)] = 0;
	strerr_die8x(0, "email rate [", strdouble1, "/", strdouble2, "] emails=", strnum, " for ", domain);
}

int
main(int argc, char **argv)
{
	int             ch, display = 0, incr = 0, consolidate = 0, 
					reset_mode = 0, test_mode = 0;
	char           *domain = 0, *rate_expr = 0, *ratelimit_dir = "ratelimit";

	while ((ch = getopt(argc, argv, "ltscRd:r:D:")) != sgoptdone) {
		switch (ch)
		{
		case 'l':
			local_time = 1;
			break;
		case 't':
			test_mode = 1;
			break;
		case 's':
			display = 1;
			break;
		case 'R':
			reset_mode = 1;
			break;
		case 'c':
			consolidate = 1;
			break;
		case 'd': /*- domain */
			domain = optarg;
			break;
		case 'r': /*- delivery rate */
			rate_expr = optarg;
			switch (*optarg)
			{
			case '+':
				incr = 1;
				break;
			case '-':
				incr = -1;
				break;
			}
			break;
		case 'D': /*- ratelimit_dir */
			ratelimit_dir = optarg;
			break;
		default:
			logerrf(usage);
			_exit (100);
			break;
		}
	}
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!domain) {
		logerrf("domain not specified\n");
		logerrf(usage);
		_exit (111);
	}
	if (chdir(auto_control) || chdir(ratelimit_dir))
		strerr_die4sys(111, FATAL, "unable to switch to ", ratelimit_dir, ": ");

	if (display)
		do_display(domain);
	else
	if (test_mode) {
		do_test(domain);
	} else {
		if (!reset_mode && !rate_expr) {
			logerrf("rate expression not specified\n");
			logerrf(usage);
			_exit(111);
		}
		set_mode(domain, rate_expr, reset_mode, consolidate, incr);
	}
	return (0);
}

void
getversion_drate_c()
{
	static char    *x = "$Id: drate.c,v 1.8 2021-05-26 10:34:22+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidgetdomainth;
	x = sccsidevalh;
	x = sccsidgetrateh;
	x = sccsidreporth;
	x++;
}
