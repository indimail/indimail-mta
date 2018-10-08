/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "rfc1035.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif


#if HAVE_SIOCGIFCONF

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <sys/ioctl.h>
#include <net/if.h>

static int getifconf(int fd, struct rfc1035_ifconf **ifconf_ptr)
{
	struct ifreq ifreq_buf[64];
	struct ifconf ifc;
	int i;
	const struct sockaddr_in *sin;
	RFC1035_ADDR addr;
	char ipaddr[RFC1035_NTOABUFSIZE];
	struct rfc1035_ifconf *ifcptr;
	struct rfc1035_ifconf **ifconf;

	ifc.ifc_len=sizeof(ifreq_buf);
	ifc.ifc_req=ifreq_buf;

	if (ioctl(fd, SIOCGIFCONF, &ifc) < 0)
		return (0);

	for (i=0; i * sizeof(struct ifreq) < ifc.ifc_len; i++)
	{
		sin=(const struct sockaddr_in *)&ifreq_buf[i].ifr_addr;

#if RFC1035_IPV6
		if (sin->sin_family == AF_INET6)
		{
			struct sockaddr_in6 sin6;

			memcpy(&sin6, sin, sizeof(sin6));
			addr=sin6.sin6_addr;
		}
		else if (sin->sin_family == AF_INET)
			rfc1035_ipv4to6(&addr, &sin->sin_addr);
		else
			continue;
#else
		if (sin->sin_family == AF_INET)
			addr=sin->sin_addr;
		else
			continue;
#endif

		rfc1035_ntoa(&addr, ipaddr);

		/*
		** Eliminate any dupes.
		*/

		ifconf=ifconf_ptr;

		while ( *ifconf )
		{
			char ipaddr2[RFC1035_NTOABUFSIZE];

			rfc1035_ntoa(&(*ifconf)->ifaddr, ipaddr2);

			if (strcmp(ipaddr, ipaddr2) == 0)
				break;

			ifconf= &(*ifconf)->next;
		}

		if ( *ifconf )
			continue;	/* Already have this IP addr */

		if ( (ifcptr=malloc(sizeof(struct rfc1035_ifconf))) == NULL ||
		     (ifcptr->ifname=strdup(ifreq_buf[i].ifr_name)) == NULL)
		{
			if (ifcptr)
				free(ifcptr);
			return (-1);
		}

		ifcptr->ifaddr=addr;
		ifcptr->next=NULL;

		*ifconf= ifcptr;
	}
	return (0);
}

/*
** On systems that support IPv6, issue an SIOCGIFCONF on both an IPv4 and
** an IPv6 socket, for good measure.
*/

static int doifconf(struct rfc1035_ifconf **ifconf_list)
{
	int fd;

	fd=socket(PF_INET, SOCK_STREAM, 0);

	if (fd >= 0)
	{
		if (getifconf(fd, ifconf_list))
		{
			close(fd);
			return (-1);
		}
		close(fd);
	}

#if RFC1035_IPV6

	fd=socket(PF_INET6, SOCK_STREAM, 0);

	if (fd >= 0)
	{
		if (getifconf(fd, ifconf_list))
		{
			close(fd);
			return (-1);
		}
		close(fd);
	}
#endif
	return (0);
}

struct rfc1035_ifconf *rfc1035_ifconf(int *errflag)
{
	struct rfc1035_ifconf *ifconf_list=NULL;
	int dummy;

	if (!errflag)
		errflag= &dummy;

	*errflag= -1;
	if (doifconf(&ifconf_list))
	{
		rfc1035_ifconf_free(ifconf_list);
		return (NULL);
	}

	*errflag=0;
	return ifconf_list;
}

#else

struct rfc1035_ifconf *rfc1035_ifconf(int *errflag)
{
	if (errflag)
		*errflag=0;
	return NULL;
}
#endif

void rfc1035_ifconf_free(struct rfc1035_ifconf *ifconf_list)
{
	while (ifconf_list)
	{
		struct rfc1035_ifconf *p=ifconf_list->next;

		free(ifconf_list->ifname);
		free(ifconf_list);
		ifconf_list=p;
	}
}

