/*
 * $Log: parsedate.c,v $
 * Revision 1.7  2023-01-03 16:39:03+05:30  Cprogrammer
 * removed auto_sysconfdir.h dependency
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2021-06-13 17:28:51+05:30  Cprogrammer
 * removed chdir(auto_sysconfdir)
 *
 * Revision 1.4  2016-05-21 14:48:24+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.3  2016-01-28 09:01:37+05:30  Cprogrammer
 * chdir qmail_home for opening etc/leapsecs.dat
 *
 * Revision 1.2  2004-10-22 20:27:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <strerr.h>
#include <subfd.h>
#include <getln.h>
#include <mess822.h>
#include <leapsecs.h>
#include <caltime.h>
#include <tai.h>
#include <noreturn.h>

#define FATAL "parsedate: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}


int
main()
{
	int             i, match;
	struct tai      sec;
	stralloc        line = { 0 };
	mess822_time    t;

	if (leapsecs_init() == -1)
		strerr_die2sys(111, FATAL, "unable to init leapsecs: ");

	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!line.len)
			break;
		if (match)
			--line.len;

		for (i = 0; i < line.len; ++i)
			if (line.s[i] == 0)
				line.s[i] = '\n';
		if (!stralloc_0(&line))
			nomem();

		t.known = 0;
		if (!mess822_when(&t, line.s))
			nomem();

		if (t.known) {
			if (!stralloc_ready(&line, caltime_fmt((char *) 0, &t.ct)))
				nomem();
			substdio_put(subfdoutsmall, line.s, caltime_fmt(line.s, &t.ct));
			if (t.known == 1)
				substdio_put(subfdoutsmall, "?", 1);
			substdio_put(subfdoutsmall, "\t", 1);

			caltime_tai(&t.ct, &sec);

			caltime_utc(&t.ct, &sec, (int *) 0, (int *) 0);
			if (!stralloc_ready(&line, caltime_fmt((char *) 0, &t.ct)))
				nomem();
			substdio_put(subfdoutsmall, line.s, caltime_fmt(line.s, &t.ct));
		}

		substdio_puts(subfdoutsmall, "\n");
		if (!match)
			break;
	}

	substdio_flush(subfdoutsmall);
	_exit(0);
}

void
getversion_parsedate_c()
{
	const char     *x = "$Id: parsedate.c,v 1.7 2023-01-03 16:39:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
