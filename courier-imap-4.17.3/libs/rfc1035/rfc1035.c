/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdio.h>
#include	"soxwrap/soxwrap.h"
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/types.h>
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif
#include	<arpa/inet.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"rfc1035.h"


#define ISSPACE(c) (strchr(" \t\r\n", (int)(unsigned char)(c)) != NULL)

#if RFC1035_IPV6

#else

struct in_addr	rfc1035_addr_any={INADDR_ANY};

#endif

void rfc1035_init_timeout(struct rfc1035_res *res, unsigned s, unsigned n)
{
	res->rfc1035_timeout_initial=s;
	res->rfc1035_timeout_backoff=n;
}

void rfc1035_init_ns(struct rfc1035_res *res, const RFC1035_ADDR *a, unsigned n)
{
unsigned i;
unsigned j;

	j=0;

	random128_binary(&res->randseed);
	md5_digest(&res->randseed, sizeof(res->randseed), res->randbuf);
	res->randptr=0;

	for (i=0; i == 0 || (i<n && i<RFC1035_MAXNS); i++)
	{
#if RFC1035_IPV6
	struct in6_addr sin;

		if (n == 0)
			sin=in6addr_loopback;
		else
			sin=a[(j+i)%n];

#else
	struct in_addr sin;

		if (n == 0)
		{
			rfc1035_aton("127.0.0.1", &sin);
		}
		else
			sin=a[(j+i)%n];
		memset(&res->nameservers[i], 0, sizeof(res->nameservers[i]));
		res->nameservers[i]=sin;
#endif
		res->nameservers[i]=sin;
	}
	res->rfc1035_nnameservers=i;

}

int rfc1035_init_defaultdomain(struct rfc1035_res *res, const char *p)
{
char	*q;

	if (res->rfc1035_defaultdomain)
		free(res->rfc1035_defaultdomain);

	if ((res->rfc1035_defaultdomain=malloc(strlen(p)+1)) == 0)
		return (-1);

	strcpy(res->rfc1035_defaultdomain, p);
	for (q=res->rfc1035_defaultdomain; *q; q++)
		if (ISSPACE(*q))
		{
			*q=0;
			break;
		}

	return (0);
}

void rfc1035_init_dnssec_enable(struct rfc1035_res *res, int flag)
{
	rfc1035_init_edns_payload(res, flag ? 1280:0);
}


void rfc1035_init_edns_payload(struct rfc1035_res *res, int payload_size)
{
	res->dnssec_payload_size=payload_size;
}

static char tl(char c)
{
	if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';

	return c;
}

void rfc1035_init_resolv(struct rfc1035_res *res)
{
FILE	*fp=fopen("/etc/resolv.conf", "r");
char rfc1035_buf[512];
RFC1035_ADDR ns[RFC1035_MAXNS];
int nns=0;

	memset(res, 0, sizeof(*res));

	while (fp && fgets(rfc1035_buf, sizeof(rfc1035_buf), fp))
	{
	char	*p;

		for (p=rfc1035_buf; *p; p++)
			*p=tl(*p);

		for (p=rfc1035_buf; *p; p++)
			if (ISSPACE(*p))	break;
		if (*p)	*p++=0;

		if (strcmp(rfc1035_buf, "domain") == 0)
		{
			while (p && ISSPACE(*p))
				++p;
			rfc1035_init_defaultdomain(res, p);
			continue;
		}

		if (strcmp(rfc1035_buf, "nameserver"))	continue;
		while (*p && ISSPACE(*p))	p++;
		if (nns < RFC1035_MAXNS)
		{
		char	*q;

			for (q=p; *q && !ISSPACE(*q); q++)
				;
			*q=0;

			if (rfc1035_aton(p, &ns[nns++]) < 0)
				--nns;
		}
	}
	if (fp) fclose(fp);

	rfc1035_init_ns(res, ns, nns);
}

void rfc1035_destroy_resolv(struct rfc1035_res *res)
{
	if (res->rfc1035_defaultdomain)
	{
		free(res->rfc1035_defaultdomain);
	}
}

/************/

struct compresslist {
	struct compresslist *next;
	unsigned offset;
	const char *ptr;
	} ;

static int mkpacketq(void (*)(const char *, unsigned, void *), void *,
		unsigned *,
		const struct rfc1035_query *,
		unsigned,
		const char *,
		struct compresslist *,
		struct rfc1035_res *);

