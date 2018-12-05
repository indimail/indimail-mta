/*
** Copyright 2004-2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"mksocket.h"

#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<stdlib.h>
#include	"soxwrap.h"


#define MKS_USEAFINET4 1
#define MKS_ERROK 2

#if	HAVE_SOXWRAP_IPV6
typedef struct in6_addr	NET_ADDR;
typedef struct sockaddr_in6 NET_SOCKADDR;
typedef struct sockaddr_storage NET_NETADDR;
#define NET_ADDRANY		in6addr_any
#else
typedef	struct in_addr	NET_ADDR;
typedef struct sockaddr_in	NET_SOCKADDR;
typedef struct sockaddr		NET_NETADDR;

extern struct in_addr rfc1035_addr_any;
#define NET_ADDRANY		rfc1035_addr_any
#endif

struct recycle_info {
	const struct sockaddr *sin;
	socklen_t sin_len;
	int found_socket;
};

/*
** If caller already has a socket listening on this address, recycle it.
*/

static int try_recycle_socket(int fd, void *voidarg)
{
	struct recycle_info *ri=(struct recycle_info *)voidarg;

	union {
		SOCKADDR_STORAGE ss;
		struct sockaddr_in sin;
#if	HAVE_SOXWRAP_IPV6
		struct sockaddr_in6 sin6;
#endif
	} sa;

	socklen_t salen=sizeof(sa);

	if (getsockname(fd, (struct sockaddr *)&sa, &salen) < 0)
		return 0;

	if (ri->sin->sa_family != sa.ss.SS_family)
		return 0;

	switch (ri->sin->sa_family) {
	case AF_INET:
		{
			struct sockaddr_in ri_sin;

			memcpy(&ri_sin, ri->sin, sizeof(ri_sin));

			if (ri_sin.sin_addr.s_addr != sa.sin.sin_addr.s_addr
			    || ri_sin.sin_port != sa.sin.sin_port)
				return 0;
		}
		break;

#if	HAVE_SOXWRAP_IPV6
	case AF_INET6:
		{
			struct sockaddr_in6 ri_sin;

			memcpy(&ri_sin, ri->sin, sizeof(ri_sin));

			if (memcmp(&ri_sin.sin6_addr, &sa.sin6.sin6_addr,
				   sizeof(sa.sin6.sin6_addr)) ||
			    ri_sin.sin6_port != sa.sin6.sin6_port)
				return 0;
		}
		break;
#endif
	default:
		return 0;
	}
	ri->found_socket=fd;
	return 1;
}

static int mksocket2(const char *ipaddrarg,	/* Host/IP address */
		     const char *servname,	/* Service/port */
		     int socktype,	/* SOCKS_STREAM */
		     int flags,
		     int (*recycle_fd_func)
		     ( int(*)(int, void *), void *, void *),
		     void *voidarg)
{
	struct servent *servptr;
	int	port;
	int	fd;
	struct recycle_info ri;
	const struct sockaddr *sinaddr;
	int	sinaddrlen;

#if	HAVE_SOXWRAP_IPV6
	struct sockaddr_in6	sin6;
#endif

	struct sockaddr_in	sin4;

	int	af;

	servptr=getservbyname(servname, "tcp");
	if (servptr)
		port=servptr->s_port;
	else
	{
		port=atoi(servname);
		if (port <= 0 || port > 65535)
		{
			fprintf(stderr, "Invalid port: %s\n", servname);
			return (-1);
		}
		port=htons(port);
	}

	/* Create an IPv6 or an IPv4 socket */

#if	HAVE_SOXWRAP_IPV6
	if (flags & MKS_USEAFINET4)
	{
		fd=sox_socket(PF_INET, socktype, 0);
		af=AF_INET;
	}
	else
#endif
	{
#if	HAVE_SOXWRAP_IPV6

		if ((fd=sox_socket(PF_INET6, socktype, 0)) >= 0)
			af=AF_INET6;
		else
#endif
		{
			af=AF_INET;
			fd=sox_socket(PF_INET, socktype, 0);
		}
	}

	if (fd < 0)
		return (-1);

	/* Figure out what to bind based on what socket we created */

	if (ipaddrarg && strcmp(ipaddrarg, "0"))
	{

#if	HAVE_SOXWRAP_IPV6
		if (af == AF_INET6)
		{
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family=af;
			if (inet_pton(af, ipaddrarg, &sin6.sin6_addr) <= 0)
			{
				errno=EINVAL;
				close(fd);
				return -1;
			}
			sin6.sin6_port=port;

			sinaddr=(const struct sockaddr *)&sin6;
			sinaddrlen=sizeof(sin6);
		}
		else
#endif
			if (af == AF_INET)
			{
				memset(&sin4, 0, sizeof(sin4));
				sin4.sin_family=AF_INET;
				if (inet_aton(ipaddrarg, &sin4.sin_addr) == 0)
				{
					errno=EINVAL;
					close(fd);
					return -1;
				}
				sin4.sin_port=port;
				sinaddr=(const struct sockaddr *)&sin4;
				sinaddrlen=sizeof(sin4);
			}
			else
			{
				errno=EAFNOSUPPORT;
				close(fd);
				return (-1);
			}
	}
	else	/* Bind default address */
	{
#if	HAVE_SOXWRAP_IPV6
		if (af == AF_INET6)
		{
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family=AF_INET6;
			sin6.sin6_addr=in6addr_any;
			sin6.sin6_port=port;
			sinaddr=(const struct sockaddr *)&sin6;
			sinaddrlen=sizeof(sin6);
		}
		else
#endif
			if (af == AF_INET)
			{
				sin4.sin_family=AF_INET;
				sin4.sin_addr.s_addr=INADDR_ANY;
				sin4.sin_port=port;
				sinaddr=(const struct sockaddr *)&sin4;
				sinaddrlen=sizeof(sin4);
			}
			else
			{
				errno=EAFNOSUPPORT;
				close(fd);
				return (-1);
			}
	}

	ri.found_socket= -1;
	ri.sin=sinaddr;
	ri.sin_len=sinaddrlen;

	if (recycle_fd_func &&
	    (*recycle_fd_func)(&try_recycle_socket, &ri, voidarg) > 0 &&
	    ri.found_socket >= 0)
	{
		close(fd);
		return dup(ri.found_socket);
	}

	{
		int dummy=1;

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			   (const char *)&dummy, sizeof(dummy));
	}

	if (fcntl(fd, F_SETFD, FD_CLOEXEC))
	{
		close(fd);
		return (-1);
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK))
	{
		close(fd);
		return (-1);
	}
	{
		int dummy=1;

		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			   (const char *)&dummy, sizeof(dummy));
	}

	if (sox_bind(fd, sinaddr, sinaddrlen) < 0)
	{
		if (flags & MKS_ERROK)
		{
			close(fd);
			return (-2);
		}
		close(fd);
		return (-1);
	}

	if (sox_listen(fd,
#ifdef	SOMAXCONN
		       SOMAXCONN
#else
		       5
#endif
		       ))
	{
		if (flags && MKS_ERROK)
		{
			close(fd);
			return (-2);
		}
		close(fd);
		return (-1);
	}
	return (fd);
}


