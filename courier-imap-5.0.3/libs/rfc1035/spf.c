/*
** Copyright 2004-2016 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"spf.h"
#include	"rfc1035mxlist.h"
#include	<stdio.h>
#include	<ctype.h>
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

struct rfc1035_spf_info {
	const char *mailfrom;
	const char *current_domain;
	const char *tcpremoteip;
	const char *tcpremotehost;
	const char *helodomain;
	const char *mydomain;
	char *errmsg_buf;
	size_t errmsg_buf_size;

	size_t *lookup_cnt;

	struct rfc1035_res res;
};

static void set_err_msg(char *errmsg_buf,
			size_t errmsg_buf_size,
			const char *errmsg)
{
	size_t l=strlen(errmsg);

	if (errmsg_buf_size == 0)
		return;

	--errmsg_buf_size;

	if (l >= errmsg_buf_size)
		l=errmsg_buf_size;
	memcpy(errmsg_buf, errmsg, l);
	errmsg_buf[l]=0;
}

static char lookup(struct rfc1035_spf_info *info);

char rfc1035_spf_lookup(const char *mailfrom,
			const char *tcpremoteip,
			const char *tcpremotehost,
			const char *helodomain,
			const char *mydomain,
			char *errmsg_buf,
			size_t errmsg_buf_size)
{
	size_t lookup_cnt=0;
	struct rfc1035_spf_info info;
	char result;

	if (!tcpremoteip) tcpremoteip="";
	if (!tcpremotehost) tcpremotehost="";
	if (!helodomain) helodomain="";
	if (!mydomain) mydomain="";

	if (errmsg_buf && errmsg_buf_size)
		*errmsg_buf=0;

	/*
	** If the <responsible-sender> has no localpart, clients MUST
	** substitute the string "postmaster" for the localpart.
	*/
	if (strchr(mailfrom, '@') == NULL)
	{
		char *buf=malloc(sizeof("postmaster@")+strlen(mailfrom));
		char err_code;

		if (buf == NULL)
		{
			set_err_msg(errmsg_buf, errmsg_buf_size,
				    strerror(errno));
			return SPF_ERROR;
		}

		err_code=rfc1035_spf_lookup(strcat(strcpy(buf, "postmaster@"),
						   mailfrom),
					    tcpremoteip,
					    tcpremotehost,
					    helodomain,
					    mydomain,
					    errmsg_buf,
					    errmsg_buf_size);
		free(buf);
		return err_code;
	}

	memset(&info, 0, sizeof(info));

	info.mailfrom=mailfrom;

	/*
	** The <current-domain> is initially drawn from the
	** <responsible-sender>.  Recursive mechanisms such as
	** Include and Redirect replace the initial
	** <current-domain> with another domain.  However, they
	** do not change the value of the <responsible-sender>.
	*/
	info.current_domain=strrchr(mailfrom, '@')+1;

	info.tcpremoteip=tcpremoteip;
	info.tcpremotehost=tcpremotehost;
	info.errmsg_buf=errmsg_buf;
	info.errmsg_buf_size=errmsg_buf_size;
	info.helodomain=helodomain;
	info.mydomain=mydomain;
	info.lookup_cnt=&lookup_cnt;

	rfc1035_init_resolv(&info.res);

	result=lookup(&info);

	if (errmsg_buf[0] == 0)
	{
		static const char errmsg[]="Address %s the Sender Policy Framework";
		char *p=malloc(sizeof(errmsg)+strlen(mailfrom)+20);

		if (p)
			sprintf(p, errmsg, result == SPF_PASS
				? "passes":"does not pass");

		set_err_msg(errmsg_buf, errmsg_buf_size,
			    p ? p:strerror(errno));
		if (p) free(p);
	}
	rfc1035_destroy_resolv(&info.res);
	return result;
}

static int isspf1(struct rfc1035_reply *reply, int n)
{
	char txtbuf[256];
	const char *p;

	rfc1035_rr_gettxt(reply->allrrs[n], 0, txtbuf);

	for (p=txtbuf; *p; p++)
		if (!isspace((int)(unsigned char)*p))
			break;

	if (strncasecmp(p, "v=spf1", 6) == 0 &&
	    (p[6] == 0 ||
	     isspace((int)(unsigned char)p[6])))
		return 1;

	return 0;
}

struct rfc1035_exp_str {
	struct rfc1035_exp_str *next;
	const char *ptr;
};

static char *rfc1035_concat_strings(struct rfc1035_rr *rr,
				    int startpos,
				    struct rfc1035_exp_str *head,
				    struct rfc1035_exp_str *tail)
{
	char buf[256];
	struct rfc1035_exp_str node;
	int ret=rfc1035_rr_gettxt(rr, startpos, buf);

