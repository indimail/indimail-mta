/*
 * $Log: slurpclose.c,v $
 * Revision 1.7  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2004-10-22 20:30:28+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:39:19+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.3  2004-07-17 21:23:19+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "stralloc.h"
#include "slurpclose.h"
#include "error.h"

int
slurpclose(int fd, stralloc *sa, int bufsize)
{
	int             r;
	for (;;)
	{
		if (!stralloc_readyplus(sa, bufsize))
		{
			close(fd);
			return -1;
		}
		r = read(fd, sa->s + sa->len, bufsize);
		if (r == -1)
			if (errno == error_intr)
				continue;
		if (r <= 0)
		{
			close(fd);
			return r;
		}
		sa->len += r;
	}
}

void
getversion_slurpclose_c()
{
	const char     *x = "$Id: slurpclose.c,v 1.7 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
