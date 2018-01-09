/*
 * $Log: sig_int.c,v $
 * Revision 1.1  2016-03-31 16:13:57+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include <signal.h>
#include "sig.h"

void
sig_intblock()
{
	sig_block(SIGINT);
}

void
sig_intunblock()
{
	sig_unblock(SIGINT);
}

void
sig_intcatch(f)
	void            (*f) ();
{
	sig_catch(SIGINT, f);
}

void
sig_intdefault()
{
	sig_catch(SIGINT, SIG_DFL);
}

void
getversion_sig_int_c()
{
	static char    *x = "$Id: sig_int.c,v 1.1 2016-03-31 16:13:57+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
