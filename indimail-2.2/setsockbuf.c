/*
 * $Log: setsockbuf.c,v $
 * Revision 1.1  2001-12-07 22:29:02+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: setsockbuf.c,v 1.1 2001-12-07 22:29:02+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

/* set the socket buffer sizes */
int
setsockbuf(fd, option, size)
	int             fd, option, size;
{
	int             len, retrycount;

	len = size;
	for (retrycount = 0; retrycount < MAXNOBUFRETRY; retrycount++)
	{
		if (setsockopt(fd, SOL_SOCKET, option, (void *) &len, sizeof(int)) == -1)
		{
			if (errno == ENOBUFS)
			{
				usleep(1000);
				continue;
			}
			close(fd);
			return (-1);
		}
		break;
	}
	return (errno = 0);
}

void
getversion_setsockbuf_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
