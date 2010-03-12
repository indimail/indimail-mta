/*
 * $Log: sig.c,v $
 * Revision 1.2  2010-03-12 08:59:06+05:30  Cprogrammer
 * added SIGUSR1
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <signal.h>
#include "sig.h"

int             sig_alarm = SIGALRM;
int             sig_child = SIGCHLD;
int             sig_cont = SIGCONT;
int             sig_hangup = SIGHUP;
int             sig_pipe = SIGPIPE;
int             sig_term = SIGTERM;
int             sig_usr1 = SIGUSR1;

void            (*sig_defaulthandler) () = SIG_DFL;
void            (*sig_ignorehandler) () = SIG_IGN;
