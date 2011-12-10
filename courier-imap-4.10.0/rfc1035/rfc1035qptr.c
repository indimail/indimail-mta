/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>
#include	<errno.h>


static int ptr(struct rfc1035_res *, const void *, int,
	       void (*)(const char *, void *),
	       void *);

static void save_name(const char *name, void *void_arg)
{
	strcpy((char *)void_arg, name);
}

int rfc1035_ptr(struct rfc1035_res *res, const RFC1035_ADDR *addr,
		  char *name)
{
	return rfc1035_ptr_x(res, addr, save_name, name);
}

/* Convenient function to do reverse IP lookup */
#if	RFC1035_IPV6

int rfc1035_ptr_x(struct rfc1035_res *res, const RFC1035_ADDR *addr,
		  void (*cb_func)(const char *, void *),
		  void *cb_arg)
{
struct	in_addr	in4;

	if (IN6_IS_ADDR_V4MAPPED(addr))
	{
		rfc1035_ipv6to4(&in4, addr);
		if (ptr(res, &in4, AF_INET, cb_func, cb_arg) == 0) return (0);
		return (-1);
	}
	return (ptr(res, addr, AF_INET6, cb_func,cb_arg));
}

#else
int rfc1035_ptr_x(struct rfc1035_res *res, const RFC1035_ADDR *a,
		  void (*cb_func)(const char *, void *),
		  void *cb_arg)
{
	return (ptr(res, a, AF_INET, cb_func, cb_arg));
}
#endif

static int ptr(struct rfc1035_res *res, const void *addr, int af,
	       void (*cb_func)(const char *, void *),
	       void *cb_arg)
{
struct	rfc1035_reply *reply;
int	n;
char	name[256], ptrbuf[256];

#if	RFC1035_IPV6

	if (af == AF_INET6)
	{
	const char *sin6=(const char *)addr;
	unsigned i;

		*name=0;
		for (i=sizeof(struct in6_addr); i; )
		{
		char	buf[10];

			--i;
			sprintf(buf, "%x.%x.",
				(int)(unsigned char)(sin6[i] & 0x0F),
				(int)(unsigned char)((sin6[i] >> 4) & 0x0F));
			strcat(name, buf);
		}
		strcat(name, "ip6.arpa");
	}
	else
#endif
	if (af != AF_INET)
	{
		errno=ENOENT;
		return (-1); /* hard error */
	}
	else
	{
	const char *p;
	unsigned char a=0,b=0,c=0,d=0;
	struct	in_addr	ia;

		memcpy(&ia, addr, sizeof(ia));
		rfc1035_ntoa_ipv4(&ia, name);
		p=name;

		while (*p >= '0' && *p <= '9')
			a= (int)a * 10 + (*p++ - '0');
		if (*p)	p++;
		while (*p >= '0' && *p <= '9')
			b= (int)b * 10 + (*p++ - '0');
		if (*p)	p++;
		while (*p >= '0' && *p <= '9')
			c= (int)c * 10 + (*p++ - '0');
		if (*p)	p++;
		while (*p >= '0' && *p <= '9')
			d= (int)d * 10 + (*p++ - '0');

		sprintf(name, "%d.%d.%d.%d.in-addr.arpa",
			(int)d, (int)c, (int)b, (int)a);
	}

	if (rfc1035_resolve_cname(res, name,
		RFC1035_TYPE_PTR, RFC1035_CLASS_IN, &reply, 0) < 0 ||
		reply == 0 ||
		(n=rfc1035_replysearch_an( res, reply, name, RFC1035_TYPE_PTR,
			RFC1035_CLASS_IN, 0)) < 0 ||
		rfc1035_replyhostname(reply, reply->allrrs[n]->rr.domainname,
			ptrbuf) == 0)
	{
		if (reply && reply->rcode != RFC1035_RCODE_NXDOMAIN &&
			reply->rcode != RFC1035_RCODE_NOERROR)
		{
			rfc1035_replyfree(reply);
			errno=EAGAIN;
			return (-1);
		}

		if (reply) rfc1035_replyfree(reply);
		errno=ENOENT;
		return (-1); /* hard error */
	}

	(*cb_func)(ptrbuf, cb_arg);

	while ((n=rfc1035_replysearch_an(res, reply, name,
					 RFC1035_TYPE_PTR,
					 RFC1035_CLASS_IN, n+1)) >= 0)
	{
		if (rfc1035_replyhostname(reply,
					  reply->allrrs[n]->rr.domainname,
					  ptrbuf))
			(*cb_func)(ptrbuf, cb_arg);
	}

	rfc1035_replyfree(reply);
	return (0);
}
