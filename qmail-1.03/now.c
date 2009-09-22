/*
 * $Log: now.c,v $
 * Revision 1.3  2004-10-22 20:27:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:19:58+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <time.h>
#include "datetime.h"
#include "now.h"

datetime_sec
now()
{
	return time((long *) 0);
}

void
getversion_now_c()
{
	static char    *x = "$Id: now.c,v 1.3 2004-10-22 20:27:44+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
