/*
 * $Log: trysgact.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <signal.h>

main()
{
	struct sigaction sa;
	sa.sa_handler = 0;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(0, &sa, (struct sigaction *) 0);
}
