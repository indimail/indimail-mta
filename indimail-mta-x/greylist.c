/*
 * $Id: greylist.c,v 1.16 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include "haveip6.h"
#include "stralloc.h"
#include "ip.h"
#include "fmt.h"
#include "byte.h"
#include "scan.h"
#include "greylist.h"
#include "query_skt.h"
#include "fn_handler.h"
#include "error.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef IPV6
#include <netdb.h>
#endif

#define FMT_ULONG 40

typedef struct sockaddr_in  sockaddr_in;
#ifdef IPV6
typedef struct sockaddr_in6 sockaddr_in6;
#endif

/*
 * Takes a string specifying IP address and port, separated by '@' If IP
 * address and/or port are missing, supplied defaults are used.
 */
int
scan_ip_port(const char *gip, const char *defaultip, unsigned int defaultport,
		union v46addr *ipp, unsigned long *portp)
{
	int             n;
	unsigned long   port;	/* long because of scan_ulong */
#ifdef IPV6
	char           *ptr;
	const char     *ip_a;
#endif

#ifdef IPV6
	if (!gip) {
		if (!(n = ip6_scan(defaultip, &ipp->ip6))) {
			errno = EINVAL;
			return (-1);
		}
		if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
			port = defaultport;
	} else {
		ip_a = (*gip == '@' && !*(gip + 1)) ? defaultip : gip;
		for (ptr = (char *) ip_a; *ptr; ptr++) {
			if (*ptr == '@' && scan_ulong(ptr + 1, &port)) {
				*portp = port;
				break;
			}
		}
		if (!*ptr)
			*portp = port = defaultport;
		else
			*ptr = 0;
		if (*ip_a == '0' && !*(ip_a + 1))
			ip_a = "0.0.0.0";
		if (!(n = ip6_scan(ip_a, &ipp->ip6))) {
			errno = EINVAL;
			return (-1);
		}
	}
#else
	if (!gip) {
		if (!(n = ip4_scan(defaultip, &ipp->ip)))
			return (-1);
		if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
			port = defaultport;
	} else {
		if (!(n = ip4_scan(gip, &ipp->ip))) {
			if (!(n = ip4_scan(defaultip, &ipp->ip)))
				return (-1);
			if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
				port = defaultport;
		} else
		if (!(*(gip + n) == '@' && scan_ulong(gip + n + 1, &port)))
			port = defaultport;
	}
#endif
	*portp = port;
	return (n);
}

int
connect_udp(union v46addr *ip, unsigned int port, void (*errfn)(const char *))
{
	int               fd;
#ifdef IPV6
	sockaddr_in6    sa;
	sockaddr_in6   *sin6;
	sockaddr_in    *sin4;
#else
	sockaddr_in     sout;
#endif

#ifdef IPV6
	byte_zero((char *) &sa, sizeof(sa));
	if (noipv6) {
		sin4 = (sockaddr_in *) &sa;
		sin4->sin_family = AF_INET;
		sin4->sin_port = htons(port);
		byte_copy((char *) &sin4->sin_addr, 4, (char *) ip->ip.d);
	} else {
		if (ip6_isv4mapped(ip->ip6.d)) {
			sin4 = (sockaddr_in *) &sa;
			sin4->sin_family = AF_INET;
			sin4->sin_port = htons(port);
			byte_copy((char *) &sin4->sin_addr, 4, (char *) ip->ip6.d + 12);
			noipv6 = 1;
		} else
		if (byte_equal((char *) ip->ip6.d, 16, (char *) V6loopback)) {
			sin4 = (sockaddr_in *) &sa;
			sin4->sin_family = AF_INET;
			sin4->sin_port = htons(port);
			byte_copy((char *) &sin4->sin_addr, 4, ip4loopback);
			noipv6 = 1;
		} else {
			sin6 = (sockaddr_in6 *) &sa;
			sin6->sin6_family = AF_INET6;
			sin6->sin6_port = htons(port);
			byte_copy((char *) &sin6->sin6_addr, 16, (char *) ip->ip6.d);
		}
	}
	if ((fd = socket(noipv6 ? AF_INET : AF_INET6, SOCK_DGRAM, 0)) == -1)
		return (errfn ? fn_handler(errfn, 0, 0, noipv6 ? "socket(AF_INET)" : "socket(AF_INET6)") : -1);
	if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
		return (errfn ? fn_handler(errfn, 0, 0, "connect") : -1);
#else
	byte_zero((char *) &sout, sizeof(sout));
	sout.sin_port = htons(port);
	sout.sin_family = AF_INET;
	byte_copy((char *) &sout.sin_addr, 4, (char *) &ip->ip);
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (errfn ? fn_handler(errfn, 0, 0, "socket(AF_INET)") : -1);
	if (connect(fd, (struct sockaddr *)&sout, sizeof(sout)) < 0)
		return (errfn ? fn_handler(errfn, 0, 0, "connect") : -1);
#endif
	return (fd);
}

