/*
 * $Log: timeoutio.c,v $
 * Revision 2.2  2008-07-13 19:48:29+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.1  2002-12-11 10:29:04+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 1.1  2002-04-10 02:59:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#ifndef	lint
static char     sccsid[] = "$Id: timeoutio.c,v 2.2 2008-07-13 19:48:29+05:30 Cprogrammer Stab mbhangui $";
#endif

int
timeoutread(t, fd, buf, len)
	int             t;
	int             fd;
	char           *buf;
	int             len;
{
	fd_set          rfds;
	int             ret;
	struct timeval  tv;
	struct timeval *tvptr;

	if(t)
	{
		tv.tv_sec = t;
		tv.tv_usec = 0;
		tvptr = &tv;
	} else
		tvptr = (struct timeval *) 0;
	for(;;)
	{
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		if (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, tvptr) == -1)
		{
#ifdef ERESTART
			if(errno == EINTR || errno == ERESTART)
#else
			if(errno == EINTR)
#endif
				continue;
			return -1;
		}
		break;
	}
	if (FD_ISSET(fd, &rfds))
	{
		for(;;)
		{
			if((ret = read(fd, buf, len)) == -1)
			{
#ifdef ERESTART
				if(errno == EINTR || errno == ERESTART)
#else
				if(errno == EINTR)
#endif
					continue;
			}
			return(ret);
		}
	}
	errno = ETIMEDOUT;
	return -1;
}

int
timeoutwrite(t, fd, buf, len)
	int             t;
	int             fd;
	char           *buf;
	int             len;
{
	fd_set          wfds;
	int             ret;
	struct timeval  tv;
	struct timeval *tvptr;

	if(t)
	{
		tv.tv_sec = t;
		tv.tv_usec = 0;
		tvptr = &tv;
	} else
		tvptr = (struct timeval *) 0;
	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);
	for(;;)
	{
		if (select(fd + 1, (fd_set *) 0, &wfds, (fd_set *) 0, tvptr) == -1)
		{
#ifdef ERESTART
			if(errno == EINTR || errno == ERESTART)
#else
			if(errno == EINTR)
#endif
				continue;
			return -1;
		}
		break;
	}
	if (FD_ISSET(fd, &wfds))
	{
		for(;;)
		{
			if((ret = write(fd, buf, len)) == -1)
			{
#ifdef ERESTART
				if(errno == EINTR || errno == ERESTART)
#else
				if(errno == EINTR)
#endif
					continue;
			}
			return(ret);
		}
	}
	errno = ETIMEDOUT;
	return -1;
}

void
getversion_timeoutio_c()
{
	printf("%s\n", sccsid);
}
