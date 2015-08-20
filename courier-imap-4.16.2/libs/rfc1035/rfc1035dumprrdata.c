/*
** Copyright 1998 - 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	"rfc822/encode.h"
#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<arpa/inet.h>
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


static char *dumpsoa(struct rfc1035_reply *r, struct rfc1035_rr *rr)
{
char	name1[RFC1035_MAXNAMESIZE+1], name2[RFC1035_MAXNAMESIZE+1];
char	*p;
char	timebuf[RFC1035_MAXTIMEBUFSIZE+1];

	rfc1035_replyhostname(r, rr->rr.soa.rname_label, name1);
	rfc1035_replyhostname(r, rr->rr.soa.mname_label, name2);

	p=(char *)malloc(strlen(name1)+strlen(name2)+500);
	if (!p)	return (0);

	strcat(strcat(strcat(strcpy(p, name2), ". "), name1), ". (\n\t\t\t\t");
	sprintf(p+strlen(p), "%lu ; serial\n", 
			(unsigned long)rr->rr.soa.serial);
	sprintf(p+strlen(p), "\t\t\t\t%s  ; refresh\n", 
			rfc1035_fmttime( rr->rr.soa.refresh, timebuf));
	sprintf(p+strlen(p), "\t\t\t\t%s  ; retry\n", 
			rfc1035_fmttime( rr->rr.soa.retry, timebuf));
	sprintf(p+strlen(p), "\t\t\t\t%s  ; expire\n", 
			rfc1035_fmttime( rr->rr.soa.expire, timebuf));
	sprintf(p+strlen(p), "\t\t\t\t%s) ; minimum", 
			rfc1035_fmttime( rr->rr.soa.minimum, timebuf));
	return (p);
}

static void str_cat(char *p, const char **q)
{
int	l=(int)(unsigned char)*(*q)++;

	while (*p)	p++;
	memcpy(p, *q, l);
	p[l]=0;
	*q += l;
}

static char *dumptxt(struct rfc1035_reply *r, struct rfc1035_rr *rr)
{
int	len=1;
char	*p=0;
int	pass;
const char *cp;

	for (pass=0; pass<2; pass++)
	{
		if (pass && (p=(char *)malloc(len)) == 0)	return (0);
		if (pass)	*p=0;

		cp=rr->rdata;
		while (cp < rr->rdata+rr->rdlength)
		{
		int l=(int)(unsigned char)*cp;

			if (l >= rr->rdata+rr->rdlength-cp)	return (0);
			if (pass == 0)
				cp += l+1;
			len += l+4;
			if (pass && *p)
				strcat(p, "\n\t\t\t");
			if (pass)
				str_cat(p, &cp);
		}
	}
	return (p);
}

struct fmt_rrsig_info {

	char *buf;
	size_t n;
};


static void append_str(struct fmt_rrsig_info *info, const char *str, size_t l)
{
	if (info->buf)
	{
		memcpy(info->buf, str, l);

		info->buf += l;
	}

	info->n += l;
}

static void tostr_callback(const char *str, void *vp)
{
	append_str( (struct fmt_rrsig_info *)vp, str, strlen(str));
}

static void append_int32(struct fmt_rrsig_info *info, RFC1035_UINT32 val)
{
	char bufp[30];
	char *q=bufp+sizeof(bufp);

	do
	{
		*--q = '0' + (val % 10);

		val /= 10;
	} while (val);

	append_str(info, q, bufp+sizeof(bufp)-q);
}

static void append_time_t(struct fmt_rrsig_info *info, time_t timeval)
{
	char bufp[30];
	struct tm result;

	gmtime_r(&timeval, &result);

	strftime(bufp, sizeof(bufp), "%Y%m%d%H%M%S", &result);

	append_str(info, bufp, strlen(bufp));
}

static int encode_callback_func(const char *p, size_t n,
				void *vp)
{
	struct fmt_rrsig_info *info=(struct fmt_rrsig_info *)vp;

	while (n)
	{
		size_t i;

		if (*p == '\n')
		{
			append_str(info, " ", 1);
			++p;
			--n;
			continue;
		}

		for (i=0; i<n; ++i)
			if (p[i] == '\n')
				break;

		append_str(info, p, i);

		p += i;
		n -= i;
	}
	return 0;

}

static size_t fmt_rrsig(struct rfc1035_reply *r,
			struct rfc1035_rr *rr, time_t now, char *buf)
{
	char	p[RFC1035_MAXNAMESIZE+1];

	char	timebuf[RFC1035_MAXTIMEBUFSIZE+1];

	time_t signature_inception, signature_expiration;

	struct libmail_encode_info lei;

	struct fmt_rrsig_info fri;

	fri.buf=buf;
	fri.n=0;

	rfc1035_type_itostr(rr->rr.rrsig.type_covered, tostr_callback, &fri);

	append_str(&fri, " ", 1);

	append_int32(&fri, rr->rr.rrsig.algorithm);

	append_str(&fri, " ", 1);

	append_int32(&fri, rr->rr.rrsig.labels);

	append_str(&fri, " ", 1);

	rfc1035_fmttime(rr->rr.rrsig.original_ttl, timebuf);

	append_str(&fri, timebuf, strlen(timebuf));

	append_str(&fri, " ", 1);

	if (sizeof(time_t) == 4)
	{
		signature_inception=rr->rr.rrsig.signature_inception;
		signature_expiration=rr->rr.rrsig.signature_expiration;
	}
	else
	{
		time_t now_epoch=(now & ~0xFFFFFFFFLL);

		time_t cur_epoch=now_epoch | rr->rr.rrsig.signature_inception;

		time_t prev_epoch=cur_epoch - 0xFFFFFFFF-1;
		time_t next_epoch=cur_epoch + 0xFFFFFFFF+1;


#define time2diff(a,b) ((a) < (b) ? (b)-(a):(a)-(b))

#define closest2now(now,time1,time2)					\
		(time2diff((now), (time1)) < time2diff((now), (time2))	\
		 ? (time1):(time2))

		signature_inception =
			closest2now(now, closest2now(now,
						     prev_epoch,
						     cur_epoch),
				    next_epoch);

		signature_expiration =
			signature_inception +
			((rr->rr.rrsig.signature_expiration -
			  rr->rr.rrsig.signature_inception)
			 & 0x7FFFFFFF);
	}

	append_time_t(&fri, signature_inception);

	append_str(&fri, " ", 1);

	append_time_t(&fri, signature_expiration);

	append_str(&fri, " ", 1);

	append_int32(&fri, rr->rr.rrsig.key_tag);

	append_str(&fri, " ", 1);

	rfc1035_replyhostname(r, rr->rr.rrsig.signer_name, p);

	append_str(&fri, p, strlen(p));

	append_str(&fri, ". ", 2);

	libmail_encode_start(&lei, "base64", encode_callback_func, &fri);

	libmail_encode(&lei, rr->rr.rrsig.signature,
		       rr->rr.rrsig.signature_len);
	libmail_encode_end(&lei);

	return fri.n;
}

char *rfc1035_dumprrdata(struct rfc1035_reply *r, struct rfc1035_rr *rr)
{
	if (rr->rrclass != RFC1035_CLASS_IN)	return (0);
	switch (rr->rrtype)	{
	case RFC1035_TYPE_A:
		{
		char	ipbuf[RFC1035_NTOABUFSIZE];

			rfc1035_ntoa_ipv4(&rr->rr.inaddr, ipbuf);
			return (strdup(ipbuf));
		}
#if	RFC1035_IPV6
	case RFC1035_TYPE_AAAA:
		{
		char	ipbuf[INET6_ADDRSTRLEN];

			if (inet_ntop(AF_INET6, &rr->rr.in6addr,
				ipbuf, sizeof(ipbuf)) == 0)
				ipbuf[0]=0;
			return (strdup(ipbuf));
		}
#endif
	case RFC1035_TYPE_TXT:
		return (dumptxt(r, rr));
	case RFC1035_TYPE_CNAME:
	case RFC1035_TYPE_MB:
	case RFC1035_TYPE_MG:
	case RFC1035_TYPE_MR:
	case RFC1035_TYPE_MD:
	case RFC1035_TYPE_MF:
	case RFC1035_TYPE_NS:
	case RFC1035_TYPE_PTR:
		{
		char	p[RFC1035_MAXNAMESIZE+1], *q;

			rfc1035_replyhostname(r, rr->rr.domainname, p);

			if ((q=(char *)malloc(strlen(p)+2)) != 0)
				strcat(strcpy(q, p), ".");
			return (q);
		}

	case RFC1035_TYPE_SOA:
		return (dumpsoa(r, rr));
		break;
	case RFC1035_TYPE_MX:

		{
		char	p[RFC1035_MAXNAMESIZE+1], *q;

			rfc1035_replyhostname(r, rr->rr.mx.mx_label, p);

			if ((q=(char *)malloc(strlen(p)+40)) != 0)
			{
				sprintf(q, "%d %s.",
					(int)rr->rr.mx.preference, p);
			}
			return (q);
		}

	case RFC1035_TYPE_HINFO:
		{
		char *p=malloc( (int)(unsigned char)*rr->rr.hinfo.hinfo_str+
				(int)(unsigned char)*rr->rr.hinfo.os_str+10);

			if (p)
			{
			const char *q=rr->rr.hinfo.hinfo_str;

				*p=0;
				str_cat(p, &q);
				strcat(p, ", ");
				q=rr->rr.hinfo.os_str;
				str_cat(p, &q);
			}
			return (p);
		}
	case RFC1035_TYPE_MINFO:
		{
		char	p[RFC1035_MAXNAMESIZE+1], t[RFC1035_MAXNAMESIZE+1], *q;

			rfc1035_replyhostname(r, rr->rr.minfo.rmailbx_label, p);
			rfc1035_replyhostname(r, rr->rr.minfo.emailbx_label, t);

			if ((q=(char *)malloc(strlen(p)+strlen(t)+4)) == 0)
				return (0);
			strcat(strcat(strcat(strcpy(q, p), ". "), t), ".");
			return (q);
		}

	case RFC1035_TYPE_RRSIG:
		{
			time_t now=time(NULL);

			size_t n=fmt_rrsig(r, rr, now, NULL);
			char *p;

			if ((p=malloc(n+1)) == 0)
				return (0);

			fmt_rrsig(r, rr, now, p);

			p[n]=0;
			return p;
		}
	}
	return (0);
}
