/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"rfc1035.h"
#include	"rfc1035mxlist.h"

void rfc1035_mxlist_free(struct rfc1035_mxlist *p)
{
struct rfc1035_mxlist *q;

	while (p)
	{
		q=p->next;
		if (p->hostname)	free(p->hostname);
		free(p);
		p=q;
	}
}

static int addrecord(struct rfc1035_mxlist **list, const char *mxname,
		     int mxpreference,

#if	RFC1035_IPV6
		     struct in6_addr *in,
#else
		     struct in_addr *in,
#endif
		     int ad,
		     int port)
{
#if	RFC1035_IPV6
struct sockaddr_in6 sin;
#else
struct sockaddr_in sin;
#endif
struct rfc1035_mxlist *p;

	if ((p=(struct rfc1035_mxlist *)malloc(sizeof(struct rfc1035_mxlist)))
		== 0 || (p->hostname=malloc(strlen(mxname)+1)) == 0)
	{
		if (p)	free ( (char *)p);
		return (-1);
	}

	memset(&sin, 0, sizeof(sin));

#if	RFC1035_IPV6
	sin.sin6_family=AF_INET6;
	sin.sin6_addr= *in;
	sin.sin6_port=htons(port);
	p->protocol=PF_INET6;
#else
	sin.sin_family=AF_INET;
	sin.sin_addr= *in;
	sin.sin_port=htons(port);
	p->protocol=PF_INET;
#endif

	while ( *list && (*list)->priority < mxpreference )
		list= &(*list)->next;

	p->next=*list;
	*list=p;
	p->ad=ad;
	p->priority=mxpreference;
	strcpy(p->hostname, mxname);
	memcpy(&p->address, &sin, sizeof(sin));
	return (0);
}

static int harvest_records(struct rfc1035_res *res,
	struct rfc1035_mxlist **list,
	struct rfc1035_reply *mxreply,
	int mxpreference,
	char *mxname,
	int q_type, int *found, int autoquery, int port);

#define	HARVEST_AUTOQUERY	1
#define	HARVEST_NODUPE		2

static int add_arecords(struct rfc1035_res *res, struct rfc1035_mxlist **list,
			struct rfc1035_reply *mxreply,
			int mxpreference,
			char *mxname, int port, int opts)
{
#if	RFC1035_IPV6
	struct in6_addr	in;
	int first_a=RFC1035_TYPE_A;
	int second_a=RFC1035_TYPE_AAAA;
	const char *prefer_ipv6=getenv("ESMTP_PREFER_IPV6_MX");
#else
	struct in_addr	in;
#endif
	int	found=0;
	int	rc;

	if (rfc1035_aton(mxname, &in) == 0)
	{	/* Broken MX record */
	char	buf[RFC1035_NTOABUFSIZE];

		rfc1035_ntoa(&in, buf);

		if (addrecord(list, buf, mxpreference, &in, 0, port))
			return (RFC1035_MX_INTERNAL);

		return (RFC1035_MX_OK);
	}

#if	RFC1035_IPV6

/*
	Here's the IPv6 strategy:

If we have an existing MX record to work with, try to harvest
both A and AAAA addresses from it.  If we find either an A or an AAAA
record, stop.

If we don't have an existing MX record, or we didn't find A or AAAA
records, then query for A records.  Query for AAAA records only if A
records weren't found.

*/
	if (prefer_ipv6 && *prefer_ipv6 && *prefer_ipv6 != '0')
	{
		first_a=RFC1035_TYPE_AAAA;
		second_a=RFC1035_TYPE_A;
	}

	if (mxreply && !(opts & RFC1035_MX_QUERYALL))
	{
		if ((rc=harvest_records(res, list, mxreply, mxpreference,
			mxname, second_a, &found, HARVEST_NODUPE,
				port))
		    != RFC1035_MX_OK)
			return (rc);

		if ((rc=harvest_records(res, list, mxreply, mxpreference,
			mxname, first_a, &found, 0, port))
				!= RFC1035_MX_OK)
			return (rc);

		if (found)	return (RFC1035_MX_OK);
	}

	if ((rc=harvest_records(res, list, mxreply, mxpreference, mxname,
		second_a, &found, HARVEST_AUTOQUERY|HARVEST_NODUPE, port))
						!= RFC1035_MX_OK)
		return (rc);

	if ((rc=harvest_records(res, list, mxreply, mxpreference, mxname,
		first_a, &found, HARVEST_AUTOQUERY, port))
			!= RFC1035_MX_OK)
		return (rc);

	if (found)	return (RFC1035_MX_OK);

#else
	if ((rc=harvest_records(res, list, mxreply, mxpreference, mxname,
		RFC1035_TYPE_A, &found, HARVEST_AUTOQUERY, port))
			!= RFC1035_MX_OK)
		return (rc);
#endif

	if (!found)	return (RFC1035_MX_HARDERR);
	return (RFC1035_MX_OK);
}

