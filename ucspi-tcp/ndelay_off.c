/*
 * $Log: ndelay_off.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "ndelay.h"

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

int
ndelay_off(int fd)
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);
}
