/*
 * $Log: lock_sh.c,v $
 * Revision 1.1  2016-03-31 16:14:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include "hasflock.h"
#include "lock.h"

#ifdef HASFLOCK
int
lock_ex(fd)
	int             fd;
{
	return flock(fd, LOCK_SH);
}
#else
#include <unistd.h>
int
lock_ex(fd)
	int             fd;
{
	return lockf(fd, F_TLOCK, 0);
}
#endif

void
getversion_lock_sh_c()
{
	static char    *x = "$Id: lock_sh.c,v 1.1 2016-03-31 16:14:07+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
