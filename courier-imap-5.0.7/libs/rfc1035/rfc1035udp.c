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
#include	<errno.h>
#include	<stdlib.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
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

int rfc1035_open_udp(int *af)
{
	return (rfc1035_mksocket(SOCK_DGRAM, 0, af));
}

int rfc1035_send_udp(int fd, const struct sockaddr *sin, int sin_len,
	const char *query, unsigned query_len)
{
	if (sox_sendto(fd, (const char *)query, query_len, 0,
		sin, sin_len) == query_len)
		return (0);
	return (-1);
}

static int dorecv(int fd, char *bufptr, int buflen, int flags,
		struct sockaddr *addr, socklen_t *addrlen)
{
socklen_t len;

	do
	{
		len=sox_recvfrom(fd, bufptr, buflen, flags, addr, addrlen);
	} while (len < 0 && errno == EINTR);
	return (len);
}

char *rfc1035_recv_udp(int fd,
	const struct sockaddr *addrshouldfrom, int addrshouldfrom_len,
	int *buflen, const char *query)
{
int	len;

#if	RFC1035_IPV6

struct sockaddr_storage addrfrom;

#else

struct sockaddr_in addrfrom;

#endif

socklen_t addrfromlen;
char	rfc1035_buf[512];
char	*bufptr=rfc1035_buf;
char	*mallocedbuf=0;

	*buflen=sizeof(rfc1035_buf);

	while ((len=dorecv(fd, bufptr, *buflen, MSG_PEEK, 0, 0)) >= *buflen )
	{
		if (len == *buflen)	len += 511;
		++len;

		if (mallocedbuf)	free(mallocedbuf);
		mallocedbuf=(char *)malloc(len);
		if (!mallocedbuf)	return (0);
		bufptr= mallocedbuf;
		*buflen=len;
	}

	addrfromlen=sizeof(addrfrom);
	if (len < 0 || (len=dorecv(fd, bufptr, *buflen, 0,
		(struct sockaddr *)&addrfrom, &addrfromlen)) < 0)
	{
		if (mallocedbuf)
			free(mallocedbuf);
		errno=EIO;
		return (0);
	}

	*buflen=len;

	if ( !rfc1035_same_ip( &addrfrom, addrfromlen,
				addrshouldfrom, addrshouldfrom_len))
	{
		if (mallocedbuf)
			free(mallocedbuf);

		errno=EAGAIN;
		return (0);
	}

	if ( *buflen < 2)
	{
		if (mallocedbuf)
			free(mallocedbuf);
		errno=EIO;
		return (0);
	}

	if ( query && (bufptr[0] != query[0] || bufptr[1] != query[1]
		|| (unsigned char)(bufptr[2] & 0x80) == 0 ))
	{
		if (mallocedbuf)
			free(mallocedbuf);
		errno=EAGAIN;
		return (0);
	}
	if (!mallocedbuf)
	{
		if ((mallocedbuf=malloc( *buflen )) == 0)
			return (0);

		memcpy(mallocedbuf, bufptr, *buflen);
		bufptr=mallocedbuf;
	}
	return (bufptr);
}

char *rfc1035_query_udp(struct rfc1035_res *res,
	int fd, const struct sockaddr *sin, int sin_len,
	const char *query, unsigned query_len, int *buflen, unsigned w)
{
time_t current_time, final_time;
char	*rc;

	time(&current_time);

	if (rfc1035_send_udp(fd, sin, sin_len, query, query_len))
		return (0);

	final_time=current_time+w;

	while (current_time < final_time)
	{
		if (rfc1035_wait_reply(fd, final_time-current_time))
			break;

		rc=rfc1035_recv_udp(fd, sin, sin_len, buflen, query);
		if (rc)	return (rc);

		if (errno != EAGAIN)	break;

		time(&current_time);
	}
	errno=EAGAIN;
	return (0);
}
