/*
 * $Log: nowutc.c,v $
 * Revision 1.5  2021-04-29 20:27:35+05:30  Cprogrammer
 * renamed variable now to cur to avoid clash with now() function.
 *
 * Revision 1.4  2020-05-11 11:03:11+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
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
#include <error.h>
#include <strerr.h>
#include "leapsecs.h"
#include "tai.h"
#include "taia.h"
#include "caltime.h"
#include "auto_sysconfdir.h"

#define FATAL "nowutc: fatal: "

struct taia     cur;
struct tai      sec;
struct caltime  ct;

char            x[TAIA_FMTFRAC];

int
main()
{
	if (chdir(auto_sysconfdir) == -1)
		strerr_die4sys(111, FATAL, "chdir: ", auto_sysconfdir, ": ");
	if (leapsecs_init() == -1)
		strerr_die2x(111, FATAL, "unable to initialize leapsecs");

	taia_now(&cur);
	x[taia_fmtfrac(x, &cur)] = 0;

	taia_tai(&cur, &sec);
	caltime_utc(&ct, &sec, (int *) 0, (int *) 0);

	printf("%ld-%02d-%02d %02d:%02d:%02d.%s\n", ct.date.year, ct.date.month, ct.date.day, ct.hour, ct.minute, ct.second, x);

	return (0);
}

void
getversion_nowutc_c()
{
	static char    *z = "$Id: nowutc.c,v 1.5 2021-04-29 20:27:35+05:30 Cprogrammer Exp mbhangui $";

	z++;
}
