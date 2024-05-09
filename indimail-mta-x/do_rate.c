/*
 * $Log: do_rate.c,v $
 * Revision 1.3  2022-04-13 07:48:09+05:30  Cprogrammer
 * return 2 if rate definition is missing or is invalid
 *
 * Revision 1.2  2021-06-05 12:44:50+05:30  Cprogrammer
 * return time_needed in seconds to reach configured rate
 *
 * Revision 1.1  2021-05-29 23:35:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stralloc.h>
#include <env.h>
#include <now.h>
#include <fmt.h>
#include <str.h>
#include <open.h>
#include <scan.h>
#include <substdio.h>
#include <getln.h>
#include <error.h>
#include <evaluate.h>
#include <lock.h>
#include <datetime.h>
#include "do_rate.h"

static stralloc fline = { 0 }, _rate_expr = { 0 };

int
get_rate(const char *expression, double *rate)
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
		return (-1);
	case ERROR_VARNOTFOUND:
		free_vartable(vt);
		return (-1);
	case ERROR_NOMEM:
		free_vartable(vt);
		return (-1);
	case ERROR_DIV0:
		free_vartable(vt);
		return (-1);
	case RESULT_OK:
		*rate = (double) ((result.type == T_INT) ? result.ival : result.rval);
		free_vartable(vt);
		return (0);
	}
	free_vartable(vt);
	return (0);
}

/*-
 * returns
 * 0 if delivery rate is exceeded
 * 1 if delivery rate is fine
 * 2 if no rate control definition exists
 */
int
is_rate_ok(const char *_file, const char *_rate_exp, unsigned long *e, double *c,
		double *r, datetime_sec *time_needed)
{
	int             at, wfd, rfd, match, line_no = -1, rate_int,
					reset, access_flag, force = 0;
	unsigned long   email_count = 0;
	char            stime[FMT_ULONG], etime[FMT_ULONG], ecount[FMT_ULONG];
	double          conf_rate, cur_rate = 0.0;
	char            inbuf[2048], outbuf[1024];
	const char     *ptr, *rate_exp, *file;
	struct substdio ssin, ssout;
	datetime_sec    starttime, endtime;
	struct stat     statbuf;

	if (_file[at = str_rchr(_file, '@')])
		file = _file + at + 1;
	else
		file = _file;
	if (e)
		*e = 0;
	starttime = endtime = now();
	if (!(ptr = env_get("RATELIMIT_INTERVAL")))
		rate_int = 86400;
	else
		scan_int(ptr, &rate_int);
	rate_exp = _rate_exp;
	if (rate_exp && !str_diffn(rate_exp, "-1", 3)) {
		force = 1;
		rate_exp = NULL;
	}
	reset = stat(file, &statbuf) ? 1 : (starttime - statbuf.st_mtime > rate_int);
	stime[fmt_ulong(stime, starttime)] = 0;
	etime[fmt_ulong(etime, endtime)] = 0;
	access_flag = access(file, F_OK);
	if (access_flag && errno != error_noent)
		return -1;
	if (rate_exp) {
		if ((wfd = (access_flag ? open_excl : open_write) (file)) == -1)
			return -1;
		else
		if (lock_ex(wfd) == -1)
			return -1;
		if (!stralloc_copys(&_rate_expr, rate_exp) ||
				!stralloc_append(&_rate_expr, DELIMITER))
			return -1;
		if (get_rate(rate_exp, &conf_rate))
			return -1;
		if (c)
			*c = conf_rate;
	} else {
		if ((wfd = open_write(file)) == -1)
			return -1;
		else
		if (lock_ex(wfd) == -1)
			return -1;
	}
	if (!access_flag) { /*- only if rate definition exists */
		if ((rfd = open_read(file)) == -1)
			return -1;
		substdio_fdbuf(&ssin, read, rfd, inbuf, sizeof(inbuf));
		for (line_no = 1;;line_no++) { /*- Line Processing */
			if (getln(&ssin, &fline, &match, DELIMITER[0]) == -1)
				return -1;
			if (!match && fline.len == 0)
				break;
			fline.len--;
			if (DELIMITER[0]) {
				if (!stralloc_0(&fline))
					return -1;
				fline.len--;
			}
			switch (line_no)
			{
			case 1: /*- rate expression */
				if (!stralloc_copy(&_rate_expr, &fline))
					return -1;
				if (get_rate(fline.s, &conf_rate))
					return -1;
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
				if (time_needed)
					*time_needed = (long int) email_count/conf_rate - endtime + starttime;
				break;
			}
		}
		close(rfd);
	}
	/*-
	 * line_no   <  1         - no point in messing. We either have invalid data
	 *                          or missing rate control definition
	 * conf_rate <  0         - update the email count, timestamps
	 * conf_rate >= 0 and
	 * cur_rate  > conf_rate - defer emails
	 */
	if (line_no < 1 || (force == 0 && conf_rate >= 0 && cur_rate > conf_rate)) {
		close(wfd);
		return (line_no < 1 ? 2 : 0);
	}
	/*- update rate control definition */
	ecount[fmt_ulong(ecount, ++email_count)] = 0;
	if (e)
		*e = email_count;
	substdio_fdbuf(&ssout, write, wfd, outbuf, sizeof(outbuf));
	if (substdio_bput(&ssout, _rate_expr.s, _rate_expr.len) == -1 ||
			substdio_bput(&ssout, DELIMITER, 1) == -1)
		return -1;

	if (substdio_bputs(&ssout, ecount) == -1 ||
			substdio_bput(&ssout, DELIMITER, 1) == -1)
		return -1;

	if (substdio_bputs(&ssout, stime) == -1 ||
			substdio_bput(&ssout, DELIMITER, 1) == -1)
		return -1;

	if (substdio_bputs(&ssout, etime) == -1 ||
			substdio_bput(&ssout, DELIMITER, 1) == -1)
		return -1;

	if (substdio_flush(&ssout) == -1)
		return -1;
	close(wfd);
	return (1);
}

void
getversion_do_rate_c()
{
	const char     *x = "$Id: do_rate.c,v 1.3 2022-04-13 07:48:09+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidevalh;
	x = sccsidgetrateh;
	x++;
}
