/*
 * $Log: tlsacheck.c,v $
 * Revision 1.3  2018-05-27 22:14:32+05:30  mbhangui
 * added defintions for qmail-daned modes
 *
 * Revision 1.2  2018-05-27 17:47:05+05:30  Cprogrammer
 * added option for qmail-remote to query/update records
 *
 * Revision 1.1  2018-04-26 01:30:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef IPV6
#include <netdb.h>
#endif
#include "haveip6.h"
#include "stralloc.h"
#include "ip.h"
#include "fmt.h"
#include "byte.h"
#include "scan.h"
#include "env.h"
#include "tlsacheck.h"
#include "error.h"
#include "fn_handler.h"
#include "query_skt.h"

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
scan_ip_port(daneip, defaultip, defaultport, ipp, portp)
	char           *daneip, *defaultip;
	unsigned int    defaultport;
	union v46addr  *ipp;
	unsigned long  *portp;
{
	int             n;
	unsigned long   port;	/* long because of scan_ulong */
#ifdef IPV6
	char           *ptr, *ip_a;
#endif

#ifdef IPV6
	if (!daneip) {
		if (!(n = ip6_scan(defaultip, &ipp->ip6))) {
			errno = EINVAL;
			return (-1);
		}
		if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
			port = defaultport;
	} else {
		ip_a = (*daneip == '@' && !*(daneip + 1)) ? defaultip : daneip;
		for (ptr = ip_a;*ptr;ptr++) {
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
	if (!daneip) {
		if (!(n = ip4_scan(defaultip, &ipp->ip)))
			return (-1);
		if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
			port = defaultport;
	} else {
		if (!(n = ip4_scan(daneip, &ipp->ip))) {
			if (!(n = ip4_scan(defaultip, &ipp->ip)))
				return (-1);
			if (!(*(defaultip + n) == '@' && scan_ulong(defaultip + n + 1, &port)))
				port = defaultport;
		} else
		if (!(*(daneip + n) == '@' && scan_ulong(daneip + n + 1, &port)))
			port = defaultport;
	}
#endif
	*portp = port;
	return (n);
}

int
connect_udp(ip, port, errfn)
	union   v46addr  *ip;
	unsigned int      port;
	void              (*errfn)();
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
 * Pass doamin to qmail-dane daemon
 * on IP address daneip. timeoutfn and errfn passed in case of error. Note that
 * tlsacheck may be called more than once during a single SMTP session (=
 * qmail-remote instance).
 */
stralloc        chkpacket = {0};
stralloc        ipbuf = {0};

int
tlsacheck(daneip, domain, qOru, rbuf, timeoutfn, errfn)
	char           *daneip, *domain;
	int             qOru;
	char            rbuf[];
	void            (*timeoutfn) (), (*errfn) (); /*- errfn must _exit */
{
	int             r, len = 0, timeout = DANETIMEOUT;
	char           *ptr;
	char            strnum[FMT_ULONG], z[IPFMT];
	unsigned long   port;
	union v46addr   ip;
	static int      sockfd = -1;

	if (!daneip) {
		errno = EINVAL;
		return (errfn ? fn_handler(errfn, 0, 0, "Invalid IP") : -1);
	}
	if (sockfd == -1) {
		if (scan_ip_port(daneip, DEFAULTDANEIP, DEFAULTDANEPORT, &ip, &port) == -1)
			return (errfn ? fn_handler(errfn, 0, 0, "ip scan") : -1);
#ifdef IPV6
		len = ip6_fmt(z, &ip.ip6);
#else
		len = ip4_fmt(z, &ip.ip);
#endif
		if (!stralloc_copyb(&ipbuf, "tlsacheck: ip[", 14))
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
	if ((ptr = env_get("DANETIMEOUT")))
		scan_int(ptr, &timeout);
	switch (qOru)
	{
		case QUERY_MODE:
			ptr = "q";
			break;
		case UPDATE_SUCCESS:
			ptr = "S";
			break;
		case UPDATE_FAILURE:
			ptr = "F";
			break;
		case DEFAULT_MODE:
		default:
			ptr = "D";
			break;
	}
	if (!stralloc_copyb(&chkpacket, ptr, 1))
		return (-2); /*- ENOMEM */
	else
	if (!stralloc_cats(&chkpacket, domain))
		return (-2);
	else
	if (!stralloc_0(&chkpacket))
		return (-2);
	else
	if ((r = query_skt(sockfd, ipbuf.s, &chkpacket, rbuf, 2, timeout, timeoutfn, errfn)) == -1)
		return -1;	/*- Permit connection (soft fail) - probably timeout */
	else {
		if (rbuf[0] == 0) { /* failure */
			if (*ptr == 'S' || *ptr == 'F')
				return (2);
			return (0);
		} else { /*- success */
			if (*ptr == 'S' || *ptr == 'F')
				return (3);
			return (1);
		}
	}
}

void
getversion_tlsacheck_c()
{
	static char    *x = "$Id: tlsacheck.c,v 1.3 2018-05-27 22:14:32+05:30 mbhangui Exp mbhangui $";

	x++;
}
