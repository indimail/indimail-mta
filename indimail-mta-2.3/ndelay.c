/*
 * $Log: ndelay.c,v $
 * Revision 1.3  2004-10-22 20:27:39+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:19:52+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "ndelay.h"

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

int
ndelay_on(fd)
	int             fd;
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void
getversion_ndelay_c()
{
	static char    *x = "$Id: ndelay.c,v 1.3 2004-10-22 20:27:39+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
