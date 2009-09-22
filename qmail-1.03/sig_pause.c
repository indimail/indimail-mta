/*
 * $Log: sig_pause.c,v $
 * Revision 1.3  2004-10-22 20:30:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:13+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <signal.h>
#include "sig.h"
#include "hassgprm.h"

void
sig_pause()
{
#ifdef HASSIGPROCMASK
	sigset_t        ss;
	sigemptyset(&ss);
	sigsuspend(&ss);
#else
	sigpause(0);
#endif
}

void
getversion_sig_pause_c()
{
	static char    *x = "$Id: sig_pause.c,v 1.3 2004-10-22 20:30:26+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
