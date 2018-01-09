/*
 * $Log: nowutc.c,v $
 * Revision 1.3  2016-05-21 14:48:14+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.2  2016-01-28 23:51:48+05:30  Cprogrammer
 * chdir /var/indimail for leapsecs.dat
 *
 * Revision 1.1  2016-01-28 23:42:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "leapsecs.h"
#include "tai.h"
#include "taia.h"
#include "caltime.h"
#include "auto_sysconfdir.h"

struct taia     now;
struct tai      sec;
struct caltime  ct;

char            x[TAIA_FMTFRAC];

int
main()
{
	if (chdir(auto_sysconfdir) == -1) {
		fprintf(stderr, "chdir: %s: %s\n", auto_sysconfdir, strerror(errno));
		_exit (111);
	}
	if (leapsecs_init() == -1) {
		fprintf(stderr, "utcnow: fatal: unable to init leapsecs\n");
		_exit (111);
	}

	taia_now(&now);
	x[taia_fmtfrac(x, &now)] = 0;

	taia_tai(&now, &sec);
	caltime_utc(&ct, &sec, (int *) 0, (int *) 0);

	printf("%ld-%02d-%02d %02d:%02d:%02d.%s\n", ct.date.year, ct.date.month, ct.date.day, ct.hour, ct.minute, ct.second, x);

	return (0);
}

void
getversion_nowutc_c()
{
	static char    *x = "$Id: nowutc.c,v 1.3 2016-05-21 14:48:14+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