	node.next=NULL;
	node.ptr=buf;
	tail->next=&node;

	if (ret < 0)
	{
		size_t s=1;
		char *buf;
		struct rfc1035_exp_str *p;

		for (p=head; p; p=p->next)
		{
			s += strlen(p->ptr);
		}

		buf=malloc(s);
		if (!buf)
			return NULL;
		*buf=0;

		for (p=head; p; p=p->next)
		{
			strcat(buf, p->ptr);
		}
		return buf;
	}

	return rfc1035_concat_strings(rr, ret,
				      head,
				      &node);
}

static char rfc1035_spf_gettxt_res(const char *current_domain,
				   char **buf,
				   struct rfc1035_res *res)
{
	struct	rfc1035_reply *reply;
	char	namebuf[RFC1035_MAXNAMESIZE+1];
	int n, o;

	*buf=0;
	namebuf[0]=0;
	strncat(namebuf, current_domain, RFC1035_MAXNAMESIZE);

	if (rfc1035_resolve_cname(res, namebuf,
				  RFC1035_TYPE_TXT, RFC1035_CLASS_IN,
				  &reply, 0) < 0 ||
	    reply == 0 ||
	    (n=rfc1035_replysearch_an(res, reply, namebuf, RFC1035_TYPE_TXT,
				      RFC1035_CLASS_IN, 0)) < 0)
	{
		switch (reply ? reply->rcode:
			RFC1035_RCODE_SERVFAIL) {
		case RFC1035_RCODE_NOERROR:
			rfc1035_replyfree(reply);
			return SPF_NONE;
		case RFC1035_RCODE_NXDOMAIN:
			rfc1035_replyfree(reply);
			return SPF_UNKNOWN;
		default:
			break;
		}
		if (reply)
			rfc1035_replyfree(reply);
		return SPF_ERROR;
	}

	while (n >= 0)
	{
		if (isspf1(reply, n))
			break;

		n=rfc1035_replysearch_an(res, reply, namebuf,
					 RFC1035_TYPE_TXT, RFC1035_CLASS_IN,
					 n+1);
	}

	if (n >= 0)
	{
		struct rfc1035_exp_str str;

		str.next=NULL;
		str.ptr="";

		for (o=n; (o=rfc1035_replysearch_an(res, reply, namebuf,
						    RFC1035_TYPE_TXT,
						    RFC1035_CLASS_IN,
						    o+1)) >= 0; )
		{

			/*
			**
			** A domain MUST NOT return multiple records that
			** begin with the version "v=spf1".  If more than
			** one "v=spf1" record is returned, this constitutes
			** a syntax error and the result is "unknown".
			*/

			if (isspf1(reply, o))
			{
				rfc1035_replyfree(reply);
				return SPF_UNKNOWN;
			}
		}

		*buf=rfc1035_concat_strings(reply->allrrs[n], 0, &str, &str);
		rfc1035_replyfree(reply);

		if (!*buf)
			return SPF_UNKNOWN;

		return SPF_PASS;
	}
	rfc1035_replyfree(reply);
	return SPF_UNKNOWN;
}

char rfc1035_spf_gettxt_n(const char *current_domain,
			  char **buf)
{
	struct rfc1035_res res;
	char c;

	rfc1035_init_resolv(&res);

	c=rfc1035_spf_gettxt_res(current_domain, buf, &res);

	rfc1035_destroy_resolv(&res);

	return c;
}

/*
** Chop up an SPF record into whitespace-delimited words.
** get_words() is called twice: once with wordptr=NULL - return # of words,
** second time with wordptr!=NULL, parse the words.
*/

static unsigned get_words(char *record, char **wordptr)
{
	unsigned n=0;

	while (*record)
	{
		if (isspace((int)(unsigned char)*record))
		{
			++record;
			continue;
		}

		if (wordptr)
			*wordptr++=record;
		++n;

		while (*record)
		{
			if (isspace((int)(unsigned char)*record))
				break;
			++record;
		}

		if (*record && wordptr)
			*record++=0;
	}
	return n;
}

static char spf_compute(char **words,
			struct rfc1035_spf_info *info);

char rfc1035_spf_compute(char *record,
			 struct rfc1035_spf_info *info)
{
	unsigned n=get_words(record, NULL);
	char **words=malloc((n+1)*sizeof(char *));
	char rc;

	if (words == NULL)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    strerror(errno));
		return SPF_ERROR;
	}

	get_words(record, words);
	words[n]=0;

	rc=spf_compute(words, info);
	free(words);
	return rc;
}

static char mechanism(const char *name,
		      struct rfc1035_spf_info *info);

