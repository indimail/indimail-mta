/*
 * $Log: sig_child.c,v $
 * Revision 1.3  2004-10-22 20:30:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:23:06+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <signal.h>
#include "sig.h"

void
sig_childblock()
{
	sig_block(SIGCHLD);
}

void
sig_childunblock()
{
	sig_unblock(SIGCHLD);
}

void
sig_childcatch(f)
	void            (*f) ();
{
	sig_catch(SIGCHLD, f);
}

void
sig_childdefault()
{
	sig_catch(SIGCHLD, SIG_DFL);
}

void
getversion_sig_child_c()
{
	static char    *x = "$Id: sig_child.c,v 1.3 2004-10-22 20:30:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
