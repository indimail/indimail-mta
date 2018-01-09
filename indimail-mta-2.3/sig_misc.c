/*
 * $Log: sig_misc.c,v $
 * Revision 1.3  2004-10-22 20:30:25+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:10+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <signal.h>
#include "sig.h"

void
sig_miscignore()
{
	sig_catch(SIGVTALRM, SIG_IGN);
	sig_catch(SIGPROF, SIG_IGN);
	sig_catch(SIGQUIT, SIG_IGN);
	sig_catch(SIGINT, SIG_IGN);
	sig_catch(SIGHUP, SIG_IGN);
#ifdef SIGXCPU
	sig_catch(SIGXCPU, SIG_IGN);
#endif
#ifdef SIGXFSZ
	sig_catch(SIGXFSZ, SIG_IGN);
#endif
}

void
getversion_sig_misc_c()
{
	static char    *x = "$Id: sig_misc.c,v 1.3 2004-10-22 20:30:25+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
