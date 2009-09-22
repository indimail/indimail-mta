/*
 * $Log: wait_nohang.c,v $
 * Revision 1.3  2004-10-22 20:32:10+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:25:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/wait.h>
#include "haswaitp.h"

int
wait_nohang(wstat)
	int            *wstat;
{
#ifdef HASWAITPID
	return waitpid(-1, wstat, WNOHANG);
#else
	return wait3(wstat, WNOHANG, (struct rusage *) 0);
#endif
}

void
getversion_wait_nohang_c()
{
	static char    *x = "$Id: wait_nohang.c,v 1.3 2004-10-22 20:32:10+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
