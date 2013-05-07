/* $Id: sighandler.c 6712 2008-04-19 02:33:29Z relson $ */

/*****************************************************************************

NAME:
   sighandler.c -- signal handler

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "sighandler.h"
#include "wordlists.h"

/* Global Definitions */

sig_atomic_t fDie = false;

/* Function Definitions */

static void mysignal(int sig, void (*hdl)(int)) {
#ifdef	SA_RESTART
    struct sigaction sa;

    memset( &sa, 0, sizeof(sa));
    sa.sa_handler = hdl;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(sig, &sa, NULL)) {
	fprintf(stderr, "Cannot set signal %d handler to %p: %s\n",
		sig, hdl, strerror(errno));
	exit(EX_ERROR);
    }
#endif
}

static void mysigdie(int sig)
{
    (void) sig;		/* suppress compiler warning */

    if (!fDie)
	fDie = true;
    else
	exit(EX_ERROR);
    /* XXX FIXME: Need something that is async-signal-safe here!

       Note that _exit bypasses atexit, so we may see database
       corruption! */
}

void signal_setup(void)
{
    mysignal(SIGINT,  mysigdie);	/*  2 */
    mysignal(SIGPIPE, SIG_IGN);		/*  1 */
    mysignal(SIGTERM, mysigdie);	/* 15 */
}
