/*
 * $Log: 822date.c,v $
 * Revision 1.9  2021-06-13 17:27:04+05:30  Cprogrammer
 * removed chdir(auto_sysconfdir)
 *
 * Revision 1.8  2020-11-24 13:42:12+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2016-05-21 14:47:32+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.6  2016-01-28 10:31:00+05:30  Cprogrammer
 * removed compiler warning for chdir()
 *
 * Revision 1.5  2016-01-28 08:59:06+05:30  Cprogrammer
 * chdir qmail_home for opening etc/leapsecs.dat
 *
 * Revision 1.4  2005-08-23 17:13:59+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 19:50:21+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-17 23:26:29+05:30  Cprogrammer
 * error handling for substdio_puts(), substdio_flush()
 *
 * Revision 1.1  2004-06-16 01:19:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "substdio.h"
#include "strerr.h"
#include "subfd.h"
#include "getln.h"
#include "mess822.h"
#include "leapsecs.h"
#include "caltime.h"
#include "tai.h"
#include "auto_sysconfdir.h"

#define FATAL "822date: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

mess822_time    t;
struct tai      sec;
unsigned char   secpack[TAI_PACK];
time_t          secunix;

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"date", 0, 0, 0, 0, &t}
	, {0, 0, 0, 0, 0, 0}
};

stralloc        line = { 0 };
int             match;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	if (leapsecs_init() == -1)
		strerr_die2sys(111, FATAL, "unable to init leapsecs: ");
	if (argv[1])
		a[0].name = argv[1];
	if (!mess822_begin(&h, a))
		nomem();
	for (;;)
	{
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (!mess822_line(&h, &line))
			nomem();
		if (!match)
			break;
	}
	if (!mess822_end(&h))
		nomem();
	if (!t.known)
		_exit(100);
	if (!stralloc_ready(&line, caltime_fmt((char *) 0, &t.ct)))
		nomem();
	if (substdio_put(subfdoutsmall, line.s, caltime_fmt(line.s, &t.ct)))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_put(subfdoutsmall, "\n", 1))
		strerr_die2sys(111, FATAL, "unable to write: ");
	caltime_tai(&t.ct, &sec);
	caltime_utc(&t.ct, &sec, (int *) 0, (int *) 0);
	if (!stralloc_ready(&line, caltime_fmt((char *) 0, &t.ct)))
		nomem();
	if (substdio_put(subfdoutsmall, line.s, caltime_fmt(line.s, &t.ct)))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_put(subfdoutsmall, "\n", 1))
		strerr_die2sys(111, FATAL, "unable to write: ");
	tai_pack((char *) secpack, &sec);
	secunix = secpack[0] - 64;
	secunix = (secunix << 8) + secpack[1];
	secunix = (secunix << 8) + secpack[2];
	secunix = (secunix << 8) + secpack[3];
	secunix = (secunix << 8) + secpack[4];
	secunix = (secunix << 8) + secpack[5];
	secunix = (secunix << 8) + secpack[6];
	secunix = (secunix << 8) + secpack[7];
	secunix -= 10;
	if (substdio_puts(subfdoutsmall, ctime(&secunix)))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_flush(subfdoutsmall))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(0);
}

void
getversion_822date_c()
{
	static char    *x = "$Id: 822date.c,v 1.9 2021-06-13 17:27:04+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
