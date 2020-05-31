/*
 * $Log: seek_set.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include "seek.h"
#include <unistd.h>

#define SET 0					/*- sigh */

int
seek_set(int fd, seek_pos pos)
{
	if (lseek(fd, (off_t) pos, SET) == -1)
		return -1;
	return 0;
}
