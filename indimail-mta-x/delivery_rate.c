/*
 * $Log: delivery_rate.c,v $
 * Revision 1.8  2022-04-13 07:59:46+05:30  Cprogrammer
 * set delivery_rate only if rate control definition exists
 *
 * Revision 1.7  2022-01-30 08:30:55+05:30  Cprogrammer
 * added additional argument in nomem()
 *
 * Revision 1.6  2021-08-13 18:25:53+05:30  Cprogrammer
 * turn off ratelimit if RATELIMIT_DIR is set but empty
 *
 * Revision 1.5  2021-06-05 12:43:43+05:30  Cprogrammer
 * set do_ratelimit
 *
 * Revision 1.4  2021-06-04 09:24:01+05:30  Cprogrammer
 * added time_needed argument to delivery_rate()
 *
 * Revision 1.3  2021-06-03 18:37:31+05:30  Cprogrammer
 * display email count if delivery had succeeded
 *
 * Revision 1.2  2021-06-03 13:22:44+05:30  Cprogrammer
 * display email/curr_rate/conf_rate in logs
 *
 * Revision 1.1  2021-06-01 01:48:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fmt.h>
#include <str.h>
#include <error.h>
#include <env.h>
#include <stralloc.h>
#include <datetime.h>
#include "qsutil.h"
#include "variables.h"
#include "do_rate.h"
#include "getDomainToken.h"
#include "control.h"
#include "delivery_rate.h"

static stralloc ratedefs = { 0 };
static stralloc ratelimit_file = { 0 };
extern char    *queuedesc;

/*-
 * returns
 *  0 - delivery rate should be throttled
 *  1 - delivery rate is fine
 * -1 - failed to get delivery rate
 * Additionally
 * set do_ratelimit if rate definition exists
 */