static int harvest_records(struct rfc1035_res *res,
	struct rfc1035_mxlist **list,
	struct rfc1035_reply *mxreply,
	int mxpreference,
	char *mxname,
	int q_type, int *found,
	int flags, int port)
{
	char	lookup_name[RFC1035_MAXNAMESIZE+1];

	struct rfc1035_reply *areply=0;
	int index;
#if	RFC1035_IPV6
	struct in6_addr in;
#else
	struct in_addr in;
#endif

	/*
	** Make a copy of mxname, because resolve_cname modifies it.
	** That is rather rude, since harvest_records gets called multiple
	** times.
	**
	** We still need to know what resolve_cname() did, since
	** after resolve_cname() we call replysearch_all(), which needs to
	** have the same hostname.
	**
	** mxname always points to a char[RFC1035_MAXNAMESIZE_1], so what's
	** good for the goose is good for the gander.
	*/

	strcpy(lookup_name, mxname);

	index= -1;

	if (!mxreply || (
		((index=rfc1035_replysearch_all( res, mxreply, lookup_name,
					q_type,
					RFC1035_CLASS_IN,
					0)) < 0 ||
			mxreply->allrrs[index]->rrtype != q_type)
		&& (flags & HARVEST_AUTOQUERY))
		)
	{
		index=rfc1035_resolve_cname(res, lookup_name,
			q_type,
			RFC1035_CLASS_IN, &areply, RFC1035_X_RANDOMIZE);
		if (index < 0)
		{
			if (!areply)
			{
				if (index == RFC1035_ERR_CNAME_RECURSIVE)
					return (RFC1035_MX_BADDNS);
				return (RFC1035_MX_INTERNAL);
			}

			if (areply->rcode == RFC1035_RCODE_NXDOMAIN ||
				areply->rcode == RFC1035_RCODE_NOERROR)
			{
				rfc1035_replyfree(areply);
				return (RFC1035_MX_OK);
			}
			rfc1035_replyfree(areply);
			return (RFC1035_MX_SOFTERR);
		}
		mxreply=areply;
	}

	for ( ; index >= 0 ;
			index=rfc1035_replysearch_all( res, mxreply,
						       lookup_name,
						       q_type,
						       RFC1035_CLASS_IN,
						       index+1))
	{
		if (mxreply->allrrs[index]->rrtype != q_type)
			continue;

#if RFC1035_IPV6
		if (q_type == RFC1035_TYPE_A)
		{
		struct rfc1035_mxlist *q;

			/* Map it to an IPv4 address */

			rfc1035_ipv4to6(&in,
				&mxreply->allrrs[index]->rr.inaddr);

			/* See if it's already here */

			for (q= *list; q; q=q->next)
			{
			struct sockaddr_in6 sin6;

				if (q->protocol != PF_INET6)
					continue;
				memcpy(&sin6, &q->address, sizeof(sin6));

				if (memcmp(&sin6.sin6_addr, &in, sizeof(in))
					== 0 && q->priority == mxpreference)
					break;
			}
			if ((flags & HARVEST_NODUPE) && q)	continue;
		}
		else
			in=mxreply->allrrs[index]->rr.in6addr;
#else
		in.s_addr=mxreply->allrrs[index]->rr.inaddr.s_addr;
#endif
		*found=1;
		if (addrecord(list, mxname, mxpreference, &in, mxreply->ad,
			      port))
		{
			if (areply)
				rfc1035_replyfree(areply);
			return (RFC1035_MX_INTERNAL);
		}
	}
	if (areply)
		rfc1035_replyfree(areply);

	return (RFC1035_MX_OK);
}

static int domxlistcreate(struct rfc1035_res *res,
			  const char *q_name, int opts,
			  struct rfc1035_mxlist **list, int port)
{
char	namebuf[RFC1035_MAXNAMESIZE+1];
struct	rfc1035_reply *replyp;
int	index;
RFC1035_ADDR	in;
int seen_softerr=0;
int seen_good=0;

	*list=0;
	if (rfc1035_aton(q_name, &in) == 0)
		return (RFC1035_MX_HARDERR);	/* Don't gimme an IP address */

	namebuf[0]=0;
	strncat(namebuf, q_name, RFC1035_MAXNAMESIZE);
	if (namebuf[0] == '[')
	{
	char	*q=strchr(namebuf, ']');

		if (!q || q[1])	return (RFC1035_MX_HARDERR);	/* Bad addr */
		*q=0;
		if (rfc1035_aton(namebuf+1, &in))
			return (RFC1035_MX_HARDERR);

		if (addrecord(list, q_name, -1, &in, 0, port))
			return (RFC1035_MX_INTERNAL);
		return (RFC1035_MX_OK);
	}

