/*
 * $Log: drate.c,v $
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
#include "substdio.h"
#include "error.h"
#include "fmt.h"
#include "sgetopt.h"
#include "evaluate.h"
#include "open.h"
#include "variables.h"
#include "strerr.h"
#include "env.h"
#include "lock.h"
#include "getln.h"
#include "now.h"
#include "scan.h"
#include "str.h"
#include "myctime.h"
#include "auto_uids.h"
#include "auto_control.h"

#define FATAL     "drate: fatal: "

char            strnum[FMT_ULONG];
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
int             qmailr_uid;
int             qmail_gid;
char           *usage = "usage: drate [-scR] -d domain -r deliveryRate [-D ratelimit_dir]\n";

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
check_rate(char *expression, double *rate)
{
	struct val      result;
	struct vartable *vt;

	/*
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		my_error("evaluate lib failed", 111);
	switch (evaluate(expression, &result, vt))
	{
	case ERROR_SYNTAX:
		free_vartable(vt);
		my_error("syntax error", 111);
		break;
	case ERROR_VARNOTFOUND:
		free_vartable(vt);
		my_error("variable not found", 111);
		break;
	case ERROR_NOMEM:
		free_vartable(vt);
		my_error("out of memory", 111);
		break;
	case ERROR_DIV0:
		free_vartable(vt);
		my_error("division by zero", 111);
		break;
	case RESULT_OK:
		*rate = (double) ((result.type == T_INT) ? result.ival : result.rval);
		free_vartable(vt);
		return;
	default:
		free_vartable(vt);
		my_error("unknown error evaluating expression", 111);
		break;
	}
	/*- does not return */
	_exit (1);
}

