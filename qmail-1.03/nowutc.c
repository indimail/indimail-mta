/*
 * $Log: nowutc.c,v $
 * Revision 1.1  2016-01-28 23:42:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <unistd.h>
#include "leapsecs.h"
#include "tai.h"
#include "taia.h"
#include "caltime.h"

struct taia     now;
struct tai      sec;
struct caltime  ct;

char            x[TAIA_FMTFRAC];

int
main()
{
	if (leapsecs_init() == -1) {
		fprintf(stderr, "utcnow: fatal: unable to init leapsecs\n");
		_exit(111);
	}

	taia_now(&now);
	x[taia_fmtfrac(x, &now)] = 0;

	taia_tai(&now, &sec);
	caltime_utc(&ct, &sec, (int *) 0, (int *) 0);

	printf("%d-%02d-%02d %02d:%02d:%02d.%s\n", ct.date.year, ct.date.month, ct.date.day, ct.hour, ct.minute, ct.second, x);

	return (0);
}

void
getversion_nowutc_c()
{
	static char    *x = "$Id: nowutc.c,v 1.1 2016-01-28 23:42:47+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
