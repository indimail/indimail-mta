/*
 * $Log: sig_alarm.c,v $
 * Revision 1.3  2004-10-22 20:30:19+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:22:56+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <signal.h>
#include "sig.h"

void
sig_alarmblock()
{
	sig_block(SIGALRM);
}

void
sig_alarmunblock()
{
	sig_unblock(SIGALRM);
}

void
sig_alarmcatch(f)
	void            (*f) ();
{
	sig_catch(SIGALRM, f);
}

void
sig_alarmdefault()
{
	sig_catch(SIGALRM, SIG_DFL);
}

void
getversion_sig_alarm_c()
{
	static char    *x = "$Id: sig_alarm.c,v 1.3 2004-10-22 20:30:19+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
