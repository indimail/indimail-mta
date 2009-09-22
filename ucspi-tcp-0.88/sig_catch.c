/*
 * $Log: sig_catch.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <signal.h>
#include "sig.h"
#include "hassgact.h"

void
sig_catch(int sig, void (*f) ())
{
#ifdef HASSIGACTION
	struct sigaction sa;
	sa.sa_handler = f;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, (struct sigaction *) 0);
#else
	signal(sig, f);				/*- won't work under System V, even nowadays---dorks */
#endif
}
