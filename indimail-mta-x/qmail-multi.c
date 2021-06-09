/*
 * $Log: $
 */
#include <unistd.h>
#include <sig.h>
#include <env.h>
#include "auto_qmail.h"
#include "qmulti.h"
#include "mailfilter.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */

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
main(int argc, char **argv)
{
	char           *filterargs;

	if (chdir(auto_qmail) == -1)
		_exit(61);
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
	static char    *x = "$Id: qmail-multi.c,v 1.53 2021-05-29 23:49:39+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidqmultih;
	x = sccsidmailfilterh;
	x++;
}
#endif