int
delivery_rate(char *_domain, unsigned long id, datetime_sec *time_needed,
		int *do_ratelimit, char *argv0)
{
	char           *rate_dir, *rate_exp, *domain;
	int             i, s, at;
	char            strdouble1[FMT_DOUBLE], strdouble2[FMT_DOUBLE],
	                email[FMT_ULONG], strnum1[FMT_ULONG], strnum2[FMT_LONG];
	unsigned long   email_count;
	double          rate, conf_rate;

	if ((rate_dir = env_get("RATELIMIT_DIR"))) {
		if (!*rate_dir) {
			if (do_ratelimit)
				*do_ratelimit = 0;
			return 1;
		}
		while (!stralloc_copys(&ratelimit_file, rate_dir))
			nomem(argv0);
		s = ratelimit_file.len;
	} else {
		while (!stralloc_copys(&ratelimit_file, queuedir) ||
				!stralloc_catb(&ratelimit_file, "/ratelimit", 10))
			nomem(argv0);
		s = ratelimit_file.len;
	}
	if (_domain[at = str_rchr(_domain, '@')])
		domain = _domain + at + 1;
	else
		domain = _domain;
	while (!stralloc_append(&ratelimit_file, "/") ||
			!stralloc_cats(&ratelimit_file, domain) ||
			!stralloc_0(&ratelimit_file))
		nomem(argv0);
	if (!access(ratelimit_file.s, W_OK)) {
		if (!(i = is_rate_ok(ratelimit_file.s, 0, &email_count, &conf_rate, &rate, time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			email[fmt_ulong(email, email_count + 1)] = 0;
			strnum1[fmt_ulong(strnum1, id)] = 0;
			strnum2[fmt_long(strnum2, *time_needed)] = 0;
			log15("warning: ", queuedesc, " ", domain, " msg ", strnum1,
					": rate exceeded [", email, "/", strdouble1,
					"/", strdouble2, "] need ", strnum2,
					" secs; will try again later\n");
			if (do_ratelimit)
				*do_ratelimit = 1;
			return 0;
		} else {
			if (i == 1 || i == 2) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				email[fmt_ulong(email, email_count)] = 0;
				strnum1[fmt_ulong(strnum1, id)] = 0;
				strnum2[fmt_long(strnum2, *time_needed)] = 0;
				log15("info: ", queuedesc, " ", domain, " msg ", strnum1,
						": rate [", email, "/", strdouble1, "/", strdouble2,
						"] ok since ", strnum2 + 1, " secs\n");
			}
			if (do_ratelimit && i == 1)
				*do_ratelimit = 1;
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;

	/*- ratecontrol */
	ratelimit_file.len = s;
	while (!stralloc_catb(&ratelimit_file, "/ratecontrol", 12) ||
			!stralloc_0(&ratelimit_file))
		nomem(argv0);
	if (!access(ratelimit_file.s, R_OK)) {
		if (control_readfile(&ratedefs, ratelimit_file.s, 0) == -1)
			return -1;
		rate_exp = getDomainToken(domain, &ratedefs);
		if (!(i = is_rate_ok(ratelimit_file.s, rate_exp, &email_count, &conf_rate, &rate, time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			email[fmt_ulong(email, email_count + 1)] = 0;
			strnum1[fmt_ulong(strnum1, id)] = 0;
			strnum2[fmt_long(strnum2, *time_needed)] = 0;
			log15("warning: ", queuedesc, " ", domain, " msg ", strnum1,
					": rate exceeded [", email, "/", strdouble1,
					"/", strdouble2, "] need ", strnum2,
					" secs; will try again later\n");
			if (do_ratelimit)
				*do_ratelimit = 1;
			return 0;
		} else {
			if (i == 1 || i == 2) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				email[fmt_ulong(email, email_count)] = 0;
				strnum1[fmt_ulong(strnum1, id)] = 0;
				strnum2[fmt_long(strnum2, *time_needed)] = 0;
				log15("info: ", queuedesc, " ", domain, " msg ", strnum1,
						": rate [", email, "/", strdouble1, "/", strdouble2,
						"] ok since ", strnum2 + 1, " secs\n");
			}
			if (do_ratelimit && i == 1)
				*do_ratelimit = 1;
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;

	/*- .global */
	ratelimit_file.len = s;
	while (!stralloc_catb(&ratelimit_file, "/.global", 8) ||
			!stralloc_0(&ratelimit_file))
		nomem(argv0);
	if (!access(ratelimit_file.s, W_OK)) {
		if (!(i = is_rate_ok(ratelimit_file.s, 0, &email_count, &conf_rate, &rate, time_needed))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			email[fmt_ulong(email, email_count + 1)] = 0;
			strnum1[fmt_ulong(strnum1, id)] = 0;
			strnum2[fmt_long(strnum2, *time_needed)] = 0;
			log15("warning: ", queuedesc, " ", domain, " msg ", strnum1,
					": rate exceeded [", email, "/", strdouble1,
					"/", strdouble2, "] need ", strnum2,
					" secs; will try again later\n");
			if (do_ratelimit)
				*do_ratelimit = 1;
			return 0;
		} else {
			if (i == 1 || i == 2) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				email[fmt_ulong(email, email_count)] = 0;
				strnum1[fmt_ulong(strnum1, id)] = 0;
				strnum2[fmt_long(strnum2, *time_needed)] = 0;
				log15("info: ", queuedesc, " ", domain, " msg ", strnum1,
						": rate [", email, "/", strdouble1, "/", strdouble2,
						"] ok since ", strnum2 + 1, " secs\n");
			}
			if (do_ratelimit && i == 1)
				*do_ratelimit = 1;
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;
	if (do_ratelimit)
		*do_ratelimit = 0;
	return 1;
}

void
getversion_delivery_rate_c()
{
	static char    *x = "$Id: delivery_rate.c,v 1.8 2022-04-13 07:59:46+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidgetdomainth;
	x = sccsidgetrateh;
	x = sccsiddelivery_rateh;
	if (x)
		x++;
}
