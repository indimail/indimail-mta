/*
 * $Log: common.c,v $
 * Revision 1.6  2017-03-13 14:16:00+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.5  2015-12-29 00:05:52+05:30  Cprogrammer
 * conditional compilation #ifndef INDIMAIL
 *
 * Revision 1.4  2013-05-15 00:30:01+05:30  Cprogrammer
 * added timeoutio functions
 *
 * Revision 1.3  2013-05-15 00:18:11+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.2  2013-02-21 22:44:45+05:30  Cprogrammer
 * use AF_INET for localhost
 *
 * Revision 1.1  2013-02-11 22:07:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: common.c,v 1.6 2017-03-13 14:16:00+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifndef HAVE_INDIMAIL
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef sun
#include <sys/systeminfo.h>
#endif

#define FMT_ULONG 40
#define SOCKBUF                 32768 /*- Buffer size used in Monkey service -*/
#define MAX_BUFF                300
#define MAXSLEEP                0
#define MAXNOBUFRETRY           60 /*- Defines maximum number of ENOBUF retries -*/
#define SELECTTIMEOUT           30 /*- secs after which select will timeout -*/
#if !defined(INADDR_NONE) && defined(sun)
#define INADDR_NONE             0xffffffff /*- should be in <netinet/in.h> -*/
#endif

static unsigned sleeptime = MAXSLEEP + 1; /*- 0 for infinite connect */

#ifdef HAVE_STDARG_H
#include <stdarg.h>
/* function to write to a file */
int
filewrt(int fout, char *fmt, ...)
#else
#include <varargs.h>
int
filewrt(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif
#ifndef HAVE_STDARG_H
	int             fout;
	int             timeout;
	char           *fmt;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	fout = va_arg(ap, int);	/* file descriptor */
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	for (len = 0, ptr = buf; *ptr++; len++);
	return(write(fout, buf, len) != len ? -1 : len);
}

void
getEnvConfigInt(long *source, char *envname, long defaultValue)
{
	char           *value;

	if (!(value = getenv(envname)))
		*source = defaultValue;
	else
		*source = atol(value);
	return;
}

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

int
scopy(char *dest, const char *src, const int bound)
{
	register int    i;

	if (!src)
	{
		for(i = 0;i < bound;i++)
			dest[i] = 0;
		return 0;
	}
	for (i = 0; src[i] && (i < (bound - 1)); i++)
		dest[i] = src[i];
	dest[i] = 0;
	if(i == (bound - 1) && src[i])
		return(-1);
	return 0;
}

char    *
Dirname(char *path)
{
	static char     tmpbuf[MAX_BUFF];
	char           *ptr;

	if (!path || !*path)
		return ((char *) 0);
	scopy(tmpbuf, path, MAX_BUFF);
	if ((ptr = strrchr(tmpbuf, '/')) != (char *) 0)
	{
		if (ptr == tmpbuf)
			return ("/");
		*ptr = 0;
		return (tmpbuf);
	} 
	return ((char *) 0);
}

int
isnum(str)
	char           *str;
{
	char           *ptr;

	for (ptr = str; *ptr; ptr++)
		if (!isdigit((int) *ptr))
			return (0);
	return (1);
}

int
tcpopen(host, service, port) /*- Thanks to Richard's Steven */
	char           *host;
	char           *service;
	int             port;
