/*
 * $Log: udplogger.c,v $
 * Revision 1.9  2024-06-16 21:28:19+05:30  Cprogrammer
 * set n=MAXLOGDATASIZE as default
 *
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-01-04 00:15:14+05:30  Cprogrammer
 * removed __USE_GNU
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2020-09-16 19:09:07+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.4  2020-07-04 21:43:46+05:30  Cprogrammer
 * removed usage of INET6 define
 *
 * Revision 1.3  2018-05-29 22:12:09+05:30  Cprogrammer
 * removed call to gethostbyname() in ipv6 code
 *
 * Revision 1.2  2016-04-15 15:42:09+05:30  Cprogrammer
 * ipv6 version
 *
 * Revision 1.1  2015-04-10 19:37:08+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sgetopt.h>
#include <subfd.h>
#include <strerr.h>
#include <sig.h>
#include <fmt.h>
#include <scan.h>
#include <byte.h>
#include <error.h>
#include <noreturn.h>
#include "haveip6.h"

#define DYNAMIC_BUF 1
#define MAXLOGDATASIZE 2000
#define DEFAULTLOGPORT 2999
#define DEFAULTLOGIP   "127.0.0.1"
#define LOGTIMEOUT     3

#define FATAL "udplogger: fatal: "
#define WARN  "udplogger: warn: "

union sockunion
{
	struct sockaddr     sa;
	struct sockaddr_in  sa4;
#ifdef IPV6
	struct sockaddr_in6 sa6;
#endif
};
static unsigned long seqno;

no_return void
die_nomem()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_puts(subfderr, FATAL) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_puts(subfderr, "out of memory\n") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	_exit(1);
}

no_return void
die_write()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_puts(subfderr, FATAL) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_puts(subfderr, "unable to write\n") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	_exit(1);
}

void
out(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerr(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerrf(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, FATAL, "write: ");
}

void
sighup(int x)
{
	sig_block(SIGHUP);
	logerrf("Received SIGHUP\n");
	seqno = 1;
	sig_unblock(SIGHUP);
}

void
sigusr1(int x)
{
	sig_block(SIGUSR1);
	logerrf("Received SIGUSR1\n");
	sig_unblock(SIGUSR1);
	return;
}

no_return void
sigterm(int x)
{
	sig_block(SIGTERM);
	logerrf("ARGH!! Committing suicide on SIGTERM\n");
	flush();
	_exit(0);
}

int
main(int argc, char **argv)
{
	int             s, buf_len, rdata_len, n, port, opt, fromlen;
	char            strnum[FMT_ULONG];
	unsigned long   timeout;
	char           *ipaddr = 0;
	const char     *usage = "usage: udplogger [-t timeout] [-p port] ipaddr:port";
	char            serv[FMT_ULONG];
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	int             noipv6 = 0;
#else
	int             noipv6 = 1;
#endif
#ifdef DYNAMIC_BUF
	char           *rdata = 0, *buf = 0;
	int             bufsize = MAXLOGDATASIZE;
#else
	char            rdata[MAXLOGDATASIZE];
#endif
	union sockunion sin, from;
#if defined(LIBC_HAS_IP6)
	struct addrinfo hints = {0}, *res = 0, *res0 = 0;
#else
	struct hostent *hp;
#endif
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
	fd_set          rfds;
	struct timeval  tv;

	/*- defaults */
	timeout = 5 * 60;    /*- 5 mins */
	while ((opt = getopt(argc, argv, "t:p:")) != opteof) {
		switch (opt) {
		case 't':
			scan_ulong(optarg, &timeout);
			timeout *= 60;
			break;
		case 'p':
			scan_int(optarg, &port);
			byte_copy(serv, FMT_ULONG, optarg);
			break;
		default:
			strerr_die1x(100, usage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	ipaddr = argv[optind++];
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY EAI_FAMILY
#endif
	if ((opt = getaddrinfo(ipaddr, serv, &hints, &res0))) {
		if (opt == EAI_ADDRFAMILY || (opt == EAI_SYSTEM && errno == EAFNOSUPPORT))
			noipv6 = 1;
		else
			strerr_die7x(111, FATAL, "getadrinfo: ", ipaddr, ": ", serv, ":", (char *) gai_strerror(opt));
	}
	close (0);
	if (!noipv6) {
		for (res = res0; res; res = res->ai_next) {
			if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
				if (errno == EINVAL || errno == EAFNOSUPPORT) {
					noipv6 = 1;
					break;
				}
				freeaddrinfo(res0);
				strerr_die2sys(111, FATAL, "unable to create socket6: ");
			}
			if (dup2(s, 0)) {
				freeaddrinfo(res0);
				strerr_die2sys(111, FATAL, "unable to dup socket: ");
			}
			if (s) {
				close(s);
				s = 0;
			}
			if (bind(s, res->ai_addr, res->ai_addrlen) == 0)
				break;
			freeaddrinfo(res0);
			strerr_die6sys(111, FATAL, "bind6: ", ipaddr, ":", serv, ": ");
		} /*- for (res = res0; res; res = res->ai_next) */
	}
	freeaddrinfo(res0);
#else
	noipv6 = 1;
#endif
	if (noipv6) {
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			strerr_die2sys(111, FATAL, "unable to create socket4: ");
		if (dup2(s, 0))
			strerr_die2sys(111, FATAL, "unable to dup socket: ");
		if (s) {
			close(s);
			s = 0;
		}
		port = atoi(serv);
		sin.sa4.sin_port = htons(port);
#if defined(LIBC_HAS_IP6)
		if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE) {
			byte_copy((char *) &sin.sa4.sin_addr, 4, (char *) &inaddr);
			if (bind(s, (struct sockaddr *) &sin.sa4, sizeof(sin)) == -1)
				strerr_die6sys(111, FATAL, "bind4: ", ipaddr, ":", serv, ": ");
		} else {
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_DGRAM;
			if ((opt = getaddrinfo(ipaddr, serv, &hints, &res0))) {
				freeaddrinfo(res0);
				errno = EINVAL;
				strerr_die7x(111, FATAL, "getadrinfo: ", ipaddr, ": ", serv, ":", (char *) gai_strerror(opt));
			}
			for (res = res0; res; res = res->ai_next) {
				if (bind(s, res->ai_addr, res->ai_addrlen) == 0)
					break;
			}
			freeaddrinfo(res0);
		}
#else
		if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE)
			byte_copy((char *) &sin.sa4.sin_addr, 4, (char *) &inaddr);
		else {
			if (!(hp = gethostbyname(ipaddr)))
				strerr_die5x(111, FATAL, "gethostbyname: ", ipaddr, ": ", (char *) hstrerror(h_errno));
			byte_copy((char *) &sin.sa4.sin_addr, hp->h_length, hp->h_addr);
		}
		if (bind(s, (struct sockaddr *) &sin.sa4, sizeof(sin)) == -1)
			strerr_die6sys(111, FATAL, "bind4: ", ipaddr, ":", serv, ": ");
#endif
	}
	sig_catch(SIGTERM, sigterm);
	sig_catch(SIGHUP, sighup);
	sig_catch(SIGUSR1, sigusr1);
	out("Ready for connections\n");
	flush();
	for (seqno = 1, buf_len = 0, rdata_len = 0;;) {
		int             ret;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		if ((ret = select(1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &tv)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_die2sys(111, FATAL, "select: ");
		} else
		if (!ret) /*- timeout occurred */
			continue;
		if (!FD_ISSET(0, &rfds))
			continue;
		/*
		 * Keep on incrementing bufsize till it is
		 * possible to fetch the entire message
		 * in one operation. This will allow the
		 * client to send the entire rcpt list in
		 * one operation.
		 *
		 * buf_len < bufsize - increase buffer size if packet cannot be read in
		 *                     one operation
		 * buf_len / bufsize - decrease buffer size if allocated size is more than 2
		 *                     times of required size
		 */
#ifdef DYNAMIC_BUF
		for (;;) {
			if ((buf_len < bufsize || buf_len/bufsize > 2) && !(buf = (char *) realloc(buf, (buf_len = bufsize))))
				die_nomem();
			if ((n = recvfrom(0, buf, bufsize, MSG_PEEK, 0, 0)) == -1) {
				if (errno == error_intr)
					continue;
				strerr_die2sys(111, FATAL, "recvfrom: MSG_PEEK: ");
			}
			if (n == bufsize)
				bufsize *= 2;
			else
				break;
		}
		if (!n)
			n = MAXLOGDATASIZE;
		if ((rdata_len < n || rdata_len/n > 2) && !(rdata = (char *) realloc(rdata, (rdata_len = n))))
			die_nomem();
#else
		n = MAXLOGDATASIZE;
#endif
		fromlen = sizeof(from);
		if ((n = recvfrom(0, rdata, n, 0, (struct sockaddr *) &from.sa6, (socklen_t *)&fromlen)) == -1) {
			if (errno == error_intr)
				continue;
			strerr_die2sys(111, FATAL, "recvfrom: ");
		}
#if defined(LIBC_HAS_IP6) && defined(IPV6)
		if (noipv6)
			out(inet_ntoa(from.sa4.sin_addr));
		else {
			static char     addrBuf[INET6_ADDRSTRLEN];
			if (from.sa.sa_family == AF_INET) {
				out((char *) inet_ntop(AF_INET,
					(void *) &from.sa4.sin_addr, addrBuf,
					INET_ADDRSTRLEN));
			} else
			if (from.sa.sa_family == AF_INET6) {
				out((char *) inet_ntop(AF_INET6,
					(void *) &from.sa6.sin6_addr, addrBuf,
					INET6_ADDRSTRLEN));
			} else
			if (from.sa.sa_family == AF_UNSPEC)
				out("::1");
		}
#else
		out(inet_ntoa(from.sa4.sin_addr));
#endif
		out(" ");
		if (rdata[n - 1] == '\n')
			n--;
		strnum[buf_len = fmt_ulong(strnum, seqno++)] = 0;
		if (substdio_put(subfdout, strnum, buf_len) == -1)
			die_write();
		out(" ");
		if (substdio_put(subfdout, rdata, n) == -1)
			die_write();
		out("\n");
		flush();
	} /*- for (i = 0, j = 0;;) */
	return (0);
}

void
getversion_udplogger_c()
{
	const char     *x = "$Id: udplogger.c,v 1.9 2024-06-16 21:28:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
