/*
 * $Log: sig_term.c,v $
 * Revision 1.3  2004-10-22 20:30:27+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <signal.h>
#include "sig.h"

void
sig_termblock()
{
	sig_block(SIGTERM);
}

void
sig_termunblock()
{
	sig_unblock(SIGTERM);
}

void
sig_termcatch(f)
	void            (*f) ();
{
	sig_catch(SIGTERM, f);
}

void
sig_termdefault()
{
	sig_catch(SIGTERM, SIG_DFL);
}

void
getversion_sig_term_c()
{
	static char    *x = "$Id: sig_term.c,v 1.3 2004-10-22 20:30:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
