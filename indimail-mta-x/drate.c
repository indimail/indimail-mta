/*
 * $Log: drate.c,v $
 * Revision 1.18  2021-08-22 00:12:29+05:30  Cprogrammer
 * fixed display variable setting
 *
 * Revision 1.17  2021-08-21 21:32:01+05:30  Cprogrammer
 * added list option
 * added option to display times in UTC
 * added domain name validation
 *
 * Revision 1.16  2021-06-10 10:52:41+05:30  Cprogrammer
 * fixed uninitialized variables in do_test()
 *
 * Revision 1.15  2021-06-05 12:48:27+05:30  Cprogrammer
 * display time_needed to reach configured rates
 *
 * Revision 1.14  2021-06-03 18:38:22+05:30  Cprogrammer
 * display email count if delivery had succeeded
 *
 * Revision 1.13  2021-06-01 10:42:03+05:30  Cprogrammer
 * display local date/time
 *
 * Revision 1.12  2021-06-01 01:51:44+05:30  Cprogrammer
 * removed report()
 *
 * Revision 1.11  2021-05-30 20:29:44+05:30  Cprogrammer
 * fixed owner and permission of rate definition files
 *
 * Revision 1.10  2021-05-29 23:37:12+05:30  Cprogrammer
 * refactored for slowq-send
 *
 * Revision 1.9  2021-05-26 16:30:44+05:30  Cprogrammer
 * added option to force update
 *
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
#include <dirent.h>
#include <regex.h>
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
#include <date822fmt.h>
#include <no_of_days.h>
#include "variables.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "do_rate.h"
#include "control.h"
#include "getDomainToken.h"

#define FATAL     "drate: fatal: "
#define WARN      "drate: warn: "

dtype           delivery;
static int      local_time = 1;
static char     strnum1[FMT_ULONG], strnum2[FMT_LONG];
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static stralloc qdir = { 0 };
static char    *usage = "usage: drate [-sulcR] [-t -C count] -d domain -r deliveryRate [-D ratelimit_dir]\n";
static char    *daytab[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static char    *montab[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

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

char           *
qtime_mess822(datetime_sec t)
{
	struct datetime dt;
	unsigned int    len;
	static char     result[33];

	datetime_tai(&dt, t);
	len = 0;
	len += fmt_str(result + len, daytab[dt.wday]);
	len += fmt_strn(result + len, ", ", 2);
	len += fmt_uint0(result + len, dt.mday, 2);
	result[len++] = ' ';
	len += fmt_str(result + len, montab[dt.mon]);
	result[len++] = ' ';
	len += fmt_uint(result + len, 1900 + dt.year);
	result[len++] = ' ';
	len += fmt_uint0(result + len, dt.hour, 2);
	result[len++] = ':';
	len += fmt_uint0(result + len, dt.min, 2);
	result[len++] = ':';
	len += fmt_uint0(result + len, dt.sec, 2);
	len += fmt_strn(result + len, " +0000\n", 7);
	result[len++] = 0;
	return result;
}

stralloc        line = { 0 }, rexpr = { 0 };

void
do_display(char *domain)
{
	int             rfd, match, line_no;
	unsigned long   email_count = 0;
	struct substdio ssfin;
	char            inbuf[2048], strdouble[FMT_DOUBLE], buf[DATE822FMT];
	double          conf_rate, rate;
	datetime_sec    starttime = 0, endtime = 0, time_needed;
	struct datetime dt;

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
			if (substdio_put(&ssout, "Conf   Rate: ", 13) == -1 ||
					substdio_put(&ssout, line.s, line.len) == -1)
				strerr_die1sys(111, "unable to write: ");
			get_rate(line.s, &conf_rate);
			strdouble[fmt_double(strdouble, conf_rate, 10)] = 0;
			if (substdio_put(&ssout, " (", 2) == -1 ||
					substdio_puts(&ssout, strdouble) == -1 ||
					substdio_put(&ssout, ")\n", 2) == -1)
				strerr_die1sys(111, "unable to write: ");
			break;
		case 2:
			scan_ulong(line.s, (unsigned long *) &email_count);
			if (substdio_put(&ssout, "Email Count: ", 13) == -1 ||
					substdio_put(&ssout, line.s, line.len) == -1 ||
					substdio_put(&ssout, "\n", 2) == -1)
				strerr_die1sys(111, "unable to write: ");
			break;
		case 3:
			scan_ulong(line.s, (unsigned long *) &starttime);
			if (substdio_put(&ssout, "Start  Time: ", 13) == -1)
				strerr_die1sys(111, "unable to write: ");
			if (local_time) {
				datetime_tai(&dt, starttime);
				if (substdio_put(&ssout, buf, date822fmt(buf, &dt)) == -1)
					strerr_die1sys(111, "unable to write: ");
			} else {
				if (substdio_puts(&ssout, qtime_mess822(starttime)))
					strerr_die1sys(111, "unable to write: ");
			}
			break;
		case 4:
			scan_ulong(line.s, (unsigned long *) &endtime);
			if (substdio_put(&ssout, "LastUpdated: ", 13) == -1)
				strerr_die1sys(111, "unable to write: ");
			if (local_time) {
			datetime_tai(&dt, endtime);
			if (substdio_put(&ssout, buf, date822fmt(buf, &dt)) == -1)
				strerr_die1sys(111, "unable to write: ");
			} else {
				if (substdio_puts(&ssout, qtime_mess822(endtime)))
					strerr_die1sys(111, "unable to write: ");
			}
			endtime = now();
			if (substdio_put(&ssout, "CurrentTime: ", 13) == -1)
				strerr_die1sys(111, "unable to write: ");
			if (local_time) {
				datetime_tai(&dt, endtime);
				if (substdio_put(&ssout, buf, date822fmt(buf, &dt)) == -1)
					strerr_die1sys(111, "unable to write: ");
			} else {
				if (substdio_puts(&ssout, qtime_mess822(endtime)))
					strerr_die1sys(111, "unable to write: ");
			}
			rate = (double) email_count / (double) (endtime - starttime);
			time_needed = (long int) email_count/conf_rate - endtime + starttime;
			strdouble[fmt_double(strdouble, rate, 10)] = 0;
			if (substdio_put(&ssout, "CurrentRate: ", 13) == -1 ||
					substdio_puts(&ssout, strdouble) == -1 ||
					substdio_put(&ssout, "\n", 1) == -1)
				strerr_die1sys(111, "unable to write: ");
			strnum1[fmt_int(strnum1, time_needed)] = 0;
			if (rate > conf_rate) {
				if (substdio_put(&ssout, "High; need ", 11) == -1 ||
						substdio_puts(&ssout, strnum1) == -1 ||
						substdio_put(&ssout, " secs", 5) == -1)
					strerr_die1sys(111, "unable to write: ");
			} else {
				if (substdio_put(&ssout, "OK since ", 9) == -1 ||
						substdio_puts(&ssout, (0 - time_needed) > 86400 ? no_of_days(0 - time_needed) : strnum1 + 1) == -1)
					strerr_die1sys(111, "unable to write: ");
				if ((0 - time_needed) <= 86400 && substdio_put(&ssout, " secs", 5) == -1)
					strerr_die1sys(111, "unable to write: ");
			}
			if (substdio_put(&ssout, "\n", 1) == -1)
				strerr_die1sys(111, "unable to write: ");
			break;
		}
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die1sys(111, "unable to write: ");
}

void
do_list(int display)
{
	DIR            *dir;
	struct dirent  *dp;

	if (!(dir = opendir(".")))
		strerr_die1sys(111, "opendir: ");
	for (;;) {
		if(!(dp = readdir(dir)))
			break;
		if (!str_diff(dp->d_name, ".") || !str_diff(dp->d_name, ".."))
			continue;
		if (substdio_put(&ssout, "domain     : ", 13) == -1 ||
				substdio_puts(&ssout, dp->d_name) == -1 ||
				substdio_put(&ssout, "\n", 1) == -1)
			strerr_die1sys(111, "unable to write: ");
		if (display)
			do_display(dp->d_name);
		if (substdio_put(&ssout, "--------------------------------------------\n", 45) == -1)
			strerr_die1sys(111, "unable to write: ");
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die1sys(111, "unable to write: ");
}

int
check_domain(char *domain)
{
	regex_t         re;
	int             r;
	char            buf[128];
	char           *reg_exp = "^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*(\\.[a-zA-Z]+)$";

	if ((r = regcomp(&re, reg_exp, REG_EXTENDED|REG_NEWLINE))) {
		regerror(r, &re, buf, sizeof(buf));
		regfree(&re);
		strerr_die3x(111, FATAL, "regcomp: ", buf);
	}
	if ((r = regexec(&re, domain, 0, NULL, 0)) == -1) {
		regerror(r, &re, buf, sizeof(buf));
		regfree(&re);
		strerr_die3x(111, FATAL, "regexec: ", buf);
	}
	regfree(&re);
	return (r == REG_NOMATCH ? 0 : 1);
}

void
update_mode(char *domain, char *rate_expr, int reset_mode, int consolidate, int incr)
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
			if (reset_mode)
				strerr_die3sys(111, "unable to read: ", domain, ": ");
			if (!stralloc_cats(&rexpr, rate_expr))
				strerr_die1sys(111, "out of memory: ");
			if (!check_domain(domain))
				strerr_die3x(100, WARN, "invalid domain: ", domain);
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
	if ((wfd = open(domain, O_WRONLY|O_CREAT, 0640)) == -1)
		strerr_die4sys(111, FATAL, "unable to write: ", domain, ": ");
	if (lock_ex(wfd) == -1)
		cleanup(domain, "unable to lock", t);
	if (chown(domain, auto_uids, auto_gidq))
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
		if (substdio_puts(&ssfout, strdouble) == -1)
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
do_test(char *domain, int force)
{
	int             i = -1;
	char           *rate_expr = 0;
	char            strdouble1[FMT_DOUBLE], strdouble2[FMT_DOUBLE];
	unsigned long   email_count;
	datetime_sec    time_needed;
	double          rate, conf_rate;

	if (!access(domain, W_OK)) {
		if (!(i = is_rate_ok(domain, force ? "-1" : 0, &email_count, &conf_rate, &rate, &time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum1[fmt_ulong(strnum1, email_count)] = 0;
			strnum2[fmt_long(strnum2, time_needed)] = 0;
			strerr_die10x(111, WARN, domain, ": delivery rate exceeded [", strnum1, "/", strdouble1, "/", strdouble2, "] need ", strnum2);
		} else
		if (i == -1)
			strerr_die2sys(111, FATAL, "is_rate_ok: ");
	} else
	if (errno != error_noent)
		strerr_die4sys(111, FATAL, "open: ", domain, ": ");
	else
	if (!access("ratecontrol", R_OK)) {
		if (control_readfile(&line, "./ratecontrol", 0) == -1)
			strerr_die2sys(111, FATAL, "Unable to read ratecontrol: ");
		delivery = remote_delivery;
		rate_expr = getDomainToken(domain, &line);
		if (!(i = is_rate_ok(domain, rate_expr, &email_count, &conf_rate, &rate, &time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum1[fmt_ulong(strnum1, email_count)] = 0;
			strnum2[fmt_long(strnum2, time_needed)] = 0;
			strerr_die10x(111, WARN, domain, ": delivery rate exceeded [", strnum1, "/", strdouble1, "/", strdouble2, "] need ", strnum2);
		} else
		if (i == -1)
			strerr_die2sys(111, FATAL, "is_rate_ok: ");
	} else
	if (errno != error_noent)
		strerr_die2sys(111, FATAL, "open: ratecontrol: ");
	else
	if (!access(".global", W_OK)) {
		if (!(i = is_rate_ok(".global", 0, &email_count, &conf_rate, &rate, &time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum1[fmt_ulong(strnum1, email_count)] = 0;
			strnum2[fmt_long(strnum2, time_needed)] = 0;
			strerr_die10x(111, WARN, domain, ": delivery rate exceeded [", strnum1, "/", strdouble1, "/", strdouble2, "] need ", strnum2);
		} else
		if (i == -1)
			strerr_die2sys(111, FATAL, "is_rate_ok: ");
	} else
	if (errno != error_noent)
		strerr_die2sys(111, FATAL, "open: .global: ");
	if (i == 1) {
		strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
		strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
		strnum1[fmt_ulong(strnum1, email_count)] = 0;
		strnum2[fmt_long(strnum2, time_needed)] = 0;
		strerr_warn9(domain, ": email rate [", strnum1, "/", strdouble1, "/", strdouble2, "] need ", strnum2, 0);
	} else
		strerr_warn2(domain, ": email rate not configured", 0);
	return;
}

int
main(int argc, char **argv)
{
	int             i, ch, display = 0, listing = 0, incr = 0, count = 1,
					consolidate = 0, reset_mode = 0, test_mode = 0,
					force = 0;
	char           *qbase, *domain = 0, *rate_expr = 0,
				   *ptr, *ratelimit_dir = "ratelimit";

	while ((ch = getopt(argc, argv, "flutscRd:r:D:C:")) != sgoptdone) {
		switch (ch)
		{
		case 'u':
			local_time = 0;
			break;
		case 't':
			display = 0;
			test_mode = 1;
			break;
		case 'f':
			force = 1;
			break;
		case 'l':
			listing = 1;
			break;
		case 's':
			display = 1;
			break;
		case 'R':
			display = 0;
			reset_mode = 1;
			break;
		case 'C':
			scan_int(optarg, &count);
			break;
		case 'c':
			consolidate = 1;
			break;
		case 'd': /*- domain */
			ptr = optarg;
			if (ptr[i = str_rchr(ptr, '@')])
				domain = ptr + i + 1;
			else
				domain = ptr;
			break;
		case 'r': /*- delivery rate */
			display = 0;
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
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&qdir, "queue_base", 0))
		{
		case -1:
			strerr_die2sys(111, FATAL, "Unable to read control file queue_base: ");
			break;
		case 0:
			if (!stralloc_copys(&qdir, auto_qmail) ||
					!stralloc_catb(&qdir, "/queue/slowq", 12) ||
					!stralloc_0(&qdir))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 1:
			qdir.len--; /*- remove NULL put by control_readfile() */
			if (!stralloc_catb(&qdir, "/slowq", 6) ||
					!stralloc_0(&qdir))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		}
	} else {
		if (!stralloc_copys(&qdir, qbase) ||
				!stralloc_catb(&qdir, "/slowq", 6) ||
				!stralloc_0(&qdir))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (chdir(qdir.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", qdir.s, ": ");
	if (chdir(ratelimit_dir))
		strerr_die4sys(111, FATAL, "unable to switch to ", ratelimit_dir, ": ");

	if (!listing && !domain) {
		logerrf("domain not specified\n");
		logerrf(usage);
		_exit (111);
	}

	if (listing)
		do_list(display);
	else
	if (display)
		do_display(domain);
	else
	if (test_mode) {
		for (i = 0; i < count; i++)
			do_test(domain, force);
	} else {
		if (!reset_mode && !rate_expr) {
			logerrf("rate expression not specified\n");
			logerrf(usage);
			_exit(111);
		}
		update_mode(domain, rate_expr, reset_mode, consolidate, incr);
	}
	return (0);
}

void
getversion_drate_c()
{
	static char    *x = "$Id: drate.c,v 1.18 2021-08-22 00:12:29+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidgetdomainth;
	x = sccsidevalh;
	x = sccsidgetrateh;
	x++;
}