/*-
 * host    - Name or dotted decimal address of the
 *           other system / Pathname for a unix domain socket
 * service - Name of the service being requested.
 *           Can be NULL, if port > 0.
 * port    - if == 0, nothin special, use port# of the service.
 *           if < 0, use a reserved port
 *           if > 0, it is the port# of server (host-byte-order)
 */
{
	int             resvport, fd = -1, optval, retval;
	char           *ptr, *hostptr;
#ifdef ENABLE_IPV6
	struct addrinfo hints, *res, *res0;
	char            serv[FMT_ULONG];
#else
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	struct servent *sp;
	struct hostent *hp;
	struct sockaddr_in tcp_srv_addr;/*- server's Internet socket address */
#endif
	struct sockaddr_un unixaddr;	/*- server's local unix socket address */
	struct linger   linger;
	char           *dir;
	char            localhost[MAXHOSTNAMELEN];

	if (host && *host && ((strchr(host, '/') || ((dir = Dirname(host)) && !access(dir, F_OK)))))
	{
		if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
			return -1;
		unixaddr.sun_family = AF_UNIX;
		scopy (unixaddr.sun_path, host, sizeof(unixaddr.sun_path));
		if (connect (fd, (struct sockaddr *) &unixaddr, sizeof(struct sockaddr_un) ) == -1)
			return -1;
		return(fd);
	}
#ifdef sun
	if (sysinfo(SI_HOSTNAME, localhost, MAXHOSTNAMELEN) > MAXHOSTNAMELEN)
#else
	if (gethostname(localhost, MAXHOSTNAMELEN))
#endif
		return (-1);
	if (!strcmp(host, localhost) || !strcmp(host, "localhost"))
		hostptr = "localhost";
	else
		hostptr = host;
	if ((ptr = (char *) getenv("SLEEPTIME")) != (char *) 0)
	{
		if (isnum(ptr))
			sleeptime = atoi(ptr);
		else
		{
			errno = EINVAL;
			return (-1);
		}
	}
#ifdef ENABLE_IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (service != (char *) NULL)
	{
		if (port > 0)
			snprintf(serv, FMT_ULONG, "%d", port);
	} else
	if (port <= 0)
	{
		errno = EINVAL;
		return (-1);
	} else
		snprintf(serv, FMT_ULONG, "%d", port);
	if ((retval = getaddrinfo(hostptr, serv, &hints, &res0)))
	{
		filewrt(2, "tcpopen: getaddrinfo: %s\n", gai_strerror(retval));
		return (-1);
	}
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next)
	{
		for (;;)
		{
			if (port >= 0)
			{
				if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
					break; /*- Try the next address record in the list. */
			}
#ifndef WindowsNT
			else /*- if (port < 0) */
			{
				resvport = IPPORT_RESERVED - 1;
				if ((fd = rresvport_af(&resvport, res->ai_family)) < 0) /*- RFC 2292 */
				{
					freeaddrinfo(res0);
					return (-1);
				}
			}
#endif /*- #ifndef WindowsNT */
			for (errno = 0;;)
			{
				if ((retval = connect(fd, res->ai_addr, res->ai_addrlen)) != -1)
					break;
				else
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					if (errno == ECONNREFUSED)
					{
						if (sleeptime <= MAXSLEEP)
						{
							if (sleeptime)
								(void) sleep(sleeptime);
							else
								(void) sleep(5);
							sleeptime += sleeptime;
							(void) close(fd);
							errno = ECONNREFUSED;
							break;
						} else
						{
							(void) close(fd);
							errno = ECONNREFUSED;
							freeaddrinfo(res0);
							return (-1);
						}
					}	/*- if (errno == ECONNREFUSED) */
					optval = errno;
					(void) close(fd);
					errno = optval;
					fd = -1;
					break;
				}
			} /*- for (errno = 0;;) */
			if (!retval || errno != ECONNREFUSED)
				break;
		} /*- for (;;) */
		/*- try the next address record in list */
	} /*- for (res = res0; res && fd == -1; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	/*
	 * Initialize the server's Internet address structure. We'll store
	 * the actual 4-byte Internet address and the 2-byte port # below.
	 */
	(void) memset((char *) &tcp_srv_addr, 0, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family = AF_INET;
	if (service != (char *) NULL)
	{
		if (port > 0)
			tcp_srv_addr.sin_port = htons(port);	/*- caller's value */
		else
		{
			if ((sp = getservbyname(service, "tcp")) == NULL)
			{
				errno = EINVAL;
				return (-1);
			}
			tcp_srv_addr.sin_port = sp->s_port;	/*- service's value */
		}
	} else
	if (port <= 0)
	{
		errno = EINVAL;
		return (-1);
	} else
		tcp_srv_addr.sin_port = htons(port);
	/*
	 * First try to convert the hostname as the dotted decimal number.
	 * Only if that fails, call gethostbyname.
	 */
	if ((inaddr = inet_addr(hostptr)) != INADDR_NONE)
	{			/*- It's a dotted decimal */
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
	} else
	if ((hp = gethostbyname(hostptr)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	} else
	{
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, hp->h_addr, hp->h_length);
	}
	for (;;)
	{
		if (port >= 0)
		{
			if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				return (-1);
		}
#ifndef WindowsNT
		else /*- if (port < 0) */
		{
			resvport = IPPORT_RESERVED - 1;
			if ((fd = rresvport(&resvport)) < 0)
				return (-1);
		}
#endif /*- #ifndef WindowsNT */
#if !defined(linux) && !defined(CYGWIN) && !defined(WindowsNT)
		if (!strcmp(hostptr, "localhost"))
		{
			optval = 1;
			if (setsockopt(fd, SOL_SOCKET, SO_USELOOPBACK, (char *) &optval, sizeof(optval)) == -1)
			{
				(void) close(fd);
				return (-1);
			}
		}
#endif
		/*- Connect to the server. */
		for (errno = 0;;)
		{
			if ((retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))) != -1)
				break;
			else
			{
#ifdef ERESTART
				if (errno == EINTR || errno == ERESTART)
#else
				if (errno == EINTR)
#endif
					continue;
				if (errno == ECONNREFUSED)
				{
					if (sleeptime <= MAXSLEEP)
					{
						if (sleeptime)
							(void) sleep(sleeptime);
						else
							(void) sleep(5);
						sleeptime += sleeptime;
						(void) close(fd);
						errno = ECONNREFUSED;
						break;
					} else
					{
						(void) close(fd);
						errno = ECONNREFUSED;
						return (-1);
					}
				}	/*- if (errno == ECONNREFUSED) */
				optval = errno;
				(void) close(fd);
				errno = optval;
				return (-1);
			}
		} /*- for (errno = 0;;) */
		if (!retval)
			break;
	} /*- for (;;) */