static void setexp(const char *name,
		   struct rfc1035_spf_info *info);
static char *expand(const char *str,
		    struct rfc1035_spf_info *info);


static char spf_compute(char **words,
			struct rfc1035_spf_info *info)
{
	size_t i;
	char rc;
	const char *exp=NULL;
	const char *redirect=NULL;

	for (i=0; words[i]; i++)
		if (strncasecmp(words[i], "exp=", 4) == 0)
			exp=words[i]+4;

	for (i=0; words[i]; i++)
	{
		const char *name;
		char prefix;

		if (strncasecmp(words[i], "redirect=", 9) == 0)
			redirect=words[i]+9;

		if (strchr(words[i], '='))
			continue;

		name=words[i];

		switch (*name) {
		case '+':
		case '-':
		case '?':
		case '~':
			prefix=*name++;
			break;
		default:
			prefix='+';
			break;
		}

		rc=mechanism(name, info);

		/*
		** When a mechanism is evaluated, one of three things can
		** happen: it can match, it can not match, or it can throw an
		** exception.
		*/

		if (rc == SPF_PASS)
		{
			/*
			** If it matches, processing ends and the prefix value
			** is returned as the result of that record.
			*/

			if (prefix != SPF_PASS && exp)
				setexp(exp, info);
			return prefix;
		}

		if (rc == SPF_FAIL)
		{
			/*
			** If it does not match, processing continues with
			** the next
			** mechanism.
			*/

			continue;
		}

		/*
		** If it throws an exception, mechanism processing ends and
		** the exception value is returned (either "error"
		** indicating a temporary failure, usually DNS-related, or
		** "unknown" indicating a syntax error or other permanent
		** failure resulting in incomplete processing.)
		*/
		return rc;
	}

	if (redirect)
	{
		/*
		** If all mechanisms fail to match, and a redirect modifier
		** is present, then processing proceeds as follows.
		**
		** The domain-spec portion of the redirect section is expanded
		** as per the macro rules in section 7.  The resulting string
		** is a new domain that is now queried:  The <current-domain>
		** is set to this new domain, and the new domain's SPF record
		** is fetched and processed.  Note that <responsible-sender>
		** does not change.
		**
		** The result of this new query is then considered the result
		** of original query.
		*/


		char *new_domain;
		struct rfc1035_spf_info newinfo;
		char rc;

		new_domain=expand(redirect, info);

		if (!new_domain)
			return SPF_ERROR;

		newinfo= *info;
		newinfo.current_domain=new_domain;

		rc=lookup(&newinfo);
		free(new_domain);
		return rc;
	}

	/*
	** If none of the mechanisms match and there is no redirect modifier,
	** then the result of the SPF query is "neutral".
	*/

	if (exp)
		setexp(exp, info);

	return SPF_NEUTRAL;
}

static int get_dual_cidr_length(const char *p)
{
#if RFC1035_IPV6
	const char *q;

	for (q=p; *q; q++)
		if (*q == '/' && q[1] == '/')
			return atoi(q+2);

	return atoi(p+1)+12*8;
#else
	if (p[1] == '/')
		return -1;
	return atoi(p+1);
#endif
}

static int ip_compare(const RFC1035_ADDR *a,
		      const RFC1035_ADDR *b,
		      int pfix)
{
	const unsigned char *ca, *cb;
	unsigned i;

	if (pfix < 0)
		return 0;

	ca=(const unsigned char *)a;
	cb=(const unsigned char *)b;

	for (i=0; i<sizeof(RFC1035_ADDR); i++)
	{
		int bits=pfix>8?8:pfix;
		unsigned char m=(unsigned char )(~0 << (8-bits));

		if ((ca[i] & m) != (cb[i] & m))
			return 0;

		pfix -= bits;
	}
	return 1;
}

static void get_domain_pfix(struct rfc1035_spf_info *info,
			    const char *start,
			    char **domain_ptr,
			    int  *pfix_ptr)
{
	*pfix_ptr=sizeof(RFC1035_ADDR)*8;

	if (*start == 0 || *start == '/')
	{
		*domain_ptr=strdup(info->current_domain);

		if (*start == '/')
			*pfix_ptr=get_dual_cidr_length(start);
	}
	else
	{
		char *p;

		*domain_ptr=strdup(*start == ':' ? start+1:start);

		p=strchr(*domain_ptr, '/');
		if (p)
		{
			*pfix_ptr=get_dual_cidr_length(p);
			*p++=0;
		}

		if (*domain_ptr == 0)
		{
			free(*domain_ptr);
			*domain_ptr=strdup(info->current_domain);
		}
	}

	if (!*domain_ptr)
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    strerror(errno));
}

