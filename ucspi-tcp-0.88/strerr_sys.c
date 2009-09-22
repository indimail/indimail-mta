/*
 * $Log: strerr_sys.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "error.h"
#include "strerr.h"

struct strerr   strerr_sys;

void
strerr_sysinit(void)
{
	strerr_sys.who = 0;
	strerr_sys.x = error_str(errno);
	strerr_sys.y = "";
	strerr_sys.z = "";
}
