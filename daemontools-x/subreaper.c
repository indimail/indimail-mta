/*- $Id: subreaper.c,v 1.1 2022-12-13 20:49:24+05:30 Cprogrammer Exp mbhangui $ */
#include "subreaper.h"

#if defined(linux)
#include <sys/prctl.h>
int
subreaper()
{
	return (prctl(PR_SET_CHILD_SUBREAPER, 1, 0, 0, 0));
}
#elif defined(__FreeBSD__)
#include <sys/procctl.h>
#else
int
subreaper()
{
	return (procctl(P_PID, 0, PROC_REAP_ACQUIRE, 0));
}
#endif
/*
 * $Log: subreaper.c,v $
 * Revision 1.1  2022-12-13 20:49:24+05:30  Cprogrammer
 * Initial revision
 *
 */
