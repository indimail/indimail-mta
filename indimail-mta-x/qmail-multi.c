/*
 * $Id: qmail-multi.c,v 1.6 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sig.h>
#include <env.h>
#include <getEnvConfig.h>
#include <noreturn.h>
#include "qmulti.h"
#include "mailfilter.h"
#include "qmail.h"

no_return void
sigalrm(int x)
{
	/*- thou shalt not clean up here */
	_exit(QQ_TIMEOUT);
}

no_return void
sigbug(int x)
{
	_exit(QQ_INTERNAL_BUG);
}

int
main(int argc, char **argv)
{
	char           *filterargs;
	unsigned long   death;

	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	getEnvConfiguLong(&death, "DEATH", DEATH);
	alarm(death);
	filterargs = env_get("FILTERARGS");
	return (filterargs ? mailfilter(argc, argv, filterargs) : qmulti(0, argc, argv));
}

#ifndef	lint
void
getversion_qmail_multi_c()
{
	const char     *x = "$Id: qmail-multi.c,v 1.6 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif

/*
 * $Log: qmail-multi.c,v $
 * Revision 1.6  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-12-25 09:30:34+05:30  Cprogrammer
 * made DEATH configurable
 *
 * Revision 1.3  2022-10-17 19:44:45+05:30  Cprogrammer
 * use exit codes defines from qmail.h
 *
 * Revision 1.2  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.1  2021-06-12 18:21:28+05:30  Cprogrammer
 * Initial revision
 *
 */