struct ptr_info {
	const char *name;
	struct rfc1035_spf_info *info;
	RFC1035_ADDR addr;
	int found;
	int error;
};

static void check_ptr(const char *ptr, void *void_arg)
{
	struct ptr_info *pinfo=(struct ptr_info *)void_arg;
	RFC1035_ADDR *addr;
	unsigned addr_cnt;
	int rc;
	unsigned i;

	if (pinfo->found)
		return; /* No need */

	rc=rfc1035_a(&pinfo->info->res, ptr, &addr, &addr_cnt);

	if (rc > 0)
		pinfo->error=1;
	if (rc)
		return;

	for (i=0; i<addr_cnt; i++)
	{
		if (memcmp(&addr[i], &pinfo->addr,
			   sizeof(pinfo->addr)) == 0)
			break;
	}

	if (i < addr_cnt)
	{
		size_t l1, l2;

		if (strcasecmp(ptr, pinfo->name) == 0)
			pinfo->found=1;

		l1=strlen(ptr);
		l2=strlen(pinfo->name);

		if (l2 < l1)
		{
			ptr=ptr+l1-l2;

			if (ptr[-1] == '.' && strcasecmp(ptr, pinfo->name)==0)
				pinfo->found=1;
		}
	}
	free(addr);
}

static char do_ptr(const char *name,
		   struct rfc1035_spf_info *info)
{
	struct ptr_info pinfo;

	if (*name++ == ':')
		pinfo.name=name;
	else
		pinfo.name=strrchr(info->mailfrom, '@')+1;

	pinfo.info=info;
	pinfo.found=0;
	pinfo.error=0;

	/*
	** First the <sending-host>'s name is looked up using this
	** procedure:
	**
	** perform a PTR lookup against the <sending-host>'s IP.  For
	** each record returned, validate the host name by looking up
	** its IP address.  If the <sending-host>'s IP is among the
	** returned IP addresses, then that host name is validated.
	**
	** Check all validated hostnames to see if they end in the
	** <target-name> domain.  If any do, this mechanism matches.
	** If no validated hostname can be found, or if none of the
	** validated hostnames end in the <target-name>, this
	** mechanism fails to match.
	*/

	if (rfc1035_aton(info->tcpremoteip, &pinfo.addr) < 0)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    "Invalid tcpremoteip.");
		return SPF_FAIL;
	}

	if (rfc1035_ptr_x(&info->res, &pinfo.addr,
			  check_ptr, &pinfo) < 0)
	{
		if (errno == ENOENT)
			return SPF_FAIL;

		return SPF_ERROR;
	}

	if (pinfo.found)
		return SPF_PASS;
	if (pinfo.error)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    "ptr lookup failed.");
		return SPF_UNKNOWN;
	}
	return SPF_FAIL;
}

static char do_ipcheck(const char *name, struct rfc1035_spf_info *info,
		       int pfix_add)
{
	char *addrptr;
	char *p;
	int pfix;
	RFC1035_ADDR addr, addrcmp;

	/*
	** These mechanisms test if the <sending-host> falls into a
	** given IP network.
	**
	** The <sending-host> is compared to the given network.  If
	** they match, the mechanism matches.
	*/

	if (rfc1035_aton(info->tcpremoteip, &addr) < 0)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    "Invalid tcpremoteip.");
		return SPF_FAIL;
	}

	if ((addrptr=strdup(name+4)) == NULL)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    strerror(errno));
		return SPF_ERROR;
	}

	p=strrchr(addrptr, '/');
	pfix=sizeof(RFC1035_ADDR)*8;

	if (p)
	{
		*p++=0;
		pfix=atol(p)+pfix_add;
	}

	if (rfc1035_aton(addrptr, &addrcmp) < 0)
	{
		free(addrptr);
		return SPF_FAIL;
	}
	free(addrptr);
	if (ip_compare(&addr, &addrcmp, pfix))
		return SPF_PASS;
	return SPF_FAIL;
}

static char mechanism(const char *name,
		      struct rfc1035_spf_info *info)
{
	if (strcasecmp(name, "all") == 0)
	{
		/*
		** The "all" mechanism is a test that always matches.  It is
		** used as the rightmost mechanism in an SPF record to
		** provide an explicit default.
		*/
		return SPF_PASS;
	}

