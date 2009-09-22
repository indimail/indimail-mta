/*
 * $Log: seek_set.c,v $
 * Revision 1.4  2004-10-22 19:39:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:22:48+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "seek.h"

#define SET 0	/*- sigh */

int
seek_set(fd, pos)
	int             fd;
	seek_pos        pos;
{
	if (lseek(fd, (off_t) pos, SEEK_SET) == -1)
		return -1;
	return 0;
}
void
getversion_seek_set_c()
{
	static char    *x = "$Id: seek_set.c,v 1.4 2004-10-22 19:39:26+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
