/*
 * $Log: qmail-nullqueue.c,v $
 * Revision 1.4  2011-05-17 21:21:11+05:30  Cprogrammer
 * added timeout
 *
 * Revision 1.3  2004-10-22 20:28:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-15 23:32:39+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2003-10-11 00:04:43+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Send Mails to Trash
 */
#include <unistd.h>
#include "sig.h"
#include "scan.h"
#include "env.h"

#define DEATH 300	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */

void
sigalrm()
{
	/*- thou shalt not clean up here */
	_exit(52);
}

void
sigbug()
{
	_exit(81);
}

int
main()
{
	int             n;
	char           *x;
	char            buf[2048];

	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	if (!(x = env_get("DEATH")))
		n = DEATH;
	else
		scan_int(x, &n);
	alarm(n);
	for (;;)
	{
		if ((n = read(0, buf, sizeof(buf))) == -1)
			_exit(54);
		if (!n)
			break;
	}
	for (;;)
	{
		if ((n = read(1, buf, sizeof(buf))) == -1)
			_exit(54);
		if (!n)
			break;
	}
	_exit (0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_nullqueue_c()
{
	static char    *x = "$Id: qmail-nullqueue.c,v 1.4 2011-05-17 21:21:11+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
