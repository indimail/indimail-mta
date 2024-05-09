/*
 * $Log: chkspawn.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2020-11-24 13:44:31+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.5  2004-10-22 20:23:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:17:37+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "fmt.h"
#include "select.h"
#include "auto_spawn.h"

char            num[FMT_ULONG];
fd_set          fds;

int
main()
{
	unsigned long   hiddenlimit;
	unsigned long   maxnumd;

	hiddenlimit = sizeof(fds) * 8;
	maxnumd = (hiddenlimit - 5) / 2;

	if (auto_spawn < 1)
	{
		substdio_puts(subfderr, "Oops. You have set conf-spawn lower than 1.\n");
		substdio_flush(subfderr);
		_exit(1);
	}
	if (auto_spawn > 65000)
	{
		substdio_puts(subfderr, "Oops. You have set conf-spawn higher than 65000.\n");
		substdio_flush(subfderr);
		_exit(1);
	}
	if (auto_spawn > maxnumd)
	{
		substdio_puts(subfderr, "Oops. Your system's FD_SET() has a hidden limit of ");
		substdio_put(subfderr, num, fmt_ulong(num, hiddenlimit));
		substdio_puts(subfderr, " descriptors.\n\
This means that the qmail daemons could crash if you set the run-time\n\
concurrency higher than ");
		substdio_put(subfderr, num, fmt_ulong(num, maxnumd));
		substdio_puts(subfderr, ". So I'm going to insist that the concurrency\n\
limit in conf-spawn be at most ");
		substdio_put(subfderr, num, fmt_ulong(num, maxnumd));
		substdio_puts(subfderr, ". Right now it's ");
		substdio_put(subfderr, num, fmt_ulong(num, (unsigned long) auto_spawn));
		substdio_puts(subfderr, ".\n");
		substdio_flush(subfderr);
		_exit(1);
	}
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_chkspawn_c()
{
	const char     *x = "$Id: chkspawn.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
