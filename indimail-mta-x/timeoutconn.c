/*
 * $Log: timeoutconn.c,v $
 * Revision 1.17  2022-11-24 08:49:54+05:30  Cprogrammer
 * converted function declaration to ansic
 *
 * Revision 1.16  2020-09-16 19:08:17+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.15  2020-06-08 22:52:23+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.14  2016-04-15 15:43:35+05:30  Cprogrammer
 * added comments of #ifdef statements
 *
 * Revision 1.13  2015-08-24 19:09:38+05:30  Cprogrammer
 * replace ip_scan() with ip4_scan(), replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.12  2011-07-29 10:20:53+05:30  Cprogrammer
 * fixed array subscript is above array bounds [-Warray-bounds] warning
 *
 * Revision 1.11  2010-04-03 12:54:31+05:30  Cprogrammer
 * fix for bind on ipv6 address
 *
 * Revision 1.10  2005-08-23 17:40:38+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.9  2005-06-17 21:51:41+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.8  2004-10-22 20:31:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-22 15:40:00+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.6  2004-10-11 14:53:58+05:30  Cprogrammer
 * use outgoing ip only if bindroutes does not return a local ip
 *
 * Revision 1.5  2004-10-09 23:38:56+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.4  2004-07-17 21:24:48+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ndelay.h"
#include "select.h"
#include "error.h"
#include "ip.h"
#include "byte.h"
#include "haveip6.h"
#include "timeoutconn.h"

#define BIND_SOCKET 1	/*- 0 to ignore bind fail, 1 to tempfail and requeue */

#ifdef BIND_SOCKET
#include "control.h"
#include "constmap.h"
#include "stralloc.h"

typedef struct sockaddr_in  sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;

int
socket_bind4(int s, char *bound, ip_addr *ipr)
{
	sockaddr_in     salocal;
	ip_addr         iplocal;
	char           *ipstr, ipstring[IPFMT + 1];
	int             iplen;
	stralloc        routes = { 0 };
	struct constmap bindroutes;
	const char     *bindroute = (char *) 0;

	*bound = 0;
	/*- Right, do we actually have any bind routes?  */
	switch (control_readfile(&routes, "bindroutes", 0))
	{
	case 0:
		return 0;	/*- no file, no bind to worry about */
	case -1:
		return -2;	/*- buggered up somewhere, urgh!  */
	case 1:
		if (!constmap_init(&bindroutes, routes.s, routes.len, 1))
			return -3;
	}
	ipstring[0] = '.';	/*- "cheating", but makes the loop check easier below!  */
	ipstr = ipstring + 1;
	iplen = ip4_fmt(ipstr, ipr);	/*- Well, Dan seems to trust its output!  */
	/*- check d.d.d.d, d.d.d., d.d., d., none */
	if (!(bindroute = constmap(&bindroutes, ipstr, iplen))) {
		while (iplen--) {	/*- no worries - the lost char must be 0-9 */
			if (ipstring[iplen] == '.' && (bindroute = constmap(&bindroutes, ipstr, iplen)))
				break;
		}
	}
	if (!bindroute || !*bindroute)
		return 0;	/*- no bind required */
	if (!ip4_scan(bindroute, &iplocal))
		return -4;	/*- wasn't an ip returned */
	byte_zero((char *) &salocal, sizeof(salocal));
	salocal.sin_family = AF_INET;
	byte_copy((char *) &salocal.sin_addr, 4, (char *) &iplocal);
	if (bind(s, (struct sockaddr *) &salocal, sizeof(salocal)))
		return BIND_SOCKET;
	*bound = 1;
	return 0;
}

