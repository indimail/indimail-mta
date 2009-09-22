/*
 * $Log: seek_end.c,v $
 * Revision 1.4  2004-10-22 20:30:10+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:22:45+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "seek.h"

#define END 2	/*- sigh */

int
seek_end(fd)
	int             fd;
{
	if (lseek(fd, (off_t) 0, SEEK_END) == -1)
		return -1;
	return 0;
}

void
getversion_seek_end_c()
{
	static char    *x = "$Id: seek_end.c,v 1.4 2004-10-22 20:30:10+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