#endif /*- #ifdef ENABLE_IPV6 */
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)) == -1)
	{
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_SNDBUF, SOCKBUF) == -1)
	{
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_RCVBUF, SOCKBUF) == -1)
	{
		(void) close(fd);
		return (-1);
	}
	return (fd);
}	/*- end tcp_open */

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

int
tcpbind(hostname, servicename, backlog)
	char           *hostname, *servicename;
	int             backlog;
{
	int             listenfd;	/*- listen socket descriptor */
#ifdef ENABLE_IPV6
	struct addrinfo hints, *res, *res0;
#else
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	struct sockaddr_in localinaddr;	/*- for local socket address */
	struct servent *sp;	/*- pointer to service information */
	struct hostent *hp;
#endif /*- #ifdef ENABLE_IPV6 */
	struct sockaddr_un localunaddr;	/*- for local socket address */
	char           *dir;
	int             idx, socket_type;
	struct linger cflinger;

	if (!hostname)
		socket_type = AF_INET;
	else
	if ((dir = Dirname(hostname)) && !access(dir, F_OK))
		socket_type = AF_UNIX;
	else
		socket_type = AF_INET;
	if (socket_type == AF_UNIX)
	{
		(void) memset((char *) &localunaddr, 0, sizeof(struct sockaddr_un));
		localunaddr.sun_family = AF_UNIX;
		scopy(localunaddr.sun_path, hostname, sizeof(localunaddr.sun_path));
		if (!access(hostname, F_OK) && unlink(hostname))
		{
			errno = EEXIST;
			return(-1);
		}
		if ((listenfd = socket(socket_type, SOCK_STREAM, 0)) == -1)
			return (-1);
		idx = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof(idx)) < 0)
		{
			(void) close(listenfd);
			return (-1);
		}
		if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
		{
			(void) close(listenfd);
			return (-1);
		}
		if (bind(listenfd, (struct sockaddr *) &localunaddr, idx) == -1)
		{
			(void) close(listenfd);
			return (-1);
		}
	}
	/*- Set up address structure for the listen socket. */
	cflinger.l_onoff = 1;
	cflinger.l_linger = 60;
