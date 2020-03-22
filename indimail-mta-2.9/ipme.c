/*
 * $Log: ipme.c,v $
 * Revision 1.24  2018-07-03 02:02:11+05:30  Cprogrammer
 * set allocated to zero for stralloc variable buf
 *
 * Revision 1.23  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.22  2016-05-16 21:15:24+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.21  2015-08-24 21:48:48+05:30  Cprogrammer
 * added :: and 0.0.0.0 as valid ip addresses for me
 *
 * Revision 1.20  2015-08-24 21:30:20+05:30  Cprogrammer
 * use getifaddrs() to get interface addresses
 *
 * Revision 1.19  2015-08-24 19:06:46+05:30  Cprogrammer
 * replaced ip_scan() with ip4_scan()
 *
 * Revision 1.18  2008-09-16 20:12:54+05:30  Cprogrammer
 * represent ipv4 addresses as ipv4 and not as IPV4 mapped addresses
 *
 * Revision 1.17  2008-09-16 11:28:34+05:30  Cprogrammer
 * replaced inet_pton with ip6_scan
 * use outgoing ip in v6 for IPV6
 *
 * Revision 1.16  2008-09-16 09:56:09+05:30  Cprogrammer
 * fallback to AF_INET if ipv6 fails
 *
 * Revision 1.15  2008-06-17 18:59:27+05:30  Cprogrammer
 * use inet_pton() to convert addresses
 *
 * Revision 1.14  2008-06-17 18:00:13+05:30  Cprogrammer
 * ix.af was not initialized
 *
 * Revision 1.13  2005-06-17 21:49:00+05:30  Cprogrammer
 * replaced struct ip_address and struct ip6_address with shorter typedefs
 *
 * Revision 1.12  2005-06-15 22:34:15+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.11  2005-06-11 21:30:57+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.10  2004-10-22 20:26:00+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.9  2004-10-22 15:35:31+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.8  2004-10-09 00:57:10+05:30  Cprogrammer
 * removed param.h
 *
 * Revision 1.7  2004-08-15 00:26:22+05:30  Cprogrammer
 * ipalias fix for 4.3 BSD systems
 *
 * Revision 1.6  2004-07-17 20:55:33+05:30  Cprogrammer
 * increased buffer length to get all interface in one read
 *
 */
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include "hasifaddr.h"
#ifdef HASIFADDR
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#else
#include <sys/ioctl.h>
#ifndef SIOCGIFCONF	/*- whatever works */
#include <sys/sockio.h>
#endif
#endif

#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "hassalen.h"
#include "byte.h"
#include "alloc.h"
#include "ip.h"
#include "ipalloc.h"
#include "stralloc.h"
#include "open.h"
#include "getln.h"
#include "ipme.h"
#include "auto_control.h"
#ifdef MOREIPME
#include "substdio.h"
#include "variables.h"
#include "env.h"
#endif

typedef struct sockaddr_in  sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;

static int      ipmeok = 0;
ipalloc         ipme = { 0 };

int
ipme_is(ip)
	ip_addr        *ip;
{
	int             i;
	if (ipme_init() != 1)
		return -1;
	for (i = 0; i < ipme.len; ++i)
	{
		if (ipme.ix[i].af == AF_INET && byte_equal((char *) &ipme.ix[i].addr.ip, 4, (char *) ip))
			return 1;
	}
	return 0;
}

#ifdef IPV6
int ipme_is6(ip)
	ip6_addr       *ip;
{
	int             i;

	if (ipme_init() != 1)
		return -1;
	for (i = 0;i < ipme.len;++i)
	{
		if (ipme.ix[i].af == AF_INET6 && byte_equal((char *) &ipme.ix[i].addr.ip6, 16, (char *) ip))
			return 1;
	}
	return 0;
}
#endif


#ifdef MOREIPME
static stralloc buf = { 0 };
#define ipme_init_retclean(ret) { \
  if (notipme.ix) alloc_free((char *) notipme.ix); \
  if (moreipme.ix) alloc_free((char *) moreipme.ix); \
  if (buf.s) {alloc_free(buf.s);buf.len = buf.a = 0;} \
  return ret; }
#endif