unsigned rfc1035_next_randid(struct rfc1035_res *res)
{
	unsigned i;

	if (res->randptr >= sizeof(res->randbuf))
	{
		for (i=0; i<sizeof(res->randseed); i++)
			if ( ++((unsigned char *)res->randseed)[i])
				break;

		md5_digest(res->randseed, sizeof(res->randseed),
			   res->randbuf);
		res->randptr=0;
	}

	i= ((unsigned)((unsigned char *)res->randbuf)[res->randptr] << 8) |
		((unsigned char *)res->randbuf)[res->randptr+1];
	res->randptr += 2;
	return i;
}

int rfc1035_mkquery(struct rfc1035_res *res,	/* resolver */
			unsigned opcode,	/* opcode */
			const struct rfc1035_query *questions,
			unsigned nquestions,
			void (*func)(const char *, unsigned, void *), void *arg)
{
struct {
	unsigned char idhi, idlo;
	unsigned char infohi, infolo;
	unsigned char qhi, qlo;
	unsigned char ahi, alo;
	unsigned char nhi, nlo;
	unsigned char auhi, aulo;
	} header;
unsigned cnt;

	unsigned id=rfc1035_next_randid(res);

	header.idhi= id >> 8;
	header.idlo= id;

	header.infohi= (opcode << 3) & 0x78;

	if (!res->norecursive)
		header.infohi |= 1; /* Want a recursive query */
	header.infolo=0;
	header.qhi=nquestions >> 8;
	header.qlo=nquestions;
	header.ahi=0;
	header.alo=0;
	header.nhi=0;
	header.nlo=0;
	header.auhi=0;
	header.aulo=0;

	if (res->dnssec_payload_size)
		header.aulo=1;

	(*func)( (const char *)&header, sizeof(header), arg);
	cnt=sizeof(header);
	if (nquestions)
		if (mkpacketq(func, arg, &cnt, questions, nquestions,
			questions->name, 0, res))	return (-1);

	if (res->dnssec_payload_size)
	{
		/* RFC 2671, section 4.3 */

		struct {
			char opt_root_domain_name;
			char opt_type_hi;
			char opt_type_lo;
			char opt_class_hi;
			char opt_class_lo;
			char opt_extendedrcode;
			char opt_version;
			char opt_ttl_zhi;
			char opt_ttl_zlo;
			char opt_rdlen_hi;
			char opt_rdlen_lo;
		} rfc2671_43;

		memset(&rfc2671_43, 0, sizeof(rfc2671_43));

		rfc2671_43.opt_type_lo=RFC1035_TYPE_OPT;

		rfc2671_43.opt_class_hi= res->dnssec_payload_size >> 8;
		rfc2671_43.opt_class_lo= res->dnssec_payload_size;
		rfc2671_43.opt_ttl_zhi |= 0x80; /* RFC 3225 */

		(*func)((char *)&rfc2671_43, sizeof(rfc2671_43), arg);
	}

	return (0);
}

int rfc1035_hostnamecmp(const char *p, const char *q)
{
	while (*p || *q)
	{
		if (*p == '.' || *q == '.' )
		{
			if ( (*p && *p != '.') || (*q && *q != '.'))
				return (1);
			while (*p == '.')	++p;
			while (*q == '.')	++q;
			continue;
		}
		if (!*p || !*q)	return (1);
		if ( tl(*p) != tl(*q))	return (1);
		++p;
		++q;
	}
	return (0);
}

static struct compresslist *search(struct compresslist *cp, const char *name)
{
	for ( ; cp; cp=cp->next)
	{
		if (rfc1035_hostnamecmp(name, cp->ptr) == 0 &&
			(cp->offset & 0x3FFF) == cp->offset)
			return (cp);
			/* Packet compression uses the two high bits */
	}
	return (0);
}

static int mkpacketq_full(void (*)(const char *, unsigned, void *),
		void *,
		unsigned *,
		const struct rfc1035_query *,
		unsigned,
		const char *,
		struct compresslist *, struct rfc1035_res *);

static int mkpacketq(void (*func)(const char *, unsigned, void *), void *arg,
		unsigned *cnt,
		const struct rfc1035_query *qp,
		unsigned nqp,
		const char *nameptr,
		struct compresslist *comp_list,
		struct rfc1035_res *res)
{
char	*buf;
int	rc;