int mksocket(const char *address,
	     const char *service,
	     int socktype,
	     int *fd1,
	     int *fd2,
	     int recycle_fd_func( int(*)(int, void *), void *, void *),
	     void *voidarg)
{
	int fd_flag=0;

	*fd1= -1;
	*fd2= -1;

	if (address && strcmp(address, "0"))
	{
#if HAVE_INET_PTON
		SOCKADDR_STORAGE ina;

		if (inet_pton(AF_INET, address, &ina) > 0)
			fd_flag=MKS_USEAFINET4;
#else
		struct in_addr ina;

		if (inet_aton(address, &ina))
			fd_flag=MKS_USEAFINET4;
#endif

	}


	*fd1=mksocket2(address, service, socktype, fd_flag,
		       recycle_fd_func, voidarg);

	/* BSD requires both an IPv6 and an IPv4 socket */

#if	HAVE_SOXWRAP_IPV6

	if (address == 0 || strcmp(address, "0") == 0)
	{
		int fd=mksocket2(address, service, socktype,
				 (MKS_USEAFINET4|MKS_ERROK),
				 recycle_fd_func, voidarg);

		if (fd < 0)
		{
			if (fd != -2)
			{
				if (*fd1 >= 0)
					close(*fd1);
				return -1;
			}
		}

		*fd2=fd;
	}
#endif
	if (*fd1 < 0 && *fd2 < 0)
		return -1;

	if (*fd1 < 0)
	{
		*fd1=*fd2;
		*fd2=-1;
	}
	return 0;
}

#if HAVE_SYS_POLL_H

#else

int poll(struct pollfd *pfd, unsigned int n, int timeout)
{
	fd_set r, w, e;
	int maxfd=-1;
	unsigned int i;
	struct timeval tv;
	int cnt=0;

	FD_ZERO(&r);
	FD_ZERO(&w);
	FD_ZERO(&e);

	for (i=0; i<n; i++)
	{
		if (pfd[i].fd >= maxfd)
			maxfd=pfd[i].fd;

		pfd[i].revents=0;
		if (pfd[i].events & (POLLIN|POLLPRI))
			FD_SET(pfd[i].fd, &r);
		if (pfd[i].events & POLLOUT)
			FD_SET(pfd[i].fd, &w);
		if (pfd[i].events & POLLPRI)
			FD_SET(pfd[i].fd, &e);
	}

	tv.tv_sec=timeout/1000;
	tv.tv_usec=(timeout % 1000) * 1000;

	if (select(maxfd+1, &r, &w, &e, timeout < 0 ?NULL:&tv) < 0)
		return -1;

	for (i=0; i<n; i++)
	{
		if (FD_ISSET(pfd[i].fd, &r))
			pfd[i].revents |= POLLIN;
		if (FD_ISSET(pfd[i].fd, &w))
			pfd[i].revents |= POLLOUT;
		if (FD_ISSET(pfd[i].fd, &e))
			pfd[i].revents |= POLLIN|POLLHUP;

		if (pfd[i].revents)
			++cnt;
	}

	return cnt;
}

#endif
