/*
 * $Log: trysgprm.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <signal.h>

main()
{
	sigset_t        ss;

	sigemptyset(&ss);
	sigaddset(&ss, SIGCHLD);
	sigprocmask(SIG_SETMASK, &ss, (sigset_t *) 0);
}
