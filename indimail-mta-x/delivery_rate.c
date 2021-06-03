/*
 * $Log: delivery_rate.c,v $
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
#include "qsutil.h"
#include "variables.h"
#include "do_rate.h"
#include "getDomainToken.h"
#include "control.h"
#include "delivery_rate.h"

static stralloc ratedefs = { 0 };
static stralloc ratelimit_file = { 0 };
extern char    *queuedesc;

int
delivery_rate(char *_domain)
{
	char           *rate_dir, *rate_exp, *domain;
	int             i, s, at;
	char            strdouble1[FMT_DOUBLE], strdouble2[FMT_DOUBLE],
	                strnum[FMT_ULONG];
	unsigned long   email_count;
	double          rate, conf_rate;

	if ((rate_dir = env_get("RATELIMIT_DIR"))) {
		if (!*rate_dir)
			return 1;
		while (!stralloc_copys(&ratelimit_file, rate_dir))
			nomem();
		s = ratelimit_file.len;
	} else {
		while (!stralloc_copys(&ratelimit_file, queuedir) ||
				!stralloc_catb(&ratelimit_file, "/ratelimit", 10))
			nomem();
		s = ratelimit_file.len;
	}
	if (_domain[at = str_rchr(_domain, '@')])
		domain = _domain + at + 1;
	else
		domain = _domain;
	while (!stralloc_append(&ratelimit_file, "/") ||
			!stralloc_cats(&ratelimit_file, domain) ||
			!stralloc_0(&ratelimit_file))
		nomem();
	if (!access(ratelimit_file.s, W_OK)) {
		if (!(i = is_rate_ok(ratelimit_file.s, 0, &email_count, &conf_rate, &rate))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum[fmt_ulong(strnum, email_count)] = 0;
			log11("warning: ", queuedesc, ": delivery rate exceeded [", strnum,
					"/", strdouble1, "/", strdouble2, "] for ", domain, "; will try again later\n");
			return 0;
		} else {
			if (i == 1) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				strnum[fmt_ulong(strnum, email_count)] = 0;
				log11(queuedesc, ": ", domain, ": delivery rate [", strdouble1,
						"] conf rate [", strdouble2, "] email count [", strnum, "]", "\n");
			}
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;

	/*- ratecontrol */
	ratelimit_file.len = s;
	while (!stralloc_catb(&ratelimit_file, "/ratecontrol", 12) ||
			!stralloc_0(&ratelimit_file))
		nomem();
	if (!access(ratelimit_file.s, R_OK)) {
		if (control_readfile(&ratedefs, ratelimit_file.s, 0) == -1)
			return -1;
		rate_exp = getDomainToken(domain, &ratedefs);
		if (!(i = is_rate_ok(ratelimit_file.s, rate_exp, &email_count, &conf_rate, &rate))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum[fmt_ulong(strnum, email_count)] = 0;
			log11("warning: ", queuedesc, ": delivery rate exceeded [", strnum,
					"/", strdouble1, "/", strdouble2, "] for ", domain, "; will try again later\n");
			return 0;
		} else {
			if (i == 1) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				strnum[fmt_ulong(strnum, email_count)] = 0;
				log11(queuedesc, ": ", domain, ": delivery rate [", strdouble1,
						"] conf rate [", strdouble2, "] email count [", strnum, "]", "\n");
			}
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;

	/*- .global */
	ratelimit_file.len = s;
	while (!stralloc_catb(&ratelimit_file, "/.global", 8) ||
			!stralloc_0(&ratelimit_file))
		nomem();
	if (!access(ratelimit_file.s, W_OK)) {
		if (!(i = is_rate_ok(ratelimit_file.s, 0, &email_count, &conf_rate, &rate))) {
			strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
			strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
			strnum[fmt_ulong(strnum, email_count)] = 0;
			log11("warning: ", queuedesc, ": delivery rate exceeded [", strnum,
					"/", strdouble1, "/", strdouble2, "] for ", domain, "; will try again later\n");
			return 0;
		} else {
			if (i == 1) {
				strdouble1[fmt_double(strdouble1, rate, 10)] = 0;
				strdouble2[fmt_double(strdouble2, conf_rate, 10)] = 0;
				strnum[fmt_ulong(strnum, email_count)] = 0;
				log11(queuedesc, ": ", domain, ": delivery rate [", strdouble1,
						"] conf rate [", strdouble2, "] email count [", strnum, "]", "\n");
			}
			return i; /*- either 1 or -1 */
		}
	} else
	if (errno != error_noent)
		return -1;
	return 1;
}

void
getversion_delivery_rate_c()
{
	static char    *x = "$Id: delivery_rate.c,v 1.2 2021-06-03 13:22:44+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidgetdomainth;
	x = sccsidgetrateh;
	x = sccsiddelivery_rateh;
	if (x)
		x++;
}
