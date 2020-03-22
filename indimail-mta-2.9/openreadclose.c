/*
 * $Log: openreadclose.c,v $
 * Revision 1.2  2004-10-22 20:27:49+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 21:26:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"
#include "open.h"
#include "openreadclose.h"

int
openreadclose(char *fn, stralloc * sa, unsigned int bufsize)
{
	int             fd;

	if ((fd = open_read(fn)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	if (readclose(fd, sa, bufsize) == -1)
		return -1;
	return 1;
}

int
readclose_append(int fd, stralloc * sa, unsigned int bufsize)
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

int
readclose(int fd, stralloc * sa, unsigned int bufsize)
{
	if (!stralloc_copys(sa, ""))
	{
		close(fd);
		return -1;
	}
	return readclose_append(fd, sa, bufsize);
}

void
getversion_openreadclose_c()
{
	static char    *x = "$Id: openreadclose.c,v 1.2 2004-10-22 20:27:49+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