	if (strncasecmp(name, "include:", 8) == 0)
	{
		char *new_domain;
		struct rfc1035_spf_info newinfo;
		char rc;

		/*
		** The "include" mechanism triggers a recursive SPF query.  The
		** domain-spec is expanded as per section 7.  Then a new query
		** is launched using the resulting string as the
		** <current-domain>.  The <responsible-sender> stays the same.
		*/

		new_domain=expand(name+8, info);

		if (!new_domain)
			return SPF_ERROR;

		newinfo= *info;
		newinfo.current_domain=new_domain;

		/*
		**      included    include
		**      query       mechanism      SPF
		**      result      result         processing
		**      -------- -- -------------- -------------------------------------
		**      pass     => match,         return the prefix value for "include"
		**      fail     => no match,      continue processing
		**      softfail => no match,      continue processing
		**      neutral  => no match,      continue processing
		**      error    => throw error,   abort processing, return error
		**      unknown  => throw unknown, abort processing, return unknown
		**      none     => throw unknown, abort processing, return unknown
		*/

		rc=lookup(&newinfo);
		free(new_domain);

		switch (rc) {
		case SPF_PASS:
			return SPF_PASS;
		case SPF_FAIL:
		case SPF_SOFTFAIL:
		case SPF_NEUTRAL:
			return SPF_FAIL;
		case SPF_ERROR:
			return SPF_ERROR;
		default:
			return SPF_UNKNOWN;
		}
	}

	if (strncasecmp(name, "a", 1) == 0 &&
	    (name[1] == 0 || name[1] == ':' || name[1] == '/'))
	{
		char *domain_spec;
		int pfix;
		RFC1035_ADDR addr;

		RFC1035_ADDR *iaptr;
		unsigned iasize;
		int rc;
		unsigned ii;

		/*
		** This mechanism matches if the <sending-host> is one of the
		** <target-name>'s IP addresses.
		**
		** A = "a" [ ":" domain-spec ] [ dual-cidr-length ]
		**
		** The <sending-host> is compared to the IP address(es) of the
		** <target-name>.  If any address matches, the mechanism
		** matches.
		*/

		get_domain_pfix(info, name+1, &domain_spec, &pfix);

		if (!domain_spec)
			return SPF_ERROR;

		if (rfc1035_aton(info->tcpremoteip, &addr) < 0)
		{
			free(domain_spec);
			set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
				    "Invalid tcpremoteip.");
			return SPF_FAIL;
		}

		rc=rfc1035_a(&info->res,
			     domain_spec,
			     &iaptr,
			     &iasize);

		free(domain_spec);

		if (rc != 0)
		{
			set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
				    "IP address lookup failed.");
			return SPF_UNKNOWN;
		}

		for (ii=0; ii<iasize; ii++)
			if (ip_compare(&addr, iaptr+ii, pfix))
			{
				free(iaptr);
				return SPF_PASS;
			}

		free(iaptr);
		return SPF_FAIL;
	}


	if (strncasecmp(name, "mx", 2) == 0 &&
	    (name[2] == 0 || name[2] == ':' || name[2] == '/'))
	{
		char *domain_spec;
		int pfix;
		int rc;
		struct rfc1035_mxlist *mxlist, *mxp;
		RFC1035_ADDR addr;

		/*
		** This mechanism matches if the <sending-host> is one of the
		** MX hosts for a domain name.

		** MX = "mx" [ ":" domain-spec ] [ dual-cidr-length ]

		** SPF clients first perform an MX lookup on the <target-name>.
		** SPF clients then perform an A lookup on each MX name
		** returned, in order of MX priority.  The <sending-host> is
		** compared to each returned IP address.  If any address
		** matches, the mechanism matches.
		*/

		get_domain_pfix(info, name+2, &domain_spec, &pfix);

		if (!domain_spec)
			return SPF_ERROR;

		if (rfc1035_aton(info->tcpremoteip, &addr) < 0)
		{
			free(domain_spec);
			set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
				    "Invalid tcpremoteip.");
			return SPF_FAIL;
		}

		rc=rfc1035_mxlist_create_x(&info->res,
					   domain_spec, RFC1035_MX_QUERYALL,
					   &mxlist);
		free(domain_spec);

		if (rc && rc != RFC1035_MX_HARDERR)
		{
			rfc1035_mxlist_free(mxlist);
			set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
				    "DNS MX lookup failed.");
			return SPF_ERROR;
		}

		for (mxp=mxlist; mxp; mxp=mxp->next)
		{
			RFC1035_ADDR addrcmp;

			if (rfc1035_sockaddrip(&mxp->address,
					       sizeof(mxp->address),
					       &addrcmp) < 0)
				continue;

			if (ip_compare(&addr, &addrcmp, pfix))
			{
				rfc1035_mxlist_free(mxlist);
				return SPF_PASS;
			}
		}
		rfc1035_mxlist_free(mxlist);
		return SPF_FAIL;
	}

	if (strncasecmp(name, "ip4:", 4) == 0)
	{
		if (strchr(name+4, ':'))
			return SPF_FAIL; /* What does IPv6 addr doing here? */

#if RFC1035_IPV6
		return do_ipcheck(name, info, 12*8);
#else
		return do_ipcheck(name, info, 0);
#endif
	}

	if (strncasecmp(name, "ip6:", 4) == 0)
	{
#if RFC1035_IPV6
		return do_ipcheck(name, info, 0);
#else
		return SPF_FAIL;
#endif
	}

	if (strncasecmp(name, "ptr", 3) == 0 &&
	    (name[3] == 0 || name[3] == ':'))
	{
		return do_ptr(name+3, info);
	}

	if (strncasecmp(name, "exists:", 7) == 0)
	{
		char *domain_spec;
		RFC1035_ADDR *iaptr;
		unsigned iasize;
		int rc;

		/*
		** This mechanism is used to construct an arbitrary host name
		** that is used for a DNS A record query.  It allows for
		** complicated schemes involving arbitrary parts of the mail
		** envelope to determine what is legal.
		**
		** exists = "exists" ":" domain-spec
		**
		** The domain-spec is expanded as per Section 7.  The
		** resulting domain name is used for a DNS A lookup.  If any
		** A record is returned, this mechanism matches.  The lookup
		** type is 'A' even when the connection type is IPv6.
		*/

		domain_spec=expand(name+7, info);
		if (!domain_spec)
			return SPF_ERROR;

		rc=rfc1035_a(&info->res,
			     domain_spec,
			     &iaptr,
			     &iasize);
		free(domain_spec);

		if (rc < 0)
			return SPF_FAIL;
		if (rc > 0)
			return SPF_ERROR;
		free(iaptr);
		return SPF_PASS;
	}

	return SPF_FAIL;
}

