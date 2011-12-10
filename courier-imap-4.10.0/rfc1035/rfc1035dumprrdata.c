/*
** Copyright 1998 - 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<arpa/inet.h>


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
	}
	return (0);
}
