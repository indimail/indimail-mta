/*
 * $Log: islocalif.c,v $
 * Revision 2.6  2016-05-17 17:09:39+05:30  mbhangui
 * use control directory set by configure
 *
 * Revision 2.5  2014-04-17 11:39:45+05:30  Cprogrammer
 * display hostname in error message
 *
 * Revision 2.4  2011-04-08 17:26:42+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.3  2009-12-11 13:11:21+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.2  2009-12-10 12:05:12+05:30  Cprogrammer
 * use ip from localiphost if present
 *
 * Revision 2.1  2009-07-09 15:55:20+05:30  Cprogrammer
 * ipv6 ready code
 *
 * Revision 1.3  2001-12-11 11:31:38+05:30  Cprogrammer
 * inclusion of sockio.h for solaris
 *
 * Revision 1.2  2001-12-09 01:53:41+05:30  Cprogrammer
 * use gethostbyname to figure out ip address when argument
 * passed is a hostname
 *
 * Revision 1.1  2001-12-08 15:43:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef linux
#include <linux/sockios.h>
#endif
#ifdef sun
#include <sys/sockio.h>
#endif
#include "indimail.h"

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef	lint
static char     sccsid[] = "$Id: islocalif.c,v 2.6 2016-05-17 17:09:39+05:30 mbhangui Exp $";
#endif

/*
 * RFC2553 proposes struct sockaddr_storage. This is a placeholder for all sockaddr-variant structures.
 * union
 * {
 *   struct sockaddr_storage ss;
 *   struct sockaddr_in s4;
 *   struct sockaddr_in6 s6;
 * } u;
 *
 *  - replace salocal references with u.ss
 *  - in the IPv4 branch, use u.s4 instead of the local sa variable
 *  - in the IPv6 branch, use u.s6 instead of the local sa variable  
 */
int
islocalif(char *hostptr)
{
	struct ifconf   ifc;
	struct ifreq   *ifr;
	int             s, len, idx, family;
	char           *buf, *ptr;
	struct sockaddr_in *sin = 0;
	char            TmpBuf[MAX_BUFF];
	char           *qmaildir, *controldir;
	FILE           *fp;
#ifdef ENABLE_IPV6
	struct sockaddr_in6 *sin6 = 0;
#endif
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	struct sockaddr *sa;
	char            addrBuf[INET6_ADDRSTRLEN];
	char            namebuf[INET6_ADDRSTRLEN];
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	int             error, salen;
#else
	struct hostent *hp;
	char           *ipaddr;
	char            ipbuf[16];
	unsigned long   inaddr;
#endif

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(TmpBuf, MAX_BUFF, "%s/localiphost", controldir);
	else {
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/localiphost", qmaildir, controldir);
	}
	if ((fp = fopen(TmpBuf, "r")))
	{
		if (!fgets(TmpBuf, MAX_BUFF - 1, fp))
			fclose(fp);
		else
		{
			fclose(fp);
			if ((ptr = strchr(TmpBuf, '\n')))
				*ptr = 0;
			if (!strncmp(hostptr, TmpBuf, MAX_BUFF))
				return (1);
		}
	}
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	/* getaddrinfo() case.  It can handle multiple addresses. */
	memset(&hints, 0, sizeof(hints));
	/* set-up hints structure */
	hints.ai_family = PF_UNSPEC;
	if ((error = getaddrinfo(hostptr, NULL, &hints, &res0)))
	{
		fprintf(stderr, "islocalif: getaddrinfo: %s: %s\n", hostptr, gai_strerror(error));
		return (-1);
	}
#else
	if ((inaddr = inet_addr(hostptr)) == INADDR_NONE) /*- It's not a dotted decimal */
	{			
		if ((hp = gethostbyname(hostptr)) == NULL)
		{
			errno = EINVAL;
			return (-1);
		} 
		memcpy(ipbuf, inet_ntoa(*((struct in_addr *) hp->h_addr)), 16);
		ipaddr = ipbuf;
	} else
		ipaddr = hostptr;
#endif
#ifdef ENABLE_IPV6
	s = socket(AF_INET6, SOCK_DGRAM, 0);
	if (s == -1 && (errno == EINVAL || errno == EAFNOSUPPORT))
		s = socket(AF_INET, SOCK_STREAM, 0);
#else
	s = socket(AF_INET, SOCK_DGRAM, 0);
#endif
	if (s == -1)
	{
#ifdef ENABLE_IPV6
		freeaddrinfo(res0);
#endif
		return (-1);
	}
	len = 8192;
	for (buf = (char *) 0;;)
	{
		buf = realloc(buf, len * sizeof(char));
		ifc.ifc_buf = buf;
		ifc.ifc_len = len;
		if (ioctl(s, SIOCGIFCONF, &ifc) >= 0) /*- > is for System V */
		{
			if (ifc.ifc_len + sizeof(*ifr) + 64 < len)
			{
				/*- what a stupid interface */
				*(buf + ifc.ifc_len) = 0;
				break;
			}
		}
		if (len > 200000)
		{
			close(s);
#ifdef ENABLE_IPV6
			freeaddrinfo(res0);
#endif
			return(-1);
		}
		len *= 2;
	}
	ifr = ifc.ifc_req;
	for (idx = ifc.ifc_len / sizeof(struct ifreq); --idx >= 0; ifr++)
	{
		family = ifr->ifr_addr.sa_family;
		if (family == AF_INET)
			sin = (struct sockaddr_in *) &ifr->ifr_addr;
#ifdef ENABLE_IPV6
		else
		if (family == AF_INET6)
			sin6 = (struct sockaddr_in6 *) &ifr->ifr_addr;
#endif
		else
			continue;
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
#ifdef ENABLE_IPV6
		if (!(ptr = (char *) inet_ntop(family, family == AF_INET ? (void *) &sin->sin_addr : (void *) &sin6->sin6_addr,
			addrBuf, INET6_ADDRSTRLEN)))
#else
		if (!(ptr = (char *) inet_ntop(AF_INET, (void *) &sin->sin_addr, addrBuf, INET6_ADDRSTRLEN)))
#endif
			continue;
		if (ioctl(s, SIOCGIFFLAGS, (char *) ifr))
			continue;
		/* Skip boring cases */
		if ((ifr->ifr_flags & IFF_UP) == 0)
			continue;
		/*
		if ((ifr->ifr_flags & (IFF_BROADCAST | IFF_POINTOPOINT)) == 0)
			continue;*/
		for (res = res0; res; res = res->ai_next) {
			sa = res->ai_addr;
			salen = res->ai_addrlen;
			if (getnameinfo(sa, salen, namebuf, sizeof(namebuf), 0, 0, NI_NUMERICHOST))
			{
				fprintf(stderr, "getnameinfo: %s\n", strerror(errno));
				continue;
			}
			if (res->ai_flags | AI_NUMERICHOST)
			{
				if (!strncmp(namebuf, ptr, INET6_ADDRSTRLEN))
				{
					close(s);
					free(buf);
					freeaddrinfo(res0);
					return (1);
				}
			}
		}
#else
		if (!(ptr = inet_ntoa(sin->sin_addr)))
			continue;
		if (!strncmp(ipaddr, ptr, INET_ADDRSTRLEN))
		{
			close(s);
			free(buf);
			return (1);
		}
#endif /*- #ifdef HAVE_STRUCT_SOCKADDR_STORAGE */
	} /*- for (idx = ifc.ifc_len / sizeof(struct ifreq); --idx >= 0; ifr++) */
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	freeaddrinfo(res0);
#endif
	free(buf);
	close(s);
	return (0);
}

void
getversion_islocalif_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
