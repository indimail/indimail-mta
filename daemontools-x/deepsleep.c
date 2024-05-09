/*
 * $Log: deepsleep.c,v $
 * Revision 1.2  2004-10-22 20:24:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:30:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "iopause.h"
#include "deepsleep.h"

void
deepsleep(unsigned int s)
{
	struct taia     now;
	struct taia     deadline;
	iopause_fd      x;

	taia_now(&now);
	taia_uint(&deadline, s);
	taia_add(&deadline, &now, &deadline);

	for (;;)
	{
		taia_now(&now);
		if (taia_less(&deadline, &now))
			return;
		iopause(&x, 0, &deadline, &now);
	}
}

void
getversion_deepsleep_c()
{
	const char     *x = "$Id: deepsleep.c,v 1.2 2004-10-22 20:24:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
