/*
 * $Log: tcpopen.c,v $
 * Revision 1.2  2015-08-20 18:56:55+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.1  2015-08-20 18:36:55+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef QMAIL_INTERNAL
#include "hasindimail.h"
#endif

#ifndef INDIMAIL
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "fmt.h"
#include "byte.h"
#include "subfd.h"
#include "strerr.h"
#ifdef sun
#include <sys/systeminfo.h>
#endif

#define MAXSLEEP                0
#define MAX_BUFF                300
#define SOCKBUF                 32768 /*- Buffer size used in Monkey service -*/
#define MAXNOBUFRETRY           60 /*- Defines maximum number of ENOBUF retries -*/
static unsigned sleeptime = MAXSLEEP + 1; /*- 0 for infinite connect */

static int
isnum(str)
	char           *str;
{
	char           *ptr;

	for (ptr = str; *ptr; ptr++)
		if (!isdigit((int) *ptr))
			return (0);
	return (1);
}

static char    *
Dirname(char *path)
{
	static char     tmpbuf[MAX_BUFF];
	char           *ptr;

	if (!path || !*path)
		return ((char *) 0);
	byte_copy(tmpbuf, MAX_BUFF, path);
	if ((ptr = strrchr(tmpbuf, '/')) != (char *) 0)
	{
		if (ptr == tmpbuf)
			return ("/");
		*ptr = 0;
		return (tmpbuf);
	} 
	return ((char *) 0);
}

static int
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
	struct servent *sp;
#ifdef IPV6
	struct addrinfo hints, *res, *res0;
	char            serv[FMT_ULONG];
#else
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
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
    	byte_copy (unixaddr.sun_path, sizeof(unixaddr.sun_path), host);
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
#ifdef IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (service != (char *) NULL)
	{
		if (port > 0)
			serv[fmt_ulong(serv, htons(port))] = 0;
		else
		{
			if (isnum(service))
				byte_copy(serv, FMT_ULONG, service);
			else {
				if ((sp = getservbyname(service, "tcp")) == NULL)
				{
					errno = EINVAL;
					return (-1);
				}
				serv[fmt_ulong(serv, htons(sp->s_port))] = 0;
			}
		}
	} else
	if (port <= 0)
	{
		errno = EINVAL;
		return (-1);
	} else
		serv[fmt_ulong(serv, htons(port))] = 0;
	if ((retval = getaddrinfo(hostptr, serv, &hints, &res0)))
	{
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, "getaddrinfo: ") == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, hostptr) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, ": ") == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, (char *) gai_strerror(retval)) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_put(subfderr, "\n", 1) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_flush(subfderr) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
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
			if (isnum(service))
				tcp_srv_addr.sin_port = htons(atoi(service));
			else {
				if ((sp = getservbyname(service, "tcp")) == NULL)
				{
					errno = EINVAL;
					return (-1);
				} 
				tcp_srv_addr.sin_port = htons(sp->s_port);	/*- service's value */
			}
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
		(void) byte_copy((char *) &tcp_srv_addr.sin_addr, sizeof(inaddr), (char *) &inaddr);
	} else
	if ((hp = gethostbyname(hostptr)) == NULL)
	{
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, "gethostbyname: ") == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, hostptr) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, ": No such host\n") == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_flush(subfderr) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		errno = EINVAL;
		return (-1);
	} else
		(void) byte_copy((char *) &tcp_srv_addr.sin_addr, hp->h_length, hp->h_addr);
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
#endif /*- #ifdef IPV6 */
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
#endif

void
getversion_tcpopen_c()
{
	static char    *x = "$Id: tcpopen.c,v 1.2 2015-08-20 18:56:55+05:30 Cprogrammer Exp mbhangui $";
	char *y = sccsid;
#ifdef INDIMAIL
	if (y)
		y = sccsidh;
#else
	if (y)
		y++;
#endif
}
