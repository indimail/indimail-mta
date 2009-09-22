/*
 * $Log: readclose.c,v $
 * Revision 1.2  2008-07-17 23:04:08+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "error.h"
#include "readclose.h"
#include <unistd.h>

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
