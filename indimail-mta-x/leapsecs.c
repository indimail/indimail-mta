/*
 * $Log: leapsecs.c,v $
 * Revision 1.3  2016-01-28 15:06:55+05:30  Cprogrammer
 * removed dependency on home directory
 *
 * Revision 1.2  2016-01-28 09:47:55+05:30  Cprogrammer
 * removed stdio to use substdio
 *
 * Revision 1.1  2016-01-28 01:42:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "getln.h"
#include "strerr.h"
#include "tai.h"
#include "leapsecs.h"
#include "caldate.h"

#define FATAL "leapsecs: fatal: "

/*
 * XXX: breaks tai encapsulation
 */

/*
 * XXX: output here has to be binary; DOS redirection uses ASCII
 */

stralloc        line = { 0 };
int             match;

int
main()
{
	struct caldate  cd;
	struct tai      t;
	char            x[TAI_PACK];
	long            leaps = 0;

	for (;;)
	{
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match)
			break;
		if (line.s[0] == '+')
			if (caldate_scan(line.s + 1, &cd)) {
				t.x = (caldate_mjd(&cd) + 1) * 86400ULL + 4611686014920671114ULL + leaps++;
				tai_pack(x, &t);
				if (substdio_put(subfdoutsmall, x, TAI_PACK))
					strerr_die2sys(111, FATAL, "unable to write: ");
			}
	}
	if (substdio_flush(subfdoutsmall))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(0);
}

void
getversion_leapsecs_c()
{
	const char     *x = "$Id: leapsecs.c,v 1.3 2016-01-28 15:06:55+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
