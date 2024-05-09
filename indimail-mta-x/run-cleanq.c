/*
 * $Log: run-cleanq.c,v $
 * Revision 1.6  2020-10-09 17:31:18+05:30  Cprogrammer
 * use SERVICEDIR define for qscanq path
 *
 * Revision 1.5  2020-10-08 22:55:35+05:30  Cprogrammer
 * servicedir changed to libexecdir/service
 *
 * Revision 1.4  2004-10-22 20:30:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-09-19 22:48:47+05:30  Cprogrammer
 * changed service name to qscanq
 *
 * Revision 1.2  2004-07-17 21:01:14+05:30  Cprogrammer
 * run-cleanq.c
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <strerr.h>
#include "auto_libexec.h"

#define FATAL   "run-cleanq: fatal: "

int
main()
{
#ifdef SERVICEDIR
	execlp("svc", "svc", "-o", SERVICEDIR"/qscanq", (char *) 0);
#else
	if (chdir(auto_libexec) == -1)
		strerr_die4sys(111, FATAL, "chdir :", auto_libexec, ": ");
	execlp("svc", "svc", "-o", "service/qscanq", (char *) 0);
#endif
	_exit(111);	/*- hopefully never reached */ ;
}

void
getversion_run_cleanq_c()
{
	const char     *x = "$Id: run-cleanq.c,v 1.6 2020-10-09 17:31:18+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