#ifdef ENABLE_IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((idx = getaddrinfo(NULL, servicename, &hints, &res0)))
	{
		filewrt(2, "tcpbind: getaddrinfo: %s\n", gai_strerror(idx));
		return (-1);
	}
	listenfd = -1;
	for (res = res0; res; res = res->ai_next) {
		if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		{
			freeaddrinfo(res0);
			return (-1);
		}
		idx = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof (int)) == -1)
		{
			close(listenfd);
			freeaddrinfo(res0);
			return (-1);
		}
		if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
		{
			close(listenfd);
			freeaddrinfo(res0);
			return (-1);
		}
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;
		close(listenfd);
	} /*- for (res = res0; res; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	(void) memset((char *) &localinaddr, 0, sizeof(struct sockaddr_in));
	localinaddr.sin_family = AF_INET;
	/*- It's a dotted decimal */
	if (!hostname)
		hostname = "127.0.0.1";
	if ((inaddr = inet_addr(hostname)) != INADDR_NONE)
		localinaddr.sin_addr.s_addr = inaddr;
	else
	if ((hp = gethostbyname(hostname)) != NULL)
		(void) memcpy((char *) &localinaddr.sin_addr, hp->h_addr, hp->h_length);
	else
	{
		(void) filewrt(2, "gethostbyname: %s: No such Host\n", hostname);
		errno = EINVAL;
		return (-1);
	}
	if (isnum(servicename))
		localinaddr.sin_port = htons(atoi(servicename));
	else
	if ((sp = getservbyname(servicename, "tcp")) != (struct servent *) 0)
		localinaddr.sin_port = sp->s_port;
	else
		return (-1);
	if ((listenfd = socket(socket_type, SOCK_STREAM, 0)) == -1)
		return (-1);
	idx = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &idx, sizeof(idx)) < 0)
	{
		(void) close(listenfd);
		return (-1);
	}
	if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
	if (bind(listenfd, (struct sockaddr *) &localinaddr, sizeof(localinaddr)) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
#endif
	if (listen(listenfd, backlog) == -1)
	{
		(void) close(listenfd);
		return (-1);
	}
	return (listenfd);
}

int
slen(const char *dest)
{
	register int    i;

	if (!dest)
		return (0);
	for (i = 0; dest[i]; i++);
	return (i);

}

int
r_mkdir(dir, perm, uid, gid)
	char           *dir;
	mode_t          perm;
	uid_t           uid;
	gid_t           gid;
{
	char           *ptr, *cptr, *path;
	int             len;

	if(!access(dir, F_OK))
		return(0);
	len = slen(dir);
	if(!(path = (char *) malloc(len + 1)))
		return(1);
	(void) scopy(path, dir, len + 1);
	if(*path == '/')
		ptr = path + 1;
	else
		ptr = path;
	for(;*ptr && (cptr = strchr(ptr, '/'));)
	{
		ptr = cptr + 1;
		if(!cptr || !*ptr)
			break;
		*cptr = 0;
		if (access(path, F_OK) && (mkdir(path, perm) || chown(path, uid, gid) || chmod(path, perm)))
		{
			*cptr = '/';
			free(path);
			return(1);
		}
		*cptr = '/';
	}
	(void) free(path);
	if (access(dir, F_OK) && (mkdir(dir, perm) || chown(dir, uid, gid) || chmod(dir, perm)))
		return(1);
	return(0);
}

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
#endif

void
getversion_common_c()
{
	(void) filewrt(1, "%s\n", sccsid);
}
