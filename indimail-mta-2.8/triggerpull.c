/*
 * $Log: triggerpull.c,v $
 * Revision 1.4  2004-10-22 20:31:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:24:59+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "ndelay.h"
#include "open.h"
#include "triggerpull.h"

void
triggerpull()
{
	int             fd;

	if((fd = open_write("lock/trigger")) >= 0)
	{
		ndelay_on(fd);
		write(fd, "", 1);		/*- if it fails, bummer */
		close(fd);
	}
}

void
getversion_triggerpull_c()
{
	static char    *x = "$Id: triggerpull.c,v 1.4 2004-10-22 20:31:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
