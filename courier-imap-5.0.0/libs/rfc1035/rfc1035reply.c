/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>


void rfc1035_replyfree(struct rfc1035_reply *p)
{
struct	rfc1035_reply	*q;

	while (p)
	{
		q=p->next;
		if (p->mallocedbuf)	free(p->mallocedbuf);
		if (p->qdptr)	free(p->qdptr);
		if (p->anptr)	free(p->anptr);
		if (p->nsptr)	free(p->nsptr);
		if (p->arptr)	free(p->arptr);
		if (p->allrrs)	free(p->allrrs);
		free(p);
		p=q;
	}
}

const char *rfc1035_replyuncompress(const char **ptr,
	const struct rfc1035_reply *reply, char *namebuf)
{
unsigned i=0;
unsigned l;
const char *end=reply->reply+reply->replylen;
const char *newptr=0;
int	cnt=0;

	for (;;)
	{
		if ( (*ptr) == end)	return (0);
		l=(unsigned char)*(*ptr)++;
		if (l > 63)
		{
			if ((l & 0xC0) != 0xC0)	return (0);
			if ( (*ptr) == end)	return (0);
			l= ((l & 63) << 8) + (unsigned char)*(*ptr)++;
			if (l >= reply->replylen)	return (0);
			ptr= &newptr;
			newptr=reply->reply+l;
			if ( ++cnt > 100)	return (0);
						/* Detect loops */

			continue;
		}
		if (l == 0)	break;
		if (i)
		{
			if (i >= RFC1035_MAXNAMESIZE)	return (0);
			if (namebuf) namebuf[i]='.';
			++i;
		}
		if (RFC1035_MAXNAMESIZE-i < l)	return (0);

		if (reply->reply + reply->replylen - (*ptr) < l)
			return (0);

		if (namebuf)
			memcpy(namebuf+i, *ptr, l);
		i += l;
		*ptr += l;
	}

	if (namebuf)	namebuf[i]=0;
	return (namebuf ? namebuf:"");
}

const char *rfc1035_replyhostname( const struct rfc1035_reply *r, const char *n,
	char *namebuf)
{
const char *p=rfc1035_replyuncompress(&n, r, namebuf);

	if (!p)	strcpy(namebuf, "error");
	return (p);
}

static void doparse(struct rfc1035_reply *, struct rfc1035_rr *);

struct rfc1035_reply *rfc1035_replyparse(const char *p, unsigned l)
{
struct rfc1035_reply *r=(struct rfc1035_reply *)
	malloc(sizeof(struct rfc1035_reply));
char	c;
unsigned n;
int	pass;
unsigned *lenptrs[3];
struct rfc1035_rr *rrptrs[3];
const char *end=p+l;
unsigned allcnt;

	if (!r)	return (0);
	r->next=0;
	r->reply=p;
	r->replylen=l;
	r->mallocedbuf=0;
	memset(&r->server_addr, 0, sizeof(r->server_addr));
	r->qdptr=0;
	r->anptr=0;
	r->nsptr=0;
	r->arptr=0;
	r->allrrs=0;

	if (l < 12)
	{
		errno=EINVAL;
		rfc1035_replyfree(r);
		return (0);
	}

	c=p[2];
	r->rd= c & 1;
	c >>= 1;
	r->tc= c & 1;
	c >>= 1;
	r->aa= c & 1;
	c >>= 1;
	r->opcode= c & 15;
	c >>= 4;
	r->qr= c & 1;

	r->ra=(p[3] >> 7) & 1;
	r->ad=(p[3] >> 5) & 1;
	r->cd=(p[3] >> 4) & 1;
	r->rcode=p[3] & 15;

	r->qdcount= ((unsigned)(unsigned char)p[4] << 8) | (unsigned char)p[5];
	r->ancount= ((unsigned)(unsigned char)p[6] << 8) | (unsigned char)p[7];
	r->nscount= ((unsigned)(unsigned char)p[8] << 8) | (unsigned char)p[9];
	r->arcount= ((unsigned)(unsigned char)p[10]<< 8) | (unsigned char)p[11];

	if (r->qdcount >= l || r->ancount >= l || r->nscount >= l ||
		r->arcount >= l ||
		(n=r->qdcount + r->ancount) >= l ||
		(n += r->nscount) >= l ||
		n + r->arcount >= l)
	{
		errno=EINVAL;
		rfc1035_replyfree(r);
		return (0);
	}

	if ((r->qdcount && (r->qdptr=malloc(sizeof(struct rfc1035_query)
				* r->qdcount)) == 0) ||
		(r->ancount && (r->anptr=malloc(sizeof(struct rfc1035_rr)
				* r->ancount)) == 0) ||
		(r->nscount && (r->nsptr=malloc(sizeof(struct rfc1035_rr)
				* r->nscount)) == 0) ||
		(r->arcount && (r->arptr=malloc(sizeof(struct rfc1035_rr)
				* r->arcount)) == 0) ||
		(r->ancount + r->nscount + r->arcount &&
			(r->allrrs=malloc(sizeof(*r->allrrs)*(
				r->ancount + r->nscount + r->arcount))) == 0)
		)
	{
		rfc1035_replyfree(r);
		return (0);
	}

