/*
 * $Log: triggerpull.c,v $
 * Revision 1.6  2020-09-16 19:08:30+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.5  2020-06-08 22:52:26+05:30  Cprogrammer
 * quench compiler warning
 *
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

	if((fd = open_write("lock/trigger")) >= 0) {
		ndelay_on(fd);
		if (write(fd, "", 1) == -1)
			; /*- if it fails, bummer */
		close(fd);
	}
}

void
getversion_triggerpull_c()
{
	const char     *x = "$Id: triggerpull.c,v 1.6 2020-09-16 19:08:30+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
