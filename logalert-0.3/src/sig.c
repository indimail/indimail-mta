#include "config.h"

#ifdef HAVE_ALARM
#include <signal.h>
#endif

#include "defs.h"
#include "monitor.h"

extern unsigned int active;

void
sig_handle_abort(int sig)
{

	debug("\n[!] Caught signal %d, exiting ...\n",sig);
	exit(0);

}

void
sig_handle_timer(uint timeout)
{
        active = YES;
}

void sig_init()
{
        signal(SIGALRM, sig_handle_timer);

        signal(SIGINT, sig_handle_abort);
        signal(SIGTERM, sig_handle_abort);

}