/*
 * Check given greylist triple: Pass connectingip+from+tolist to greydaemon
 * on IP address gip. timeoutfn and errfn passed in case of error. Note that
 * greylist may be called more than once during a single SMTP session (=
 * qmail-smtpd instance).
 */
stralloc        chkpacket = {0};
stralloc        ipbuf = {0};

int
greylist(const char *gip, const char *connectingip, const char *from,
		const char *tolist, int tolen, void (*timeoutfn) (void), void (*errfn) (const char *)) /*- errfn must _exit */
{
	int             r, len = 0;
	char            strnum[FMT_ULONG];
	char            z[IPFMT];
	unsigned long   port;
	union v46addr   ip;
	char            rbuf[2];
	static int      sockfd = -1;

	if (!gip) {
		errno = EINVAL;
		return (errfn ? fn_handler(errfn, 0, 0, "Invalid IP") : -1);
	}
	if (sockfd == -1) {
		if (scan_ip_port(gip, DEFAULTGREYIP, DEFAULTGREYPORT, &ip, &port) == -1)
			return (errfn ? fn_handler(errfn, 0, 0, "ip scan") : -1);
#ifdef IPV6
		len = ip6_fmt(z, &ip.ip6);
#else
		len = ip4_fmt(z, &ip.ip);
#endif
		if (!stralloc_copyb(&ipbuf, "greylist: ip[", 13))
			return (-2);
		else
		if (!stralloc_catb(&ipbuf, z, len))
			return (-2);
		else
		if (!stralloc_catb(&ipbuf, "] port[", 7))
			return (-2);
		strnum[len = fmt_ulong(strnum, port)] = 0;
		if (!stralloc_catb(&ipbuf, strnum, len))
			return (-2);
		else
		if (!stralloc_catb(&ipbuf, "]", 1))
			return (-2);
		else
		if (!stralloc_0(&ipbuf))
			return (-2);
		if ((sockfd = connect_udp(&ip, port, errfn)) == -1)
			return (errfn ? fn_handler(errfn, 0, 0, "connect_udp") : -1);
	}
	if (!stralloc_copyb(&chkpacket, "I", 1))
		return (-2); /*- ENOMEM */
	else
	if (!stralloc_cats(&chkpacket, connectingip))
		return (-2);
	else
	if (!stralloc_0(&chkpacket))
		return (-2);
	else
	if (!stralloc_append(&chkpacket, "F"))
		return (-2);
	else
	if (!stralloc_cats(&chkpacket, from))
		return (-2);
	else
	if (!stralloc_0(&chkpacket))
		return (-2);
	else
	if (!stralloc_catb(&chkpacket, tolist, tolen))
		return (-2);
	else
	if (!stralloc_0(&chkpacket))
		return (-2);
	else
	if ((r = query_skt(sockfd, ipbuf.s, &chkpacket, rbuf, sizeof rbuf, GREYTIMEOUT, timeoutfn, errfn)) == -1)
		return -1;	/*- Permit connection (soft fail) - probably timeout */
	else
	if (rbuf[0] == 0)
		return (0);	/* greylist */
	return (1);
}

void
getversion_greylist_c()
{
	const char     *x = "$Id: greylist.c,v 1.16 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: greylist.c,v $
 * Revision 1.16  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.15  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2018-05-30 23:25:31+05:30  Cprogrammer
 * moved noipv6 variable to variables.c
 *
 * Revision 1.12  2018-04-25 22:48:02+05:30  Cprogrammer
 * fixed error message length
 *
 * Revision 1.11  2018-04-25 21:39:39+05:30  Cprogrammer
 * moved query_skt(), fn_handler() to its own source file
 *
 * Revision 1.10  2016-04-19 10:33:35+05:30  Cprogrammer
 * added additional diagnostics when logging errors
 *
 * Revision 1.9  2016-04-15 15:44:11+05:30  Cprogrammer
 * create ipv4 socket if ipv6 stack is disabled
 *
 * Revision 1.8  2016-04-10 22:17:32+05:30  Cprogrammer
 * null terminate packet
 *
 * Revision 1.7  2016-04-10 13:27:12+05:30  Cprogrammer
 * fixed ip6/ip4 address to connect
 *
 * Revision 1.6  2015-12-31 00:44:18+05:30  Cprogrammer
 * added IPV6 code
 *
 * Revision 1.5  2015-08-24 19:06:05+05:30  Cprogrammer
 * replace ip_scan() with ip4_scan()
 *
 * Revision 1.4  2014-01-29 14:00:42+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.3  2009-10-28 13:34:29+05:30  Cprogrammer
 * fix scan_ip_port()
 * remove newline as delimiter
 *
 * Revision 1.2  2009-08-29 15:28:45+05:30  Cprogrammer
 * send the entire RCPT list in one packet
 *
 * Revision 1.1  2009-08-25 16:31:56+05:30  Cprogrammer
 * Initial revision based on code by Richard Andrews
 *
 */