#ifdef MOREIPME
int
ipme_readipfile(ipa, fn)
	ipalloc        *ipa;
	char           *fn;
{
	int             ret = 1;
	int             fd, match;
	static stralloc controlfile;
	char            inbuf[1024];
	substdio        ss;
	stralloc        l = { 0 };
	struct ip_mx    ix;

	if(!controldir)
	{
		if(!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&controlfile, controldir))
		return(-1);
	if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
		return(-1);
	if (!stralloc_cats(&controlfile, fn))
		return(-1);
	if (!stralloc_0(&controlfile))
		return(-1);
	if ((fd = open_read(controlfile.s)) != -1)
	{
		substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
		while ((getln(&ss, &l, &match, '\n') != -1) && (match || l.len))
		{
			l.len--;
			if (!stralloc_0(&l))
			{
				ret = 0;
				break;
			}
#ifdef IPV6
			ix.af = AF_INET6;
			if (!ip6_scan(l.s, &ix.addr.ip6)) /*- you can use inet_pton */
				continue;
			if (ip6_isv4mapped(&ix.addr.ip6.d))
			{
				ix.af = AF_INET;
				if (!ip4_scan(l.s, &ix.addr.ip))
					continue;
			}
#else
			ix.af = AF_INET;
			if (!ip4_scan(l.s, &ix.addr.ip))
				continue;
#endif
			if (!ipalloc_append(ipa, &ix))
			{
				ret = 0;
				break;
			}
		}
		if (l.s)
			alloc_free(l.s);
		if ((fd >= 0) && (close(fd) == -1))
			ret = 0;
	}
	return ret;
}

int
ipme_append_unless(ix, notip)
	struct ip_mx   *ix;
	struct ipalloc *notip;
{
	int             i;
	for (i = 0; i < notip->len; ++i)
	{
#ifdef IPV6
		if (notip->ix[i].af == AF_INET)
		{
			if (byte_equal((char *) &notip->ix[i].addr.ip, 4, (char *) &ix->addr.ip))
				return 1;
		} else
		if (byte_equal((char *) &notip->ix[i].addr.ip6, 16, (char *) &ix->addr.ip6))
			return 1;
#else
		if (byte_equal((char *) &notip->ix[i].addr.ip, 4, (char *) &ix->addr.ip))
			return 1;
#endif
	}
	return ipalloc_append(&ipme, ix);
}
#endif

int
ipme_init()
{
	int             family, s;
	sockaddr_in    *sin;
#ifdef HASIFADDR
#ifdef IPV6
	sockaddr_in6   *sin6;
#endif
	struct ifaddrs *ifaddr, *ifa;
	int             n;
	char            host[NI_MAXHOST];
#else
	struct ifconf   ifc;
	char           *x;
	struct ifreq   *ifr;
	int             len;
#endif /*- #ifdef HASIFADDR */
#ifdef MOREIPME
	ipalloc         notipme = { 0 };
	ipalloc         moreipme = { 0 };
	int             i;
#endif
	struct ip_mx    ix;

#ifdef MOREIPME
	if (ipmeok)
		return 1;
	if (!ipalloc_readyplus(&ipme, 0))
		ipme_init_retclean(0);
	if (!ipalloc_readyplus(&notipme, 0))
		ipme_init_retclean(0);
	if (!ipalloc_readyplus(&moreipme, 0))
		ipme_init_retclean(0);
#else
	if (ipmeok)
		return 1;
	if (!ipalloc_readyplus(&ipme, 0))
		return 0;
#endif
	ipme.len = 0;
	ix.pref = 0;
#ifdef MOREIPME
	if (!ipme_readipfile(&notipme, "notipme"))
		ipme_init_retclean(0);
#endif
	/*
	 * 0.0.0.0 is a special address which always refers to
	 * "this host, this network", according to RFC 1122, Sec. 3.2.1.3a.
	 */
#ifdef IPV6
	s = 0;
	if (!ip6_scan("::", &ix.addr.ip6))
	{
		s = -1;
		byte_copy((char *) &ix.addr.ip, 4, "\0\0\0\0");
		ix.af = AF_INET;
	} else
		ix.af = AF_INET6;
#else
	byte_copy((char *) &ix.addr.ip, 4, "\0\0\0\0");
	ix.af = AF_INET;
#endif
#ifdef MOREIPME
	if (!ipme_append_unless(&ix, &notipme))
		ipme_init_retclean(0);
#else
	if (!ipalloc_append(&ipme, &ix))
		return 0;
#endif
#ifdef IPV6
	if (!s) {
		byte_copy((char *) &ix.addr.ip, 4, "\0\0\0\0");
		ix.af = AF_INET;
#ifdef MOREIPME
		if (!ipme_append_unless(&ix, &notipme))
			ipme_init_retclean(0);
#else
		if (!ipalloc_append(&ipme, &ix))
			return 0;
#endif
	}
#endif

#ifdef HASIFADDR
	if (getifaddrs(&ifaddr) == -1)
#ifdef MOREIPME
		ipme_init_retclean(-1);
#else
		return 0;
#endif
	/*-
	 * walk through linked list, maintaining head pointer so we
	 * can free list later 
	 */
	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL)
			continue;
		family = ifa->ifa_addr->sa_family;
		if (family == AF_PACKET)
			continue;
		if (!(ifa->ifa_flags & IFF_UP))
			continue;
		/*-
		 * For an AF_INET* interface address, display the address 
		 */
		s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof (struct sockaddr_in) : sizeof (struct sockaddr_in6), host,
					NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			freeifaddrs(ifaddr);
