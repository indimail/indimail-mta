/*
 * $Log: strerr_sys.c,v $
 * Revision 1.4  2008-07-14 20:58:18+05:30  Cprogrammer
 * initialize strerr_sys
 *
 * Revision 1.3  2004-10-22 20:30:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:20+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "error.h"
#include "strerr.h"

struct strerr   strerr_sys = {0, 0, 0, 0};

void
strerr_sysinit()
{
	strerr_sys.who = 0;
	strerr_sys.x = error_str(errno);
	strerr_sys.y = "";
	strerr_sys.z = "";
}

void
getversion_strerr_sys_c()
{
	static char    *x = "$Id: strerr_sys.c,v 1.4 2008-07-14 20:58:18+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
