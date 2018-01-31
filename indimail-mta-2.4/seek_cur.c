/*
 * $Log: seek_cur.c,v $
 * Revision 1.4  2004-10-22 20:30:09+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:22:44+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "seek.h"

#define CUR 1	/*- sigh */

seek_pos
seek_cur(fd)
	int             fd;
{
	return lseek(fd, (off_t) 0, SEEK_CUR);
}

void
getversion_seek_cur_c()
{
	static char    *x = "$Id: seek_cur.c,v 1.4 2004-10-22 20:30:09+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
