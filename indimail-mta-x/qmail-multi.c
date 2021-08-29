/*
 * $Log: qmail-multi.c,v $
 * Revision 1.2  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.1  2021-06-12 18:21:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sig.h>
#include <env.h>
#include <noreturn.h>
#include "qmulti.h"
#include "mailfilter.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */

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
main(int argc, char **argv)
{
	char           *filterargs;

	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	filterargs = env_get("FILTERARGS");
	return (filterargs ? mailfilter(argc, argv, filterargs) : qmulti(0, argc, argv));
}

#ifndef	lint
void
getversion_qmail_multi_c()
{
	static char    *x = "$Id: qmail-multi.c,v 1.2 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidqmultih;
	x = sccsidmailfilterh;
	x++;
}
#endif
