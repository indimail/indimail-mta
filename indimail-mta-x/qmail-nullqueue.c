/*
 * $Id: qmail-nullqueue.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <sig.h>
#include <scan.h>
#include <env.h>
#include <getEnvConfig.h>
#include <noreturn.h>
#include "qmail.h"

no_return void
sigalrm()
{
	/*- thou shalt not clean up here */
	_exit(52);
}

no_return void
sigbug()
{
	_exit(81);
}

int
main()
{
	int             n;
	unsigned long   death;
	char            buf[8192];

	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	getEnvConfiguLong(&death, "DEATH", DEATH);
	alarm(death);
	for (;;) {
		if ((n = read(0, buf, sizeof(buf))) == -1)
			_exit(54);
		if (!n)
			break;
	}
	for (;;) {
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
	const char     *x = "$Id: qmail-nullqueue.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail-nullqueue.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-12-25 09:30:39+05:30  Cprogrammer
 * made DEATH configurable
 *
 * Revision 1.5  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
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