	p += 12;
	l -= 12;
	for (n=0; n<r->qdcount; n++)
	{
		r->qdptr[n].name=p;
		if (rfc1035_replyuncompress( &p, r, 0 ) == 0)
		{
			errno=EINVAL;
			rfc1035_replyfree(r);
			return (0);
		}

		if (p > end-4)	return (0);

		r->qdptr[n].qtype=((unsigned)(unsigned char)p[0] << 8)
				| (unsigned char)p[1];
		p += 2;
		r->qdptr[n].qclass=((unsigned)(unsigned char)p[0] << 8)
				| (unsigned char)p[1];
		p += 2;
	}

	lenptrs[0]= &r->ancount;
	lenptrs[1]= &r->nscount;
	lenptrs[2]= &r->arcount;
	rrptrs[0]= r->anptr;
	rrptrs[1]= r->nsptr;
	rrptrs[2]= r->arptr;

	allcnt=0;

	for (pass=0; pass<3; pass++)
	{
	struct rfc1035_rr *rrp= rrptrs[pass];

		for (n=0; n< *lenptrs[pass]; n++)
		{
			r->allrrs[allcnt++]=rrp;
			rrp->rrname=p;
			if (rfc1035_replyuncompress( &p, r, 0 )
				== 0)
			{
				errno=EINVAL;
				rfc1035_replyfree(r);
				return (0);
			}

			if (p > end-10)	return (0);
			rrp->rrtype=((unsigned)(unsigned char)p[0] << 8)
				| (unsigned char)p[1];
			p += 2;
			rrp->rrclass=((unsigned)(unsigned char)p[0] << 8)
				| (unsigned char)p[1];
			p += 2;
			rrp->ttl=((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
			p += 4;

			rrp->rdlength=((unsigned)(unsigned char)p[0] << 8)
				| (unsigned char)p[1];
			p += 2;
			rrp->rdata= rrp->rdlength ? p:0;
			if (end - p < rrp->rdlength)
			{
				errno=EINVAL;
				rfc1035_replyfree(r);
				return (0);
			}
			p += rrp->rdlength;
			doparse(r, rrp);
			++rrp;
		}
	}
	return (r);
}

static void doparse(struct rfc1035_reply *r, struct rfc1035_rr *rr)
{
const char *p;
static const char error[]="\005error";

	memset(&rr->rr, '\0', sizeof(rr->rr));
	if (rr->rrclass != RFC1035_CLASS_IN)	return;

	switch (rr->rrtype)	{
	case RFC1035_TYPE_A:

		if (rr->rdlength < 4)
			rr->rr.inaddr.s_addr=0;
		else
			memcpy(&rr->rr.inaddr.s_addr,
				rr->rdata, 4);
		break;
#if	RFC1035_IPV6
	case RFC1035_TYPE_AAAA:
		memset(&rr->rr.in6addr, 0, sizeof(rr->rr.in6addr));
		if (rr->rdlength >= sizeof(struct in6_addr))
			memcpy(&rr->rr.in6addr, rr->rdata,
				sizeof(rr->rr.in6addr));
		break;
#endif
	case RFC1035_TYPE_CNAME:
	case RFC1035_TYPE_MB:
	case RFC1035_TYPE_MG:
	case RFC1035_TYPE_MR:
	case RFC1035_TYPE_MD:
	case RFC1035_TYPE_MF:
	case RFC1035_TYPE_NS:
	case RFC1035_TYPE_PTR:
		rr->rr.domainname=p=rr->rdata;
		if (rfc1035_replyuncompress(&p, r, 0) == 0 ||
			p > rr->rdata + rr->rdlength)
			rr->rr.domainname=error;
		break;

	case RFC1035_TYPE_SOA:
		rr->rr.soa.mname_label=p=rr->rdata;

		if (rr->rdlength < 20 ||
			rfc1035_replyuncompress(&p, r, 0) == 0 ||
				p > rr->rdata + rr->rdlength ||
			(rr->rr.soa.rname_label=p, rfc1035_replyuncompress(&p,
				r, 0)) == 0 ||
				p > rr->rdata + rr->rdlength - 20)
		{
			rr->rr.soa.mname_label=error;
			rr->rr.soa.rname_label=error;
			rr->rr.soa.serial=0;
			rr->rr.soa.refresh=0;
			rr->rr.soa.retry=0;
			rr->rr.soa.expire=0;
			rr->rr.soa.minimum=0;
		}
		else
		{
			rr->rr.soa.serial=
				((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
			p += 4;
			rr->rr.soa.refresh=
				((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
			p += 4;
			rr->rr.soa.retry=
				((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
			p += 4;
			rr->rr.soa.expire=
				((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
			p += 4;
			rr->rr.soa.minimum=
				((RFC1035_UINT32)(unsigned char)p[0] << 24) |
				((RFC1035_UINT32)(unsigned char)p[1] << 16) |
				((RFC1035_UINT32)(unsigned char)p[2] << 8) |
				(unsigned char)p[3];
		}
		break;
	case RFC1035_TYPE_MX:
		p=rr->rdata;
		if (rr->rdlength < 2)
		{
			rr->rr.mx.preference=0;
			rr->rr.mx.mx_label=error;
		}
		else
		{
			rr->rr.mx.preference=
				((unsigned)(unsigned char)p[0] << 8) |
				(unsigned char)p[1];
			p += 2;
			rr->rr.mx.mx_label=p;
			if (rfc1035_replyuncompress(&p, r, 0) == 0
				|| p > rr->rdata + rr->rdlength)
			{
				rr->rr.mx.preference=0;
				rr->rr.mx.mx_label=error;
			}
		}
		break;

	case RFC1035_TYPE_HINFO:
		p=rr->rdata;
		rr->rr.hinfo.hinfo_str=error;
		rr->rr.hinfo.os_str=error;
		if (rr->rdlength && (unsigned char)*p < rr->rdlength)
		{
		const char *q=p+1+(unsigned char)*p;

			if (q < rr->rdata + rr->rdlength &&
				q+(unsigned char)*q < rr->rdata + rr->rdlength)
			{
				rr->rr.hinfo.hinfo_str=p;
				rr->rr.hinfo.os_str=q;
			}
		}
		break;

	case RFC1035_TYPE_MINFO:
		rr->rr.minfo.rmailbx_label=p=rr->rdata;
		if (rfc1035_replyuncompress(&p, r, 0) == 0 ||
			p > rr->rdata + rr->rdlength ||
			(rr->rr.minfo.emailbx_label=p,
				rfc1035_replyuncompress(&p, r, 0)) == 0 ||
				p > rr->rdata + rr->rdlength)
		{
			rr->rr.minfo.rmailbx_label=error;
			rr->rr.minfo.emailbx_label=error;
		}
		break;
	case RFC1035_TYPE_RRSIG:

		p=rr->rdata;

		if (rr->rdlength < 18 ||

		    ((rr->rr.rrsig.type_covered=
		      ((unsigned)(unsigned char)p[0] << 8)
		      | (unsigned char)p[1]),

		     (rr->rr.rrsig.algorithm=p[2]),
		     (rr->rr.rrsig.labels=p[3]),
		     (rr->rr.rrsig.original_ttl=
		      ((RFC1035_UINT32)(unsigned char)p[4] << 24) |
		      ((RFC1035_UINT32)(unsigned char)p[5] << 16) |
		      ((RFC1035_UINT32)(unsigned char)p[6] << 8) |
		      (unsigned char)p[7]),
		     (rr->rr.rrsig.signature_expiration=
		      ((RFC1035_UINT32)(unsigned char)p[8] << 24) |
		      ((RFC1035_UINT32)(unsigned char)p[9] << 16) |
		      ((RFC1035_UINT32)(unsigned char)p[10] << 8) |
		      (unsigned char)p[11]),
		     (rr->rr.rrsig.signature_inception=
		      ((RFC1035_UINT32)(unsigned char)p[12] << 24) |
		      ((RFC1035_UINT32)(unsigned char)p[13] << 16) |
		      ((RFC1035_UINT32)(unsigned char)p[14] << 8) |
		      (unsigned char)p[15]),
 		     (rr->rr.rrsig.key_tag=
		      ((RFC1035_UINT16)(unsigned char)p[16] << 8) |
		      (unsigned char)p[17]),
		     (rr->rr.rrsig.signer_name=(p += 18)),

		     rfc1035_replyuncompress(&p, r, 0) == 0) ||
		    p > rr->rdata + rr->rdlength)
		{
			memset(&rr->rr.rrsig, 0, sizeof(rr->rr.rrsig));
			rr->rr.rrsig.signer_name=error;
			break;
		}

		rr->rr.rrsig.signature=p;
		rr->rr.rrsig.signature_len=rr->rdata + rr->rdlength - p;
		break;

	default:
		break;
	}
}

/*
** Randomize order of resource records.
*/

static void rr_random(struct rfc1035_rr *p, unsigned n)
{
	unsigned i;

	for (i=0; i<n; i++)
	{
		size_t j=rand() % n;

		{
			struct rfc1035_rr buf=p[j];

			p[j]=p[i];
			p[i]=buf;
		}
	}
}

void rfc1035_rr_rand_an(struct rfc1035_reply *rr)
{
	rr_random(rr->anptr, rr->ancount);
}

void rfc1035_rr_rand_ns(struct rfc1035_reply *rr)
{
	rr_random(rr->nsptr, rr->nscount);
}

void rfc1035_rr_rand_ar(struct rfc1035_reply *rr)
{
	rr_random(rr->arptr, rr->arcount);
}

void rfc1035_rr_rand(struct rfc1035_reply *rr)
{
	rfc1035_rr_rand_an(rr);
	rfc1035_rr_rand_ns(rr);
	rfc1035_rr_rand_ar(rr);
}
