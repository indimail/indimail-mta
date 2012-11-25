/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"rfc1035.h"
#include	<stdio.h>
#include	<string.h>
#include	"soxwrap/soxwrap.h"
#include	<sys/types.h>
#include	<arpa/inet.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>
#include	<errno.h>
#include	<fcntl.h>
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif


int rfc1035_open_tcp(struct rfc1035_res *res, const RFC1035_ADDR *addr)
{
RFC1035_NETADDR addrbuf;
int	af;
const struct sockaddr *addrptr;
int	addrptrlen;

int	fd=rfc1035_mksocket(SOCK_STREAM, 0, &af);

	if (fd < 0)	return (-1);

	if (rfc1035_mkaddress(af, &addrbuf,
		addr, htons(53),
		&addrptr, &addrptrlen))
	{
		close(fd);
		return (-1);
	}

	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY) < 0)
	{
		sox_close(fd);
		return (-1);
	}

	if (sox_connect(fd, addrptr, addrptrlen) == 0)
		return (fd);

	if (errno == EINPROGRESS || errno == EAGAIN)
	{
	unsigned	w=res->rfc1035_timeout_initial;

		if (!w)	w=RFC1035_DEFAULT_INITIAL_TIMEOUT;
		if (rfc1035_wait_query(fd, w) == 0 &&
			(sox_connect(fd, addrptr, addrptrlen) == 0
				|| errno == EISCONN))
		{
			return (fd);
		}
	}
	sox_close(fd);
	return (-1);
}

int rfc1035_send_tcp(int fd, const char *query, unsigned query_len)
{
int	n=query_len+2;
char	*buf=malloc(n);
char	*p;
int	pl;

	if (!buf)	return (-1);

	buf[0]=query_len >> 8;
	buf[1]=query_len;

	memcpy(buf+2, query, query_len);

	p=buf;
	pl=n;
	while (pl)
	{
	int	i=sox_write(fd, p, pl);

		if (i < 0 && errno == EINTR)	continue;
		if (i <= 0)	break;
		p += i;
		pl -= i;
	}
	free(buf);

	if (pl)	return (-1);
	return (0);
}

static int doread(int fd, char *buf, int bufsize)
{
int	len;

	do
	{
		len=sox_read(fd, buf, bufsize);
	} while (len < 0 && errno == EINTR);
	return (len);
}

char *rfc1035_recv_tcp(struct rfc1035_res *res, int fd, int *buflen, unsigned w)
{
int	len;
unsigned response_len;
char lenbuf[2];
char	*mallocedbuf=0;
time_t	current_time, finish_time;

	time(&current_time);
	finish_time=current_time+w;

	if (rfc1035_wait_reply(fd, w))
		return (0);

	len=doread(fd, lenbuf, 2);
	if (len <= 0)
	{
		errno=EIO;
		return (0);
	}
	if (len == 1)
	{
		time(&current_time);
		if (current_time >= finish_time)	return (0);
		if (rfc1035_wait_reply(fd, finish_time - current_time))
			return (0);

		len=doread(fd, lenbuf+1, 1);
		if (len <= 0)
		{
			errno=EIO;
			return (0);
		}
		++len;
	}

	response_len= ((unsigned)(unsigned char)lenbuf[0] << 8)
			| (unsigned char)lenbuf[1];

	if ((mallocedbuf=malloc(response_len)) == 0)
		return (0);

	*buflen=0;

	while ((unsigned)*buflen < response_len)
	{
		time(&current_time);
		if (current_time >= finish_time ||
			rfc1035_wait_reply(fd, finish_time - current_time))
		{
			len=0;
			errno=ETIMEDOUT;
		}
		else
			len=doread(fd, mallocedbuf + *buflen,
						response_len - *buflen);
		if (len <= 0)
		{
			free(mallocedbuf);
			return (0);
		}
		*buflen += len;
	}
	return (mallocedbuf);
}

char *rfc1035_query_tcp(struct rfc1035_res *res,
	int fd, const char *query, unsigned query_len,
	int *buflen, unsigned s)
{
char	*p;

	if ( rfc1035_send_tcp(fd, query, query_len) < 0 ||
		(p=rfc1035_recv_tcp(res, fd, buflen, s)) == 0)
		return (0);
	return (p);
}
