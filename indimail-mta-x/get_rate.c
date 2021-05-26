/*
 * $Log: get_rate.c,v $
 * Revision 1.2  2021-05-26 07:42:11+05:30  Cprogrammer
 * made DELIMITER configurable
 * return email count, configure rate and current rate in is_rate_ok()
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stralloc.h>
#include <env.h>
#include <now.h>
#include <fmt.h>
#include <open.h>
#include <scan.h>
#include <substdio.h>
#include <getln.h>
#include <error.h>
#include <evaluate.h>
#include <lock.h>
#include "report.h"
#include "get_rate.h"

static stralloc fline = { 0 }, _rate_expr = { 0 };

int
get_rate(char *expression, double *rate)
{
	struct val      result;
	struct vartable *vt;

	/*-
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		return (-1);
	switch (evaluate(expression, &result, vt))
	{
	case ERROR_SYNTAX:
		free_vartable(vt);
		report(111, "spawn: ", "get_rate: syntax error: [", expression, "]. (#4.3.0)", 0, 0);
		return (-1);
	case ERROR_VARNOTFOUND:
		free_vartable(vt);
		report(111, "spawn: ", "get_rate: variable not found: ", expression, ". (#4.3.0)", 0, 0);
		return (-1);
	case ERROR_NOMEM:
		free_vartable(vt);
		report(111, "spawn: ", "out of memory", ". (#4.3.0)", 0, 0, 0);
		return (-1);
	case ERROR_DIV0:
		free_vartable(vt);
		report(111, "spawn: ", "get_rate: division by zero: ", expression, ". (#4.3.0)", 0, 0);
		return (-1);
	case RESULT_OK:
		*rate = (double) ((result.type == T_INT) ? result.ival : result.rval);
		free_vartable(vt);
		return (0);
	}
	free_vartable(vt);
	return (0);
}

int
is_rate_ok(char *file, char *rate_exp, unsigned long *e, double *c, double *r)
{
	int             wfd, rfd, match, line_no = -1, rate_int, access_flag;
	unsigned long   email_count = 0;
	char            reset, stime[FMT_ULONG], etime[FMT_ULONG], ecount[FMT_ULONG];
	double          conf_rate, cur_rate = 0.0;
	char            inbuf[2048], outbuf[1024];
	char           *ptr;
	struct substdio ssin, ssout;
	datetime_sec    starttime, endtime;
	struct stat     statbuf;

	if (e)
		*e = 0;
	starttime = endtime = now();
	if (!(ptr = env_get("RATELIMIT_INTERVAL")))
		rate_int = 86400;
	else
		scan_int(ptr, &rate_int);
	reset = stat(file, &statbuf) ? 1 : (starttime - statbuf.st_mtime > rate_int);
	stime[fmt_ulong(stime, starttime)] = 0;
	etime[fmt_ulong(etime, endtime)] = 0;
	ecount[0] = '1'; /*- we are delivering the first email since rate control has been imposed */
	ecount[1] = 0;
	access_flag = access(file, F_OK);
	if (access_flag && errno != error_noent)
		report(111, "spawn: ", "open: ", file, ": ", error_str(errno), ". (#4.3.0)");
	if (rate_exp) {
		if ((wfd = (access_flag ? open_excl : open_write) (file)) == -1)
			report(111, "spawn: ", "unable to write_excl: ", file, ": ", error_str(errno), ". (#4.3.0)");
		if (lock_ex(wfd) == -1)
			report(111, "spawn: ", "unable to lock: ", file, ": ", error_str(errno), ". (#4.3.0)");
		if (!stralloc_copys(&_rate_expr, rate_exp))
			report(111, "spawn: ", "out of memory", ". (#4.3.0)", 0, 0, 0);
		else
		if (!stralloc_append(&_rate_expr, DELIMITER))
			report(111, "spawn: ", "out of memory", ". (#4.3.0)", 0, 0, 0);
		get_rate(rate_exp, &conf_rate);
		if (c)
			*c = conf_rate;
	} else {
		if ((wfd = open_write(file)) == -1)
			report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
		if (lock_ex(wfd) == -1)
			report(111, "spawn: ", "unable to lock: ", file, ": ", error_str(errno), ". (#4.3.0)");
	}
	if (!access_flag) { /*- only if rate definition exists */
		if ((rfd = open_read(file)) == -1)
			report(111, "spawn: ", "unable to read: ", file, ": ", error_str(errno), ". (#4.3.0)");
		substdio_fdbuf(&ssin, read, rfd, inbuf, sizeof(inbuf));
		for (line_no = 1;;line_no++) { /*- Line Processing */
			if (getln(&ssin, &fline, &match, DELIMITER[0]) == -1)
				report(111, "spawn: ", "unable to read: ", file, ": ", error_str(errno), ". (#4.3.0)");
			if (!match && fline.len == 0)
				break;
			fline.len--;
			if (DELIMITER[0]) {
				if (!stralloc_0(&fline))
					report(111, "spawn: ", "out of memory", ". (#4.3.0)", 0, 0, 0);
				fline.len--;
			}
			switch (line_no)
			{
			case 1: /*- rate expression */
				if (!stralloc_copy(&_rate_expr, &fline))
					report(111, "spawn: ", "out of memory", ". (#4.3.0)", 0, 0, 0);
				get_rate(fline.s, &conf_rate);
				if (c)
					*c = conf_rate;
				break;
			case 2: /*- email count */
				if (reset)
					continue;
				scan_ulong(fline.s, (unsigned long *) &email_count);
				if (e)
					*e = email_count;
				break;
			case 3: /*- start time */
				if (reset)
					continue;
				scan_ulong(fline.s, (unsigned long *) &starttime);
				stime[fmt_ulong(stime, starttime)] = 0;
				break;
			case 4: /*- current rate */
				if (reset)
					continue;
				/* do not divide by zero */
				cur_rate = (endtime == starttime) ? 0 : ((float) email_count / (float) (endtime - starttime));
				if (r)
					*r = cur_rate;
				break;
			}
		}
		close(rfd);
	}
	/*-
	 * line_no   < 1        - no point in messing with invalid data
	 * conf_rate < 0        - update the email count, timestamps
	 * conf_rate = 0        - defer emails
	 * cur_rate > conf_rate - defer emails
	 */
	if (line_no < 1 || (conf_rate >= 0 && cur_rate > conf_rate)) {
		close(wfd);
		return (line_no < 1 ? 1 : 0);
	}
	ecount[fmt_ulong(ecount, ++email_count)] = 0;
	if (e)
		*e = email_count;
	substdio_fdbuf(&ssout, write, wfd, outbuf, sizeof(outbuf));
	if (substdio_bput(&ssout, _rate_expr.s, _rate_expr.len) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
	if (substdio_bput(&ssout, DELIMITER, 1) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");

	if (substdio_bputs(&ssout, ecount) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
	if (substdio_bput(&ssout, DELIMITER, 1) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");

	if (substdio_bputs(&ssout, stime) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
	if (substdio_bput(&ssout, DELIMITER, 1) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");

	if (substdio_bputs(&ssout, etime) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
	if (substdio_bput(&ssout, DELIMITER, 1) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");

	if (substdio_flush(&ssout) == -1)
		report(111, "spawn: ", "unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)");
	close(wfd);
	return (1);
}

void
getversion_get_rate_c()
{
	static char    *x = "$Id: get_rate.c,v 1.2 2021-05-26 07:42:11+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidevalh;
	x = sccsidgetrateh;
	x = sccsidreporth;
	x++;
}
