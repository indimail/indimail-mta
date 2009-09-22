/*
 * $Log: open_trunc.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int
open_trunc(char *fn)
{
	return open(fn, O_WRONLY | O_NDELAY | O_TRUNC | O_CREAT, 0644);
}
