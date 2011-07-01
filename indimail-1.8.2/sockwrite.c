/*
 * $Log: sockwrite.c,v $
 * Revision 2.3  2008-07-13 19:47:25+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.2  2002-12-11 10:28:50+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.1  2002-06-22 01:45:58+05:30  Cprogrammer
 * return 0 if write returns 0 bytes
 *
 * Revision 1.1  2001-12-08 00:32:06+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: sockwrite.c,v 2.3 2008-07-13 19:47:25+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int
sockwrite(fd, wbuf, len)
	int             fd;
	char           *wbuf;
	int             len;
{
	char           *ptr;
	int             rembytes, wbytes;

	for (ptr = wbuf, rembytes = len; rembytes;)
	{
		for (;;)
		{
			errno = 0;
			if ((wbytes = write(fd, ptr, rembytes)) == -1)
			{
#ifdef ERESTART
				if (errno == EINTR || errno == ERESTART)
#else
				if (errno == EINTR)
#endif
					continue;
				return (-1);
			} else
			if (!wbytes)
				return(0);
			break;
		}
		ptr += wbytes;
		rembytes -= wbytes;
	}
	return (len);
}

void
getversion_sockwrite_c()
{
	printf("%s\n", sccsid);
}