#ifdef MOREIPME
			ipme_init_retclean(-1);
#else
			return 0;
#endif
		}
		if (strstr(host, ifa->ifa_name))
			continue;
		ix.af = family;
		if (family == AF_INET) {
			sin = (sockaddr_in *) ifa->ifa_addr;
			byte_copy((char *) &ix.addr.ip, 4, (char *) &sin->sin_addr);
		}
#ifdef IPV6
		else
		if (family == AF_INET6) {
			sin6 = (sockaddr_in6 *) ifa->ifa_addr;
			byte_copy((char *) &ix.addr.ip6, 16, (char *) &sin6->sin6_addr);
		}
#else
		if (family != AF_INET)
			continue;
#endif
#ifdef MOREIPME
		if (!ipme_append_unless(&ix, &notipme)) {
			freeifaddrs(ifaddr);
			ipme_init_retclean(0);
		}
#else
		if (!ipalloc_append(&ipme, &ix)) {
			freeifaddrs(ifaddr);
			return 0;
		}
#endif
	}
	freeifaddrs(ifaddr);
#else
#ifdef IPV6
	family = AF_INET6;
#else
	family = AF_INET;
#endif
#ifdef MOREIPME
	s = socket(family, SOCK_STREAM, 0);
#ifdef IPV6
	if (s == -1 && (errno == EINVAL || errno == EAFNOSUPPORT))
		s = socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (s == -1)
		ipme_init_retclean(-1);
#else /*- #ifdef MOREIPME */
	s = socket(family, SOCK_STREAM, 0);
#ifdef IPV6
	if (s == -1 && (errno == EINVAL || errno == EAFNOSUPPORT))
		s = socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (s == -1)
		return -1;
#endif /*- #ifdef MOREIPME */
	len = 8192; /*- any value big enough to get all interfaces in one read is good */
	for (;;)
	{
		if (!stralloc_ready(&buf, len))
		{
			close(s);
#ifdef MOREIPME
			ipme_init_retclean(0);
#else
			return(0);
#endif
		}
		buf.len = 0;
		ifc.ifc_buf = buf.s;
		ifc.ifc_len = len;
		if (ioctl(s, SIOCGIFCONF, &ifc) >= 0) /*- > is for System V */
		{
			if (ifc.ifc_len + sizeof(*ifr) + 64 < len)
			{
				/*- what a stupid interface */
				buf.len = ifc.ifc_len;
				break;
			}
		}
		if (len > 200000)
		{
			close(s);
#ifdef MOREIPME
			ipme_init_retclean(-1);
#else
			return(-1);
#endif
		}
		len *= 2;
	}
	x = buf.s;
	while (x < buf.s + buf.len)
	{
		ifr = (struct ifreq *) x;
#ifdef HASSALEN
		len = sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len;
#else
		len = sizeof(*ifr);
#endif
		if (len < sizeof(*ifr))
			len = sizeof(*ifr);
		if (ifr->ifr_addr.sa_family == AF_INET)
		{
			sin = (sockaddr_in *) &ifr->ifr_addr;
			byte_copy((char *) &ix.addr.ip, 4, (char *) &sin->sin_addr);
			ix.af = AF_INET;
			if (ioctl(s, SIOCGIFFLAGS, x) == 0)
			{
				if (ifr->ifr_flags & IFF_UP)
				{
#ifdef MOREIPME
					if (!ipme_append_unless(&ix, &notipme))
					{
						close(s);
						ipme_init_retclean(0);
					}
#else
					if (!ipalloc_append(&ipme, &ix))
					{
						close(s);
						return 0;
					}
#endif
				}
			}
		}
		x += len;
	}
	close(s);
#endif /*- #ifdef HASIFADDR */

#ifdef MOREIPME
	if (!ipme_readipfile(&moreipme, "moreipme"))
		ipme_init_retclean(0);
	for (i = 0; i < moreipme.len; ++i)
	{
		if (!ipme_append_unless(&moreipme.ix[i], &notipme))
			ipme_init_retclean(0);
	}
	ipmeok = 1;
	ipme_init_retclean(1);
#else
	ipmeok = 1;
	return 1;
#endif
}

void
getversion_ipme_c()
{
	static char    *x = "$Id: ipme.c,v 1.24 2018-07-03 02:02:11+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
