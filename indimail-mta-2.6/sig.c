/*
 * $Log: sig.c,v $
 * Revision 1.2  2004-10-22 20:30:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:31:38+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Public domain. 
 */

#include <signal.h>
#include "sig.h"

int             sig_alarm = SIGALRM;
int             sig_child = SIGCHLD;
int             sig_cont = SIGCONT;
int             sig_hangup = SIGHUP;
int             sig_int = SIGINT;
int             sig_pipe = SIGPIPE;
int             sig_term = SIGTERM;

void            (*sig_defaulthandler) () = SIG_DFL;
void            (*sig_ignorehandler) () = SIG_IGN;

void
getversion_sig_c()
{
	static char    *x = "$Id: sig.c,v 1.2 2004-10-22 20:30:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
