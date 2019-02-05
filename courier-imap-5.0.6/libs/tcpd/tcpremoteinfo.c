/*
** Copyright 1998 - 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"tcpremoteinfo.h"
#include	"soxwrap/sconnect.h"

#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>

#include	"soxwrap/soxwrap.h"


const char *tcpremoteinfo(const RFC1035_ADDR *laddr, int lport,
	const RFC1035_ADDR *raddr, int rport, const char **ostype)
{
int	fd;
time_t	current_time, max_time;
fd_set	fds;
struct	timeval	tv;
static char buf[512];
char	*bufptr;
int	bufleft, n;
char	*p;
char	*q;
RFC1035_NETADDR	sin;
const struct sockaddr *addr;
int	addrlen;

	fd=rfc1035_mksocket(SOCK_STREAM, 0, &n);
	if (fd < 0)	return (0);

	if (rfc1035_mkaddress(n, &sin, laddr, 0, &addr, &addrlen) < 0)
	{
		close(fd);
		return (0);
	}

	if (sox_bind(fd, addr, addrlen) < 0)
	{
		sox_close(fd);
		return (0);
	}

	time (&current_time);
	max_time=current_time+30;

	if (rfc1035_mkaddress(n, &sin, raddr, htons(113), &addr, &addrlen) < 0)
	{
		sox_close(fd);
		return (0);
	}

	if (s_connect(fd, addr, addrlen, max_time - current_time) < 0)
	{
		sox_close(fd);
		return (0);
	}

	sprintf(buf, "%d,%d\r\n", ntohs(rport), ntohs(lport));
	bufptr=buf;
	bufleft=strlen(buf);
	while (bufleft)
	{
		time(&current_time);
		if (current_time >= max_time)
		{
			sox_close(fd);
			return (0);
		}

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec=max_time-current_time;
		tv.tv_usec=0;
		if (sox_select(fd+1, 0, &fds, 0, &tv) != 1 ||
			!FD_ISSET(fd, &fds))
		{
			sox_close(fd);
			return (0);
		}
		n=sox_write(fd, bufptr, bufleft);
		if (n <= 0)
		{
			sox_close(fd);
			return (0);
		}
		bufptr += n;
		bufleft -= n;
	}

	bufptr=buf;
	bufleft=sizeof(buf);
	do
	{
		if (bufleft == 0)
		{
			sox_close(fd);
			return (0);
		}

		time(&current_time);
		if (current_time >= max_time)
		{
			sox_close(fd);
			return (0);
		}

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec=max_time-current_time;
		tv.tv_usec=0;
		if (sox_select(fd+1, &fds, 0, 0, &tv) != 1 ||
			!FD_ISSET(fd, &fds))
		{
			sox_close(fd);
			return (0);
		}

		n=sox_read(fd, bufptr, bufleft);
		if (n <= 0)
		{
			sox_close(fd);
			return (0);
		}
		bufptr += n;
		bufleft -= n;
	} while (bufptr[-1] != '\n');
	sox_close(fd);
	bufptr[-1]=0;
	--bufptr;
	if (bufptr > buf && bufptr[-1] == '\r')
		bufptr[-1]=0;

	if ((p=strchr(buf, ':')) == 0)
		return (0);

	q=++p;
	if ((p=strchr(p, ':')) == 0)
		return (0);

	*p++=0;
	q=strtok(q, " \t");
	if (!q || strcmp(q, "USERID"))	return (0);
	if (ostype)	*ostype=p;
	if ((p=strchr(p, ':')) == 0)
		return (0);
	*p++=0;
	while (*p && (*p == ' ' || *p == '\t'))	p++;
	return (p);
}