static void setexp(const char *exp,
		   struct rfc1035_spf_info *info)
{
	struct	rfc1035_reply *reply;
	char	namebuf[RFC1035_MAXNAMESIZE+1];
	char	txtbuf[256];
	int n;
	char	*str;

	/*
	** The argument to the explanation modifier is a domain-spec
	** to be TXT queried.  The result of the TXT query is a
	** macro-string that is macro-expanded.  If SPF processing
	** results in a rejection, the expanded result SHOULD be
	** shown to the sender in the SMTP reject message.  This
	** string allows the publishing domain to communicate further
	** information via the SMTP receiver to legitimate senders in
	** the form of a short message or URL.
	*/


	namebuf[0]=0;
	strncat(namebuf, exp, RFC1035_MAXNAMESIZE);

	if (rfc1035_resolve_cname(&info->res, namebuf,
				  RFC1035_TYPE_TXT, RFC1035_CLASS_IN,
				  &reply, 0) < 0 ||
	    reply == 0 ||
	    (n=rfc1035_replysearch_an(&info->res,
				      reply, namebuf, RFC1035_TYPE_TXT,
				      RFC1035_CLASS_IN, 0)) < 0)
	{
		set_err_msg(info->errmsg_buf,
			    info->errmsg_buf_size,
			    "A DNS lookup error occured while"
			    " fetching the SPF explanation record.");
	}
	else
	{
		rfc1035_rr_gettxt(reply->allrrs[n], 0, txtbuf);

		str=expand(txtbuf, info);

		set_err_msg(info->errmsg_buf,
			    info->errmsg_buf_size,
			    str ? str:strerror(errno));
		if (str)
			free(str);
	}
	rfc1035_replyfree(reply);
}

static char lookup(struct rfc1035_spf_info *info)
{
	char *record;
	char c;

	/*
	**
	** If a loop is detected, or if more than 20 subqueries are triggered,
	** an SPF client MAY abort the lookup and return the result "unknown".
	*/

	if (++*info->lookup_cnt > 20)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    "Maximum of 20 nested SPF queries exceeded.");
		return SPF_UNKNOWN;
	}

	c=rfc1035_spf_gettxt_n(info->current_domain, &record);

	if (c != SPF_PASS)
		return c;

	c=rfc1035_spf_compute(record, info);
	free(record);
	return c;
}

/*
**
** Certain directives perform macro interpolation on their arguments.
**
** Two passes: count # of chars in the expanded macro, generate the macro.
*/

static int do_expand(const char *str, struct rfc1035_spf_info *info,
		     void (*cb_func)(const char *, size_t n, void *),
		     void *void_arg);

static void do_count(const char *p, size_t n, void *va)
{
	*(size_t *)va += n;
}

static void do_save(const char *p, size_t n, void *va)
{
	char **b=(char **)va;

	memcpy(*b, p, n);
	*b += n;
}

static char *expand(const char *str,
		    struct rfc1035_spf_info *info)
{
	size_t cnt=1;
	char *buf;
	char *p;

