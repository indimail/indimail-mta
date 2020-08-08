/*
 * $Log: tcpopen.c,v $
 * Revision 1.7  2020-08-08 10:54:03+05:30  Cprogrammer
 * added missing return on connect() failure
 *
 * Revision 1.6  2020-07-03 22:21:10+05:30  Cprogrammer
 * fixed SIGSEGV if a readonly string was used for host
 *
 * Revision 1.4  2018-05-29 21:47:11+05:30  Cprogrammer
 * removed call to gethostbyname() in ipv6 code
 *
 * Revision 1.3  2018-01-09 12:37:04+05:30  Cprogrammer
 * removed header hasindimail.h
 *
 * Revision 1.2  2016-06-13 14:17:37+05:30  Cprogrammer
 * close socket on connect() failure
 *
 * Revision 1.1  2015-08-20 19:03:05+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "haveip6.h"
#include "fmt.h"
#include "env.h"
#include "scan.h"
#include "str.h"
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

static int
setsockbuf(fd, option, size)
	int             fd, option, size;
{
	int             len, retrycount;

	len = size;
	for (retrycount = 0; retrycount < MAXNOBUFRETRY; retrycount++) {
		if (setsockopt(fd, SOL_SOCKET, option, (void *) &len, sizeof(int)) == -1) {
			if (errno == ENOBUFS) {
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
 *           it could be a service in /etc/services if port < 0
 * port    - if == 0, nothin special, use port# of the service.
 *           if < 0, use a reserved port
 *           if > 0, it is the port# of server (host-byte-order)
 */
{
	int             resvport, fd = -1, optval, retval, i;
	char           *ptr, *hostptr;
	struct servent *sp;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	struct addrinfo hints = {0}, *res = 0, *res0 = 0;
	char            serv[FMT_ULONG];
#else
	struct hostent *hp;
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	struct sockaddr_in tcp_srv_addr;/*- server's Internet socket address */
#endif
	struct sockaddr_un unixaddr;	/*- server's local unix socket address */
	struct linger   linger;
	char            localhost[MAXHOSTNAMELEN];

	i = str_chr(host, '/');
	if (host && *host && host[i] && !access(host, F_OK)) {
		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        	return -1;
    	unixaddr.sun_family = AF_UNIX;
    	str_copyb(unixaddr.sun_path, host, sizeof(unixaddr.sun_path));
    	if (connect(fd, (struct sockaddr *) &unixaddr, sizeof(struct sockaddr_un)) == -1) {
			close (fd);
        	return -1;
		}
		return(fd);
	}
#ifdef sun
	if (sysinfo(SI_HOSTNAME, localhost, MAXHOSTNAMELEN) > MAXHOSTNAMELEN)
#else
	if (gethostname(localhost, MAXHOSTNAMELEN))
#endif
		return (-1);
	if (!str_diff(host, localhost) || !str_diffn(host, "localhost", 10))
		hostptr = "localhost";
	else
		hostptr = host;
	if ((ptr = (char *) env_get("SLEEPTIME")) != (char *) 0) {
		if (isnum(ptr))
			scan_uint(ptr, &sleeptime);
		else {
			errno = EINVAL;
			return (-1);
		}
	}
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (service != (char *) NULL) {
		if (port > 0)
			serv[fmt_ulong(serv, port)] = 0;
		else {
			if (isnum(service))
				str_copyb(serv, service, FMT_ULONG);
			else {
				if ((sp = getservbyname(service, "tcp")) == NULL) {
					errno = EINVAL;
					return (-1);
				}
				serv[fmt_ulong(serv, htons(sp->s_port))] = 0;
			}
		}
	} else
	if (port <= 0) {
		errno = EINVAL;
		return (-1);
	} else
		serv[fmt_ulong(serv, port)] = 0;
	if ((retval = getaddrinfo(hostptr, serv, &hints, &res0)))
		strerr_die7x(111, "tcpopen: ", "getaddrinfo: ", hostptr, ": ", serv, ":", (char *) gai_strerror(retval));
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next) {
		for (;;) {
			if (port >= 0) {
				if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
					break; /*- Try the next address record in the list. */
			} else { /*- if (port < 0) */
				resvport = IPPORT_RESERVED - 1;
				if ((fd = rresvport_af(&resvport, res->ai_family)) < 0) /*- RFC 2292 */ {
					freeaddrinfo(res0);
					return (-1);
				}
			}
			for (errno = 0;;) {
				if (!(retval = connect(fd, res->ai_addr, res->ai_addrlen)))
					break;
				else {
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					if (errno == ECONNREFUSED) {
						if (sleeptime <= MAXSLEEP) {
							(void) close(fd);
							if (sleeptime)
								(void) sleep(sleeptime);
							else
								(void) sleep(5);
							sleeptime += sleeptime;
							errno = ECONNREFUSED;
							break;
						} else {
							(void) close(fd);
							freeaddrinfo(res0);
							errno = ECONNREFUSED;
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
	 	if (!retval || errno != ECONNREFUSED)
	 		break;
		/*- try the next address record in list */
	} /*- for (res = res0; res && fd == -1; res = res->ai_next) */
	freeaddrinfo(res0);
#else /*- #if defined(LIBC_HAS_IP6) && defined(IPV6) */
	/*
	 * Initialize the server's Internet address structure. We'll store
	 * the actual 4-byte Internet address and the 2-byte port # below.
	 */
	byte_zero((char *) &tcp_srv_addr, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family = AF_INET;
	if (service != (char *) NULL) {
		if (port > 0)
			tcp_srv_addr.sin_port = htons(port);	/*- caller's value */
		else {
			if (isnum(service)) {
				scan_int(service, &i);
				tcp_srv_addr.sin_port = htons(i);
			} else {
				if ((sp = getservbyname(service, "tcp")) == NULL) {
					errno = EINVAL;
					return (-1);
				} 
				tcp_srv_addr.sin_port = htons(sp->s_port);	/*- service's value */
			}
		}
	} else
	if (port <= 0) {
		errno = EINVAL;
		return (-1);
	} else
		tcp_srv_addr.sin_port = htons(port);
	/*
	 * First try to convert the hostname as the dotted decimal number.
	 * Only if that fails, call gethostbyname.
	 */
	if ((inaddr = inet_addr(hostptr)) != INADDR_NONE) /*- It's a dotted decimal */
		(void) byte_copy((char *) &tcp_srv_addr.sin_addr, sizeof(inaddr), (char *) &inaddr);
	else
	if ((hp = gethostbyname(hostptr)) == NULL)
		strerr_die5x(111, "tcpopen", "gethostbyname: ", hostptr, ": ", (char *) hstrerror(h_errno));
	else
		(void) byte_copy((char *) &tcp_srv_addr.sin_addr, hp->h_length, hp->h_addr);
	for (;;) {
		if (port >= 0) {
			if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				return (-1);
		} else { /*- if (port < 0) */
			resvport = IPPORT_RESERVED - 1;
			if ((fd = rresvport(&resvport)) < 0)
				return (-1);
		}
#if !defined(linux) && !defined(CYGWIN) && !defined(WindowsNT)
		if (!str_diffn(hostptr, "localhost", 10)) {
			optval = 1;
			if (setsockopt(fd, SOL_SOCKET, SO_USELOOPBACK, (char *) &optval, sizeof(optval)) == -1) {
				(void) close(fd);
				return (-1);
			}
		}
#endif
		/*- Connect to the server. */
		for (errno = 0;;) {
			if (!(retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))))
				break;
			else {
#ifdef ERESTART
				if (errno == EINTR || errno == ERESTART)
#else
				if (errno == EINTR)
#endif
					continue;
				if (errno == ECONNREFUSED) {
					if (sleeptime <= MAXSLEEP) {
						if (sleeptime)
							(void) sleep(sleeptime);
						else
							(void) sleep(5);
						sleeptime += sleeptime;
						(void) close(fd);
						errno = ECONNREFUSED;
						break;
					} else {
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
#endif /*- #if defined(LIBC_HAS_IP6) && defined(IPV6) */
	if (!fd)
		return (-1);
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)) == -1) {
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_SNDBUF, SOCKBUF) == -1) {
		(void) close(fd);
		return (-1);
	} else
	if (setsockbuf(fd, SO_RCVBUF, SOCKBUF) == -1) {
		(void) close(fd);
		return (-1);
	}
	return (fd);
}	/*- end tcp_open */

void
getversion_tcpopen_c()
{
	static char    *x = "$Id: tcpopen.c,v 1.7 2020-08-08 10:54:03+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
