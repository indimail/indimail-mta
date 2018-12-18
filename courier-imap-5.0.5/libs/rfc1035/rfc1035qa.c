/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<errno.h>
#include	<string.h>
#include	<stdlib.h>
#include	<arpa/inet.h>

/* Convenient function to do forward IP lookup */

#if	RFC1035_IPV6

static int rfc1035_a_ipv4(struct rfc1035_res *res,
	const char *name, struct in_addr **iaptr, unsigned *iasize)
#else
int rfc1035_a(struct rfc1035_res *res,
	const char *name, RFC1035_ADDR **iaptr, unsigned *iasize)
#endif
{
struct	rfc1035_reply *reply;
int	n, o;
char	namebuf[RFC1035_MAXNAMESIZE+1];

	namebuf[0]=0;
	strncat(namebuf, name, RFC1035_MAXNAMESIZE);

	*iasize=0;
	if (rfc1035_resolve_cname(res, namebuf,
		RFC1035_TYPE_A, RFC1035_CLASS_IN, &reply, 0) < 0 ||
		reply == 0 ||
		(n=rfc1035_replysearch_an( res, reply, namebuf, RFC1035_TYPE_A,
			RFC1035_CLASS_IN, 0)) < 0)
	{
		if (reply && reply->rcode != RFC1035_RCODE_NXDOMAIN &&
			reply->rcode != RFC1035_RCODE_NOERROR)
		{
			errno=EAGAIN;
			rfc1035_replyfree(reply);
			return (1);	/* soft error */
		}

		if (reply) rfc1035_replyfree(reply);
		errno=ENOENT;
		return (-1); /* hard error */
	}

	for (o=n; o >= 0; o=rfc1035_replysearch_an(res, reply, namebuf,
			RFC1035_TYPE_A, RFC1035_CLASS_IN, o+1))
		++*iasize;

	if ( *iasize == 0 )
	{
		errno=EAGAIN;
		rfc1035_replyfree(reply);
		return (-1);
	}

	if ( (*iaptr=(struct in_addr *)malloc(sizeof(**iaptr)* *iasize)) == 0)
	{
		rfc1035_replyfree(reply);
		return (-1);
	}

	for (*iasize=0; n >= 0; n=rfc1035_replysearch_an(res, reply, namebuf,
			RFC1035_TYPE_A, RFC1035_CLASS_IN, n+1))
	{
		(*iaptr)[*iasize]= reply->allrrs[n]->rr.inaddr;
		++*iasize;
	}

	rfc1035_replyfree(reply);
	return (0);
}

#if	RFC1035_IPV6

/*
**	The IPv6 version issues two queries - for both A and AAAA records,
**	then maps any A record to IPv6.
**
**	If we get back both an AAAA for the IPv4-mapped address, and the
**	A record itself, ignore the dupe.
*/

static int we_have_that_ipv4(struct in6_addr in6,
	const struct	in_addr	*ia4ptr,
	unsigned	ia4len)
{
char	buf[INET6_ADDRSTRLEN];
const char *p;
struct in_addr in4;
unsigned i;

	if (!IN6_IS_ADDR_V4MAPPED((&in6))) return (0);	/* Not an IPv4 addy */

	if (inet_ntop(AF_INET6, &in6, buf, sizeof(buf)) == 0)
		return (0);	/* WTF??? */

	if ((p=strrchr(buf, ':')) != 0)
		++p;
	else
		p=buf;

	rfc1035_aton_ipv4(p, &in4);

	for (i=0; i<ia4len; i++)
		if (ia4ptr[i].s_addr == in4.s_addr)	return (1);
	return (0);
}

int rfc1035_a(struct rfc1035_res *res,
	const char *name, struct in6_addr **iaptr, unsigned *iasize)
{
struct	rfc1035_reply *reply;
int	n, o;
char	namebuf[RFC1035_MAXNAMESIZE+1];
int	enotfound=0;

struct	in_addr	*ia4ptr;
unsigned	ia4len;
unsigned k;

	n=rfc1035_a_ipv4(res, name, &ia4ptr, &ia4len);

	if (n > 0) return (n);
	if (n < 0)
	{
		if (errno != ENOENT)	return (n);
		ia4len=0;
		ia4ptr=0;
		enotfound=1;
	}

	namebuf[0]=0;
	strncat(namebuf, name, RFC1035_MAXNAMESIZE);

	*iasize=ia4len;
	reply=0;

	/*
	** Resist the temptation to stick in "ia4len > 0 &&", below.  Why?
	** A) We get a connection from an IPv6 address.
	** B) Spam check: the IP address must resolve backwards and forwards.
	** C) There are IPv4 records for the same hostname as well.
	** D) This sux.
	*/

	if (rfc1035_resolve_cname(res, namebuf,
		RFC1035_TYPE_AAAA, RFC1035_CLASS_IN, &reply, 0) < 0 ||
		reply == 0 ||
		(n=rfc1035_replysearch_an( res, reply, namebuf,
					   RFC1035_TYPE_AAAA,
					   RFC1035_CLASS_IN, 0)) < 0)
	{
		if (reply && reply->rcode != RFC1035_RCODE_NXDOMAIN &&
			reply->rcode != RFC1035_RCODE_NOERROR &&
			*iasize == 0)
			/* Unfortunately this is necessary.  Some swervers
			** return a TEMPFAIL for AAAA queries.
			*/
		{
			errno=EAGAIN;
			rfc1035_replyfree(reply);
			if (ia4len)
				free(ia4ptr);
			return (1);	/* soft error */
		}

		if (reply) rfc1035_replyfree(reply);
		if (enotfound)
			return (-1);
		enotfound=1;
		reply=0;
		n= -1;
	}
	else
	{
		for (o=n; o >= 0; o=rfc1035_replysearch_an(res, reply, namebuf,
				RFC1035_TYPE_AAAA, RFC1035_CLASS_IN, o+1))
		{
			if (we_have_that_ipv4(reply->allrrs[n]->rr.in6addr,
				ia4ptr, ia4len))
				continue;

			++*iasize;
		}
	}

	if ( *iasize == 0 && enotfound)
	{
		if (ia4len)
			free(ia4ptr);
		errno=ENOENT;
		return (-1);
	}

	if ( *iasize == 0 )
	{
		errno=EAGAIN;
		rfc1035_replyfree(reply);
		return (-1);
	}

	if ( (*iaptr=(struct in6_addr *)malloc(sizeof(**iaptr)* *iasize)) == 0)
	{
		rfc1035_replyfree(reply);
		return (-1);
	}

	for (*iasize=0; n >= 0; n=rfc1035_replysearch_an(res, reply, namebuf,
			RFC1035_TYPE_AAAA, RFC1035_CLASS_IN, n+1))
	{
		if (we_have_that_ipv4(reply->allrrs[n]->rr.in6addr,
			ia4ptr, ia4len))
			continue;
		(*iaptr)[*iasize]= reply->allrrs[n]->rr.in6addr;
		++*iasize;
	}

	for (k=0; k<ia4len; k++)
	{
	char	buf[INET6_ADDRSTRLEN];

		strcpy(buf, "::ffff:");
		rfc1035_ntoa_ipv4( &ia4ptr[k], buf+sizeof("::ffff:")-1);
		if (inet_pton( AF_INET6, buf, (*iaptr)+ *iasize) <= 0)
			memset( (*iaptr)+ *iasize, 0, sizeof(**iaptr));
		++*iasize;
	}
	if (ia4len)
		free (ia4ptr);
	rfc1035_replyfree(reply);
	return (0);
}
#endif
