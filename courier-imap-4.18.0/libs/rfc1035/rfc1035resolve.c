/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdio.h>
#include	"soxwrap/soxwrap.h"
#include	"rfc1035.h"
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>
#include	<string.h>


struct querybuf {
	char qbuf[512];
	unsigned qbuflen;
	} ;

static void putqbuf(const char *p, unsigned l, void *q)
{
struct querybuf *qp=(struct querybuf *)q;

	if (qp->qbuflen < sizeof(qp->qbuf) &&
			sizeof(qp->qbuf) - qp->qbuflen >= l)
		memcpy(qp->qbuf+qp->qbuflen, p, l);
	qp->qbuflen += l;
}

struct rfc1035_reply *rfc1035_resolve_multiple(
			struct rfc1035_res *res,
			int opcode,
			const struct rfc1035_query *queries,
			unsigned nqueries)
{
struct	querybuf qbuf;
int	udpfd;
int	attempt;
const RFC1035_ADDR *ns;
unsigned nscount;
unsigned current_timeout, timeout_backoff;
unsigned nbackoff, backoff_num;
int	af;
static const char fakereply[]={0, 0, 0, RFC1035_RCODE_SERVFAIL,
		0, 0,
		0, 0,
		0, 0,
		0, 0};

	nscount=res->rfc1035_nnameservers;
	ns=res->nameservers;

	if (res->rfc1035_good_ns >= nscount)
		res->rfc1035_good_ns=0;

	qbuf.qbuflen=0;
	if ( rfc1035_mkquery(res,
		opcode, queries, nqueries, &putqbuf, &qbuf))
	{
		errno=EINVAL;
		return (0);
	}

	/* Prepare the UDP socket */

	if ((udpfd=rfc1035_open_udp(&af)) < 0)	return (0);

	/* Keep trying until we get an answer from a nameserver */

	current_timeout=res->rfc1035_timeout_initial;
	nbackoff=res->rfc1035_timeout_backoff;
	if (!current_timeout)	current_timeout=RFC1035_DEFAULT_INITIAL_TIMEOUT;
	if (!nbackoff)	nbackoff=RFC1035_DEFAULT_MAXIMUM_BACKOFF;

	timeout_backoff=current_timeout;
    for (backoff_num=0; backoff_num < nbackoff; backoff_num++,
					current_timeout *= timeout_backoff)


	for ( attempt=0; attempt < nscount; ++attempt)
	{
	int	nbytes;
	char	*reply;
	struct	rfc1035_reply *rfcreply=0;

	const RFC1035_ADDR *sin=&ns[(res->rfc1035_good_ns+attempt) % nscount];
	int	sin_len=sizeof(*sin);

	int	dotcp=0, isaxfr=0;
	unsigned i;

		for (i=0; i<nqueries; i++)
			if (queries[i].qtype == RFC1035_TYPE_AXFR)
			{
				dotcp=1;
				isaxfr=1;
				break;
			}

		if (isaxfr && nqueries > 1)
			return (rfc1035_replyparse(fakereply,
				sizeof(fakereply)));

		if (!dotcp)
		{
		/* Send the query via UDP */
		RFC1035_NETADDR	addrbuf;
		const struct sockaddr *addrptr;
		int	addrptrlen;

			if (rfc1035_mkaddress(af, &addrbuf,
				sin, htons(53),
				&addrptr, &addrptrlen))
				continue;

			if ((reply=rfc1035_query_udp(res, udpfd, addrptr,
				addrptrlen, qbuf.qbuf, qbuf.qbuflen, &nbytes,
					current_timeout)) == 0)
				continue;

			res->rfc1035_good_ns= (res->rfc1035_good_ns + attempt) %
					nscount;

		/* Parse the reply */

			rfcreply=rfc1035_replyparse(reply, nbytes);
			if (!rfcreply)
			{
				free(reply);
				if (errno == ENOMEM)	break;
				continue;
			/* Bad response from the server, try the next one. */
			}
			rfcreply->mallocedbuf=reply;
		/*
		** If the reply came back with the truncated bit set,
		** retry the query via TCP.
		*/

			if (rfcreply->tc)
			{
				dotcp=1;
				rfc1035_replyfree(rfcreply);
			}
		}

		if (dotcp)
		{
		int	tcpfd;
		struct	rfc1035_reply *firstreply=0, *lastreply=0;

			if ((tcpfd=rfc1035_open_tcp(res, sin)) < 0)
				continue;	/*
						** Can't connect via TCP,
						** try the next server.
						*/

			reply=rfc1035_query_tcp(res, tcpfd, qbuf.qbuf,
				qbuf.qbuflen, &nbytes, current_timeout);

			if (!reply)
			{
				sox_close(tcpfd);
				continue;
			}

			res->rfc1035_good_ns= (res->rfc1035_good_ns
					+ attempt) % nscount;

			rfcreply=rfc1035_replyparse(reply, nbytes);
			if (!rfcreply)
			{
				free(reply);
				sox_close(tcpfd);
				continue;
			}
			rfcreply->mallocedbuf=reply;
			firstreply=lastreply=rfcreply;
			while (isaxfr && rfcreply->rcode == 0)
			{
				if ((reply=rfc1035_recv_tcp(res,
					tcpfd, &nbytes, current_timeout))==0)
					break;
				
				rfcreply=rfc1035_replyparse(reply, nbytes);
				if (!rfcreply)
				{
					free(reply);
					rfc1035_replyfree(firstreply);
					firstreply=0;
					break;
				}
				rfcreply->mallocedbuf=reply;
				lastreply->next=rfcreply;
				lastreply=rfcreply;

				if ( rfcreply->ancount &&
					rfcreply->anptr[0].rrtype ==
						RFC1035_TYPE_SOA)
					break;
			}
			sox_close(tcpfd);
			if (!firstreply)
				return (0);
			rfcreply=firstreply;
		}
		memcpy(&rfcreply->server_addr, sin, sin_len);
		sox_close(udpfd);
		return (rfcreply);
	}

	/*
	** Return a fake server failure reply, when we couldn't contact
	** any name server.
	*/
	sox_close(udpfd);
	return (rfc1035_replyparse(fakereply, sizeof(fakereply)));
}

struct rfc1035_reply *rfc1035_resolve(
	struct rfc1035_res *res,
	int opcode,
	const char *name,
	unsigned qtype,
	unsigned qclass)
{
struct rfc1035_query q;

	q.name=name;
	q.qtype=qtype;
	q.qclass=qclass;
	return (rfc1035_resolve_multiple(res, opcode, &q, 1));
}