	if (!res->rfc1035_defaultdomain || strchr(nameptr, '.'))
		return (mkpacketq_full(func, arg, cnt, qp, nqp, nameptr,
			comp_list, res));

	/* Append default domain */

	if ((buf=malloc(strlen(nameptr)+
		strlen(res->rfc1035_defaultdomain)+2)) == 0)
		return (-1);

	strcat(strcat(strcpy(buf, nameptr), "."),
		res->rfc1035_defaultdomain);

	rc=mkpacketq_full(func, arg, cnt, qp, nqp, buf, comp_list, res);
	free(buf);
	return (rc);
}

static int mkpacketq_full(void (*func)(const char *, unsigned, void *),
		void *arg,
		unsigned *cnt,
		const struct rfc1035_query *qp,
		unsigned nqp,
		const char *nameptr,
		struct compresslist *comp_list,
		struct rfc1035_res *res)
{
unsigned llen;
struct	compresslist *cp;

	while (nameptr && *nameptr == '.')
		++nameptr;

	if (!nameptr || !*nameptr)
	{
	struct {
		unsigned char padtail;
		unsigned char qtypehi, qtypelo;
		unsigned char qclasshi, qclasslo;
		} qtail;

		qtail.padtail=0;
		qtail.qtypehi=qp->qtype >> 8;
		qtail.qtypelo=qp->qtype;
		qtail.qclasshi=qp->qclass >> 8;
		qtail.qclasslo=qp->qclass;

		(*func)((const char *)&qtail, sizeof(qtail), arg);
		++qp;
		--nqp;
		*cnt += sizeof(qtail);
		if (nqp)
			return (mkpacketq(func, arg, cnt,
				qp, nqp, qp->name, comp_list, res));
		return (0);
	}

	for (llen=0; nameptr[llen] && nameptr[llen] != '.'; llen++)
		;
	cp=search(comp_list, nameptr);
	if (cp)
	{
	struct {
		unsigned char ptrhi, ptrlo;
		unsigned char qtypehi, qtypelo;
		unsigned char qclasshi, qclasslo;
		} qtail;

		qtail.ptrhi= (cp->offset >> 8) | 0xC0;
		qtail.ptrlo= cp->offset;
		qtail.qtypehi=qp->qtype >> 8;
		qtail.qtypelo=qp->qtype;
		qtail.qclasshi=qp->qclass >> 8;
		qtail.qclasslo=qp->qclass;

		(*func)( (const char *)&qtail, sizeof(qtail), arg);
		++qp;
		--nqp;
		*cnt += sizeof(qtail);

		if (nqp)
			return (mkpacketq(func, arg, cnt,
				qp, nqp, qp->name, comp_list, res));
	}
	else
	{
	unsigned n=llen;
	unsigned char c;
	struct compresslist newc;

		if (n > 63)	return (-1);

		newc.next=comp_list;
		newc.offset= *cnt;
		newc.ptr=nameptr;

		c=(unsigned char)n;
		(*func)((const char *) &c, 1, arg);
		(*func)( nameptr, c, arg);
		*cnt += 1+c;
		return (mkpacketq_full(func, arg, cnt,
				qp, nqp, nameptr+llen, &newc, res));
	}
	return (0);
}

/*******************************************************/

int rfc1035_wait_reply(int fd, unsigned nsecs)
{
fd_set	fds;
struct	timeval tv;
int	n;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec=nsecs;
	tv.tv_usec=0;
	while ((n=sox_select(fd+1, &fds, 0, 0, &tv)) < 0)
	{
		if (errno != EINTR)
			break;
	}

	if (n > 0 && FD_ISSET(fd, &fds))
		return (0);
	errno=ETIMEDOUT;
	return (-1);
}

int rfc1035_wait_query(int fd, unsigned nsecs)
{
fd_set	fds;
struct	timeval tv;
int	n;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec=nsecs;
	tv.tv_usec=0;
	while ((n=sox_select(fd+1, 0, &fds, 0, &tv)) < 0)
	{
		if (errno != EINTR)
			break;
	}

	if (n > 0 && FD_ISSET(fd, &fds))
		return (0);
	errno=ETIMEDOUT;
	return (-1);
}
