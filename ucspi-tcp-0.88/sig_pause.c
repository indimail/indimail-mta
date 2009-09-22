/*
 * $Log: sig_pause.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <signal.h>
#include "sig.h"
#include "hassgprm.h"

void
sig_pause(void)
{
#ifdef HASSIGPROCMASK
	sigset_t        ss;
	sigemptyset(&ss);
	sigsuspend(&ss);
#else
	sigpause(0);
#endif
}