#ifdef IPV6
int
socket_bind6(int s, char *bound, ip6_addr *ipr)
{
	ip6_addr        iplocal;
#ifdef LIBC_HAS_IP6
	sockaddr_in6    salocal;
	char           *ipstr, ipstring[IPFMT + 1];
	int             iplen;
	stralloc        routes = { 0 };
	struct constmap bindroutes;
	const char     *bindroute = (char *) 0;

	*bound = 0;
	/*- Right, do we actually have any bind routes?  */
	switch (control_readfile(&routes, "bindroutes", 0))
	{
	case 0:
		return 0;	/*- no file, no bind to worry about */
	case -1:
		return -2;	/*- buggered up somewhere, urgh!  */
	case 1:
		if (!constmap_init(&bindroutes, routes.s, routes.len, 1))
			return -3;
	}
	ipstring[0] = '.';	/*- "cheating", but makes the loop check easier below!  */
	ipstr = ipstring + 1;
	iplen = ip6_fmt(ipstr, ipr);
	/*- check d.d.d.d, d.d.d., d.d., d., none */
	if (!(bindroute = constmap(&bindroutes, ipstr, iplen))) {
		while (iplen--) {	/*- no worries - the lost char must be 0-9 */
			if (ipstring[iplen] == '.' && (bindroute = constmap(&bindroutes, ipstr, iplen)))
				break;
		}
	}
	if (!bindroute || !*bindroute)
		return 0;	/*- no bind required */
	if (!ip6_scan(bindroute, &iplocal))
		return -4;	/*- wasn't an ip returned */
	if (noipv6) {
		int             i;

		for (i = 0; i < 16; i++) {
			if (iplocal.d[i] != 0)
				break;
		}
		if (i == 16 || ip6_isv4mapped(iplocal.d))
			return socket_bind4(s, bound, (ip_addr *) (iplocal.d + 12));
		else {
			errno = error_proto;
			return (-1);
		}
	}
	byte_zero((char *) &salocal, sizeof(salocal));
	salocal.sin6_family = AF_INET6;
	byte_copy((char *) &salocal.sin6_addr, 16, (char *) &iplocal);
	if (bind(s, (struct sockaddr *) &salocal, sizeof(salocal)))
		return BIND_SOCKET;
	*bound = 1;
	return 0;
#else
	int             i;

	for (i = 0; i < 16; i++) {
		if (iplocal.d[i] != 0)
			break;
	}
	if (i == 16 || ip6_isv4mapped(iplocal.d))
		return socket_bind4(s, bound, (ip_addr *) iplocal.d + 12);
	errno = error_proto;
	return -1;
#endif /*- #ifdef LIBC_HAS_IP6 */
}
#endif /*- ifdef IPV6 */
#endif /*- ifdef BIND_SOCKET */

