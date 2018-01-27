/*
 * $Log: sockread.c,v $
 * Revision 2.2  2008-07-13 19:47:14+05:30  Cprogrammer
 * use ERESTART only if available.
 *
 * Revision 2.1  2002-12-11 10:28:46+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 1.1  2001-12-08 00:32:01+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: sockread.c,v 2.2 2008-07-13 19:47:14+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int
sockread(fd, buffer, len)
	int             fd;
	char           *buffer;
	int             len;
{
	char           *ptr;
	int             rembytes, rbytes, retrycount;

	for (retrycount = 0, rembytes = len, ptr = buffer; rembytes;)
	{
		errno = 0;
		if ((rbytes = read(fd, ptr, rembytes)) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY)
			{
				usleep(1000);
				continue;
			}
#if defined(HPUX_SOURCE)
			if (errno == EREMOTERELEASE)
			{
				rbytes = 0;
				break;
			}
#endif
			return (-1);
		} else
		if (!rbytes)	/* EOF */
			break;;
		rembytes -= rbytes;
		if (!rembytes)
			break;
		ptr += rbytes;
	}
	return (len - rembytes);
}

void
getversion_sockread_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