	index=rfc1035_resolve_cname(res, namebuf,
		RFC1035_TYPE_MX,
		RFC1035_CLASS_IN, &replyp, RFC1035_X_RANDOMIZE);

	if (index < 0)
	{
		if (!replyp)
		{
			if (index == RFC1035_ERR_CNAME_RECURSIVE)
				return (RFC1035_MX_BADDNS);
			return (RFC1035_MX_INTERNAL);
		}

		if (replyp->rcode == RFC1035_RCODE_NXDOMAIN ||
			replyp->rcode == RFC1035_RCODE_NOERROR)
		{
			rfc1035_replyfree(replyp);
			strcpy(namebuf, q_name);

			if (opts & RFC1035_MX_AFALLBACK)
				return (add_arecords(res, list, 0, -1,
						     namebuf, port, opts));
			return RFC1035_MX_HARDERR;
		}

		rfc1035_replyfree(replyp);
		return (RFC1035_MX_SOFTERR);
	}

	for ( ; index >= 0;
			index=rfc1035_replysearch_all( res, replyp, namebuf,
					RFC1035_TYPE_MX,
					RFC1035_CLASS_IN,
					index+1))
	{
	char	mxname[RFC1035_MAXNAMESIZE+1];

		if (replyp->allrrs[index]->rrtype != RFC1035_TYPE_MX)
			continue;

		if (rfc1035_replyhostname(replyp,
			replyp->allrrs[index]->rr.mx.mx_label, mxname) == 0)
			continue;

		switch (add_arecords(res, list, replyp,
				     replyp->allrrs[index]->rr.mx.preference,
				     mxname,
				     port, opts)) {
		case	RFC1035_MX_SOFTERR:
			seen_softerr=1;
			continue;
		case	RFC1035_MX_INTERNAL:
			rfc1035_replyfree(replyp);
			return (RFC1035_MX_INTERNAL);
		case	RFC1035_MX_BADDNS:
			rfc1035_replyfree(replyp);
			return (RFC1035_MX_BADDNS);
		default:
			seen_good=1;
			continue;
		}
	}

	rfc1035_replyfree(replyp);

	if (seen_good && (opts & RFC1035_MX_IGNORESOFTERR))
		seen_softerr=0;
		/* At least some A records were probably fetched */

	if (seen_softerr)
		return (RFC1035_MX_SOFTERR);

	if (*list)	return (RFC1035_MX_OK);
	return (RFC1035_MX_HARDERR);
}

static int domxlistcreate2(struct rfc1035_res *res,
			   const char *q_name,
			   int opts,
			   struct rfc1035_mxlist **list,
			   int port)
{
char	*buf;
int	rc;

	if (strchr(q_name, '.') || strchr(q_name, ':') ||
		!res->rfc1035_defaultdomain)
		return (domxlistcreate(res, q_name, opts, list, port));

	if ((buf=malloc(strlen(q_name)+
		strlen(res->rfc1035_defaultdomain)+2)) == 0)
		return (-1);

	strcat(strcat(strcpy(buf, q_name), "."), res->rfc1035_defaultdomain);

	rc=domxlistcreate(res, buf, opts, list, port);

	free(buf);
	return (rc);
}

static int domxlistcreate3(struct rfc1035_res *res,
			   const char *q_name,
			   int opts,
			   struct rfc1035_mxlist **list)
{
char	*buf;
int	rc;
const char *p;

	p=strchr(q_name, ',');

	if (p == 0)	return (domxlistcreate2(res, q_name, opts, list, 25));

	if ((buf=malloc(p-q_name+1)) == 0)
		return (-1);

	memcpy(buf, q_name, p-q_name);
	buf[p-q_name]=0;

	rc=domxlistcreate2(res, buf, opts, list, atoi(p+1));

	free(buf);
	return (rc);
}

int rfc1035_mxlist_create_x(struct rfc1035_res *res,
			    const char *q_name,
			    int opts,
			    struct rfc1035_mxlist **list)
{
int	rc=domxlistcreate3(res, q_name, opts, list);

	if (rc != RFC1035_MX_OK)
	{
		rfc1035_mxlist_free(*list);
		*list=0;
	}
	return (rc);
}

int rfc1035_mxlist_create(struct rfc1035_res *res,
			  const char *q_name,
			  struct rfc1035_mxlist **list)
{
	return rfc1035_mxlist_create_x(res, q_name,
				       RFC1035_MX_AFALLBACK |
				       RFC1035_MX_IGNORESOFTERR,
				       list);
}