	if (do_expand(str, info, do_count, &cnt) < 0)
		return NULL;

	buf=malloc(cnt);

	if (!buf)
	{
		set_err_msg(info->errmsg_buf, info->errmsg_buf_size,
			    strerror(errno));
		return NULL;
	}

	p=buf;
	if (do_expand(str, info, do_save, &p) < 0)
	{
		free(buf);
		return NULL;
	}

	*p=0;
	return buf;
}

static char *get_macro(struct rfc1035_spf_info *info, char name);

static char *transform(char *macro,
		       unsigned transformer_count,
		       char transformer_reverse,
		       char delimiter_char);

static int do_expand(const char *str, struct rfc1035_spf_info *info,
		      void (*cb_func)(const char *, size_t, void *),
		      void *void_arg)
{
	unsigned char alpha, lalpha;
	unsigned transformer_count;
	char transformer_reverse;
	char delimiter_char;
	char *macro;

	/*
	**     macro-string = *( macro-char / VCHAR )
	**     macro-char   = ( "%{" ALPHA transformer *delimiter "}" )
	**                    / "%%" / "%_" / "%-"
	**     transformer  = [ *DIGIT ] [ "r" ]
	**     delimiter    = "." / "-" / "+" / "," / "/" / "_" / "="
	**
	*/

	while (*str)
	{
		size_t i;

		for (i=0; str[i]; i++)
			if (str[i] == '%')
				break;

		if (i)
		{
			(*cb_func)(str, i, void_arg);
			str += i;
			continue;
		}

		/*
		**   A literal "%" is expressed by "%%".
		**   %_ expands to a single " " space.
		**   %- expands to a URL-encoded space, viz. "%20".
		*/

		switch (str[i+1]) {
		case '{':
			break;
		case '%':
			(*cb_func)("%", 1, void_arg);
			str += 2;
			continue;
		case '_':
			(*cb_func)(" ", 1, void_arg);
			str += 2;
			continue;
		case '-':
			(*cb_func)("%20", 3, void_arg);
			str += 2;
			continue;
		default:
			++str;
			continue;
		}

		str += 2;

		if (!*str)
			continue;
		alpha=(unsigned char)*str++;
		transformer_count=0;
		while (*str && isdigit((unsigned char)*str))
		{
			transformer_count=transformer_count * 10 +
				(*str++ - '0');
		}

		transformer_reverse=0;
		delimiter_char=0;

		while (*str && *str != '}')
		{
			switch (*str) {
			case 'r':
			case 'R':
				transformer_reverse='r';
				break;
			case '.':
			case '-':
			case '+':
			case ',':
			case '/':
			case '_':
			case '=':
				delimiter_char= *str;
				break;
			}
			++str;
		}
		lalpha=tolower(alpha);

		macro=get_macro(info, lalpha);
		if (macro && (transformer_reverse || transformer_count))
		{
			char *new_macro=transform(macro, transformer_count,
						  transformer_reverse,
						  delimiter_char);

			free(macro);
			macro=new_macro;
		}

		if (macro && lalpha != alpha)
		{
			static const char validchars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
			size_t l=1;
			size_t i,j;
			char *p;

			for (i=0; macro[i]; i++)
			{
				++l;
				if (strchr(validchars, macro[i]) == NULL)
					l += 2;
			}

			p=malloc(l);
			for (i=j=0; p && macro[i]; i++)
			{
				if (strchr(validchars, macro[i]))
				{
					p[j]=macro[i];
					++j;
				}
				else
				{
					sprintf(p+j, "%%%02X",
						(int)(unsigned char)macro[i]);
					j += 3;
				}
			}
			if (p)
				p[j]=0;
			free(macro);
			macro=p;
		}

		if (macro == NULL)
		{
			set_err_msg(info->errmsg_buf,
				    info->errmsg_buf_size,
				    strerror(errno));
			return -1;
		}

		(*cb_func)(macro, strlen(macro), void_arg);
		free(macro);
		if (*str == '}')
			++str;
	}
	return 0;
}

static char *expandc(const char *ipaddr);
static char *expandi(const char *ipaddr);

