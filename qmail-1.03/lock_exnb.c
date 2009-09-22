/*
 * $Log: lock_exnb.c,v $
 * Revision 1.4  2004-10-22 20:26:07+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:19:23+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include "hasflock.h"
#include "lock.h"

#ifdef HASFLOCK
int
lock_exnb(fd)
	int             fd;
{
	return flock(fd, LOCK_EX | LOCK_NB);
}
#else
#include <unistd.h>
int
lock_exnb(fd)
	int             fd;
{
	return lockf(fd, 2, 0);
}
#endif

void
getversion_lock_exnb_c()
{
	static char    *x = "$Id: lock_exnb.c,v 1.4 2004-10-22 20:26:07+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
