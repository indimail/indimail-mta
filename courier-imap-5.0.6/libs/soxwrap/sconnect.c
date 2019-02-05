/*
** Copyright 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"sconnect.h"

#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>

#include	"soxwrap.h"


int s_connect(int sockfd, const struct sockaddr *addr, size_t addr_s,
	      time_t connect_timeout)
{
	fd_set fdr;
	struct timeval tv;
	int	rc;

#ifdef SOL_KEEPALIVE
	setsockopt(sockfd, SOL_SOCKET, SOL_KEEPALIVE,
		   (const char *)&dummy, sizeof(dummy));
#endif

#ifdef  SOL_LINGER
        {
		struct linger l;

                l.l_onoff=0;
                l.l_linger=0;

                setsockopt(sockfd, SOL_SOCKET, SOL_LINGER,
			   (const char *)&l, sizeof(l));
        }
#endif

        /*
        ** If configuration says to use the kernel's timeout settings,
        ** just call connect, and be done with it.
        */

        if (connect_timeout == 0)
                return ( sox_connect(sockfd, addr, addr_s));

        /* Asynchronous connect with timeout. */

        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)     return (-1);

        if ( sox_connect(sockfd, addr, addr_s) == 0)
        {
                /* That was easy, we're done. */

                if (fcntl(sockfd, F_SETFL, 0) < 0)      return (-1);
                return (0);
        }
        else
                if (errno != EINPROGRESS) return (-1);

	/* Wait for the connection to go through, until the timeout expires */

        FD_ZERO(&fdr);
        FD_SET(sockfd, &fdr);
        tv.tv_sec=connect_timeout;
        tv.tv_usec=0;

        rc=sox_select(sockfd+1, 0, &fdr, 0, &tv);
        if (rc < 0)     return (-1);

        if (!FD_ISSET(sockfd, &fdr))
        {
                errno=ETIMEDOUT;
                return (-1);
        }

	{
		int     gserr;
		socklen_t gslen = sizeof(gserr);

		if (sox_getsockopt(sockfd, SOL_SOCKET,
				   SO_ERROR,
				   (char *)&gserr, &gslen)==0)
		{
			if (gserr == 0)
				return 0;

			errno=gserr;
		}
	}
	return (-1);
}