static char *get_macro(struct rfc1035_spf_info *info, char name)
{
	char *p;
	const char *cp;

	switch (name) {
	case 'l':
		/* l = local-part of responsible-sender */

		cp=strrchr(info->mailfrom, '@');
		p=malloc(cp-info->mailfrom+1);
		if (!p)
			return p;
		memcpy(p, info->mailfrom, cp-info->mailfrom);
		p[cp-info->mailfrom]=0;
		return p;
	case 's':
		/* s = responsible-sender */
		return strdup(info->mailfrom);
	case 'o':
		return strdup(strrchr(info->mailfrom, '@')+1);
		/* o = responsible-domain */
	case 'd':
		return strdup(info->current_domain);
		/* d = current-domain */
	case 'c':
		return expandc(info->tcpremoteip);
		/* c = SMTP client IP (easily readable format) */
	case 'i':
		return expandi(info->tcpremoteip);
		/* i = SMTP client IP (nibble format when an IPv6 address) */
	case 'p':
		return strdup(info->tcpremotehost);
		/* p = SMTP client domain name */
	case 'v':
		return (strdup(strchr(info->tcpremoteip, ':') &&
			       strncmp(info->tcpremoteip, "::ffff:", 7)
			       ? "ip6":"in-addr"));
		/* v = client IP version string: "in-addr" for ipv4 or "ip6" for ipv6 */
	case 'h':
	/* h = HELO/EHLO domain */
		return strdup(info->helodomain);
	case 'r':
	/* r = receiving domain */
		return strdup(info->mydomain);
	}

	return strdup("");
}

/*
**
**   For IPv4 addresses, both the "i" and "c" macros expand to the
**   standard dotted-quad format.
**
**   For IPv6 addresses, the "i" macro expands to dot-format address; it
**   is intended for use in %{ir}.  The "c" macro may expand to any of
**   the hexadecimal colon-format addresses specified in [RFC3513] section
**   2.2.  It is intended for humans to read.
*/

static char *expandc(const char *ipaddr)
{
	if (strncmp(ipaddr, "::ffff:", 7) == 0)
		return strdup(ipaddr+7);
	return strdup(ipaddr);
}

static char *expandi(const char *ipaddr)
{
	if (strchr(ipaddr, ':') &&
	    strncmp(ipaddr, "::ffff:", 7))
	{
		RFC1035_ADDR addr;

		if (rfc1035_aton(ipaddr, &addr) == 0)
		{
			char name[sizeof(addr)*4+1];
			char *p=name;
			int i;
			unsigned char *q=(unsigned char *)&addr;

			for (i=0; i<sizeof(addr); i++)
			{
				sprintf(p, "%s%x.%x", i ? ".":"",
					(int)((q[i] >> 4) & 0x0F),
					(int)(q[i] & 0x0F));
				p += strlen(p);
			}
			return strdup(name);
		}

	}
	return expandc(ipaddr);
}

/*
**   If transformers or delimiters are provided, the macro strings are
**   split into parts.  After performing any reversal operation or
**   removal of left-hand parts, the parts are rejoined using "." and not
**   the original splitting characters.
*/

static unsigned tsplit(char *macro, char delimiter, char **wordptr)
{
	/* Two passes */
	unsigned cnt=0;

	if (!delimiter)
		delimiter='.';

	while (*macro)
	{
		++cnt;

		if (wordptr)
			*wordptr++=macro;

		while (*macro && *macro != delimiter)
			++macro;

		if (*macro)
		{
			if (wordptr)
				*macro=0;
			++macro;
		}

	}
	return cnt;
}

static char *transform(char *macro,
		       unsigned transformer_count,
		       char transformer_reverse,
		       char delimiter_char)
{
	char **words;
	unsigned n=tsplit(macro, delimiter_char, NULL);
	unsigned start;
	unsigned i;
	char *buf;
	size_t len;

	if ((words=malloc(sizeof(char *)*(n+1))) == NULL)
		return NULL;
	tsplit(macro, delimiter_char, words);
	words[n]=NULL;

	/*
	** The DIGIT transformer indicates the number of right-hand parts to
	** use after optional reversal.  If a DIGIT is specified, it MUST be
	** nonzero.  If no DIGITs are specified, or if the value specifies more
	** parts than are available, all the available parts are used.  If the
	** DIGIT was 5, and only 3 parts were available, the macro interpreter
	** would pretend the DIGIT was 3.  Implementations MAY limit the
	** number, but MUST support at least a value of 9.
	*/

	if (transformer_count > n || transformer_count <= 0)
		transformer_count=n;

	if (transformer_reverse)
	{
		start=0;
		n=transformer_count;
	}
	else
	{
		start=n-transformer_count;
	}

	len=1;

	for (i=start; i<n; i++)
	{
		len += strlen(words[i])+1;
	}

	buf=malloc(len);
	if (!buf)
	{
		free(words);
		return NULL;
	}

	*buf=0;
	if (transformer_reverse)
	{
		for (i=n; i>start; )
		{
			if (*buf)
				strcat(buf, ".");
			strcat(buf, words[--i]);
		}
	}
	else
	{
		for (i=start; i<n; i++)
		{
			if (*buf)
				strcat(buf, ".");
			strcat(buf, words[i]);
		}
	}
	free(words);
	return buf;
}