#ifdef IPV6
int
timeoutconn6(int s, ip6_addr *ipr, union v46addr *ipl, unsigned int port, int timeout)
{
	int             i;
	char            ch, bound = 0;
	sockaddr_in    *sin4;
#if LIBC_HAS_IP6
	sockaddr_in6    sa;
	sockaddr_in6   *sin6;
#else
	struct sockaddr sa;
#endif
	fd_set          wfds;
	struct timeval  tv;

	/*- XXX: could bind s */
#ifdef BIND_SOCKET
	if ((ch = socket_bind6(s, &bound, ipr)))
		return ch;
#endif
	/*
	 * use outgoing ipaddr only if bind_socket did not bind on a local IP
	 */
	if (!bound) {
		byte_zero((char *) &sa, sizeof(sa));
#ifdef LIBC_HAS_IP6
		if (noipv6) {
			for (i = 0; i < 16; i++) {
				if (ipl->ip6.d[i] != 0)
					break;
			}
			if (i == 16 || ip6_isv4mapped(ipl->ip6.d)) {
				sin4 = (sockaddr_in *) &sa;
				sin4->sin_family = AF_INET;
				byte_copy((char *) &sin4->sin_addr, 4, (char *) &ipl->ip6 + 12);
			} else {
				errno = error_proto;
				return -1;
			}
		} else {
			sin6 = &sa;
			sin6->sin6_family = AF_INET6;
			byte_copy((char *) &sin6->sin6_addr, 16, (char *) &ipl->ip6);
		}
#else
		for (i = 0; i < 16; i++) {
			if (ipl->ip.d[i] != 0)
				break;
		}
		if (i == 16 || ip6_isv4mapped(ipl->ip.d)) {
			sin4 = (sockaddr_in *) &sa;
			sin4->sin_family = AF_INET;
			byte_copy((char *) &sin4->sin_addr, 4, (char *) &ipl->ip6 + 12);
		} else {
			errno = error_proto;
			return -1;
		}
#endif
		if (-1 == bind(s, (struct sockaddr *) &sa, sizeof(sa)))
			return -1;
	}
#if LIBC_HAS_IP6
	byte_zero((char *) &sa, sizeof(sa));
	if (noipv6) {
		if (ip6_isv4mapped(ipr->d)) {
			sin4 = (sockaddr_in *) &sa;
			sin4->sin_family = AF_INET;
			sin4->sin_port = htons(port);
			byte_copy((char *) &sin4->sin_addr, 4, (char *) &ipr->d + 12);
		} else
		if (byte_equal((char *) ipr->d, 16, (char *) V6loopback)) {
			sin4 = (sockaddr_in *) &sa;
			sin4->sin_family = AF_INET;
			sin4->sin_port = htons(port);
			byte_copy((char *) &sin4->sin_addr, 4, ip4loopback);
		} else {
			errno = error_proto;
			return -1;
		}
	} else {
		sin6 = (sockaddr_in6 *) &sa;
		sin6->sin6_family = AF_INET6;
		sin6->sin6_port = htons(port);
		byte_copy((char *) &sin6->sin6_addr, 16, (char *) ipr);
	}
#else
	if (ip6_isv4mapped(ipr->d)) {
		sin4 = (sockaddr_in *) &sa;
		sin4->sin_family = AF_INET;
		sin4->sin_port = htons(port);
		byte_copy((char *) &sin4->sin_addr, 4, (char *) &ipr->d + 12);
	} else
	if (byte_equal(ipr->d, 16, (char *) V6loopback)) {
		sin4 = (sockaddr_in *) &sa;
		sin4->sin_family = AF_INET;
		sin4->sin_port = htons(port);
		byte_copy((char *) &sin4->sin_addr, 4, ip4loopback);
	} else {
		errno = error_proto;
		return -1;
	}
#endif
	if (ndelay_on(s) == -1)
		return -1;
	if (connect(s, (struct sockaddr *) &sa, sizeof(sa)) == 0) {
		ndelay_off(s);
		return 0;
	}
	if ((errno != error_inprogress) && (errno != error_wouldblock))
		return -1;
	FD_ZERO(&wfds);
	FD_SET(s, &wfds);
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	if (select(s + 1, (fd_set *) 0, &wfds, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(s, &wfds)) {
		int             dummy;
		dummy = sizeof(sa);
		if (getpeername(s, (struct sockaddr *) &sa, (socklen_t *) &dummy) == -1) {
			if (read(s, &ch, 1) == -1)
				;
			return -1;
		}
		ndelay_off(s);
		return 0;
	}
	errno = error_timeout;	/*- note that connect attempt is continuing */
	return -1;
}
#endif /*- #ifdef IPV6 */

int
timeoutconn4(int s, ip_addr *ipr, union v46addr *ipl, unsigned int port, int timeout)
{
	char            ch, bound = 0;
	sockaddr_in     sin;
	fd_set          wfds;
	struct timeval  tv;

	/*- XXX: could bind s */
#ifdef BIND_SOCKET
	if ((ch = socket_bind4(s, &bound, ipr)))
		return ch;
#endif
	/*
	 * use outgoing ipaddr only if bind_socket did not bind on a local IP
	 */
	if (!bound) {
		byte_zero((char *) &sin, sizeof(sin));
		sin.sin_family = AF_INET;
		byte_copy((char *) &sin.sin_addr.s_addr, 4, (char *) &ipl->ip);
		if (-1 == bind(s, (struct sockaddr *) &sin, sizeof(sin)))
			return -1;
	}
	byte_zero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	byte_copy((char *) &sin.sin_addr, 4, (char *) ipr);
	sin.sin_port = htons(port);
	if (ndelay_on(s) == -1)
		return -1;
	if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) == 0) {
		ndelay_off(s);
		return 0;
	}
	if ((errno != error_inprogress) && (errno != error_wouldblock))
		return -1;
	FD_ZERO(&wfds);
	FD_SET(s, &wfds);
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	if (select(s + 1, (fd_set *) 0, &wfds, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(s, &wfds)) {
		int             dummy;
		dummy = sizeof(sin);
		if (getpeername(s, (struct sockaddr *) &sin, (socklen_t *) &dummy) == -1) {
			if (read(s, &ch, 1) == -1)
				;
			return -1;
		}
		ndelay_off(s);
		return 0;
	}
	errno = error_timeout;	/*- note that connect attempt is continuing */
	return -1;
}

void
getversion_timeoutconn_c()
{
	const char     *x = "$Id: timeoutconn.c,v 1.17 2022-11-24 08:49:54+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
