/*
 * $Log: wait_nohang.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
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