int
main(int argc, char **argv)
{
	int             ch, rfd, wfd, match, line_no, display = 0, incr = 0,
					consolidate = 0, reset = 0;
	unsigned long   email_count = 0;
	double          rate;
	char           *domain = 0, *rate_expr = 0, *directory = "ratelimit";
	char            stime[FMT_ULONG], etime[FMT_ULONG], ecount[FMT_ULONG];
	char            strdouble[FMT_DOUBLE];
	char            inbuf[2048], outbuf[1024];
	struct substdio ssfin, ssfout;
	datetime_sec    starttime = 0, endtime = 0;
	stralloc        line = { 0 }, rexpr = { 0 };

	while ((ch = getopt(argc, argv, "scRd:r:D:")) != sgoptdone) {
		switch (ch)
		{
		case 's':
			display = 1;
			break;
		case 'R':
			reset = 1;
			break;
		case 'd': /*- domain */
			domain = optarg;
			break;
		case 'c':
			consolidate = 1;
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
		case 'D': /*- directory */
			directory = optarg;
			break;
		default:
			logerrf(usage);
			_exit (100);
			break;
		}
	}
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!domain) {
		logerrf("domain not specified\n");
		logerrf(usage);
		_exit (111);
	}
	if (chdir(auto_control) || chdir(directory))
		strerr_die4sys(111, FATAL, "unable to switch to ", directory, ": ");
	if (display) {
		if ((rfd = open_read(domain)) == -1)
			strerr_die3sys(111, "unable to read: ", domain, ": ");
		substdio_fdbuf(&ssfin, read, rfd, inbuf, sizeof(inbuf));
		for (line_no = 1;;line_no++) { /*- Line Processing */
			if (getln(&ssfin, &line, &match, '\n') == -1)
				strerr_die3sys(111, "unable to read: ", domain, ": ");
			if (!match && line.len == 0)
				break;
			switch (line_no)
			{
			case 1:
				line.len--;
				if (!stralloc_0(&line))
					strerr_die1sys(111, "out of mem: ");
				if (substdio_put(&ssout, "Conf   Rate: ", 13))
					strerr_die1sys(111, "unable to write: ");
				if (substdio_put(&ssout, line.s, line.len) == -1)
					strerr_die1sys(111, "unable to write: ");
				check_rate(line.s, &rate);
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
	} else {
		if (!rate_expr) {
			logerrf("rate expression not specified\n");
			_exit(111);
		}
		check_rate(incr ? rate_expr + 1 : rate_expr, &rate);
		if ((rfd = open_read(domain)) == -1 && errno != error_noent)
			strerr_die3sys(111, "unable to read: ", domain, ": ");
		else {
			if (rfd == -1 || reset) {
				if (!stralloc_cats(&rexpr, rate_expr))
					strerr_die1sys(111, "out of mem: ");
				consolidate = 0;
				goto new;
			}
			substdio_fdbuf(&ssfin, read, rfd, inbuf, sizeof(inbuf));
			for (line_no = 1;;line_no++) { /*- Line Processing */
				if (getln(&ssfin, &line, &match, '\n') == -1)
					strerr_die3sys(111, "unable to read: ", domain, ": ");
				if (!match && line.len == 0)
					break;
				switch (line_no)
				{
				case 1:
					/*- we are going to overwrite this */
					line.len--;
					if (!stralloc_0(&line))
						strerr_die1sys(111, "out of mem: ");
					if (incr) {
						line.len--;
						if (!stralloc_cat(&rexpr, &line))
							strerr_die1sys(111, "out of mem: ");
						if (!stralloc_cats(&rexpr, rate_expr))
							strerr_die1sys(111, "out of mem: ");
					} else {
						if (!stralloc_cats(&rexpr, rate_expr))
							strerr_die1sys(111, "out of mem: ");
					}
					continue;
					break;
				case 2:
					line.len--;
					if (!stralloc_0(&line))
						strerr_die1sys(111, "out of mem: ");
					scan_ulong(line.s, (unsigned long *) &email_count);
					ecount[fmt_ulong(ecount, email_count)] = 0;
					break;
				case 3:
					line.len--;
					if (!stralloc_0(&line))
						strerr_die1sys(111, "out of mem: ");
					scan_ulong(line.s, (unsigned long *) &starttime);
					stime[fmt_ulong(stime, starttime)] = 0;
					break;
				case 4:
					line.len--;
					if (!stralloc_0(&line))
						strerr_die1sys(111, "out of mem: ");
					scan_ulong(line.s, (unsigned long *) &endtime);
					stime[fmt_ulong(etime, endtime)] = 0;
					break;
				}
			}
		}
new:
		qmailr_uid = auto_uidr;
		qmail_gid = auto_gidq;
		if ((wfd = open(domain, O_WRONLY|O_CREAT, 0644)) == -1)
			strerr_die4sys(111, FATAL, "unable to write: ", domain, ": ");
		if (lock_ex(wfd) == -1)
			strerr_die3sys(111, "unable to lock: ", domain, ": ");
		if (chown(domain, auto_uidr, auto_gidq))
			strerr_die4sys(111, FATAL, "unable to chown: ", domain, ": ");
		if (ftruncate(wfd, 0) == -1)
			strerr_die3sys(111, "unable to truncate: ", domain, ": ");
		substdio_fdbuf(&ssfout, write, wfd, outbuf, sizeof(outbuf));
		if (consolidate) {
			if (!stralloc_0(&rexpr))
				strerr_die1sys(111, "out of mem: ");
			rexpr.len--;
			check_rate(rexpr.s, &rate);
			strdouble[fmt_double(strdouble, rate, 10)] = 0;
			if (substdio_puts(&ssfout, strdouble))
				strerr_die1sys(111, "unable to write: ");
		} else
		if (substdio_bput(&ssfout, rexpr.s, rexpr.len) == -1)
			strerr_die3sys(111, "unable to write: ", domain, ": ");
		if (substdio_bput(&ssfout, "\n", 1) == -1)
			strerr_die3sys(111, "unable to write: ", domain, ": ");
		if (!email_count || !starttime || !endtime) {
			if (substdio_flush(&ssfout) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
		} else {
			if (substdio_bputs(&ssfout, ecount) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
			if (substdio_bput(&ssfout, "\n", 1) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
			if (substdio_bputs(&ssfout, stime) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
			if (substdio_bput(&ssfout, "\n", 1) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
			if (substdio_bputs(&ssfout, etime) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
			if (substdio_bput(&ssfout, "\n", 1) == -1)
				strerr_die3sys(111, "unable to write: ", domain, ": ");
		}
		if (substdio_flush(&ssfout) == -1)
			strerr_die3sys(111, "unable to write: ", domain, ": ");
		close(wfd);
	}
	return (0);
}

void
getversion_drate_c()
{
	static char    *x = "$Id: drate.c,v 1.7 2021-05-23 07:07:23+05:30 Cprogrammer Exp mbhangui $";

	x++;
	if (x)
		x = sccsidevalh;
}
