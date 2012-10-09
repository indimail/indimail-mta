/*
 * $Log: dns.c,v $
 * Revision 1.27  2012-10-09 18:09:20+05:30  Cprogrammer
 * removed DISABLE_CNAME_LOOKUP
 *
 * Revision 1.26  2012-10-08 19:34:11+05:30  Cprogrammer
 * added DISABLE_CNAME_LOOKUP to bypass cname lookup
 *
 * Revision 1.25  2012-06-20 18:38:46+05:30  Cprogrammer
 * moved strsalloc_readyplus() to spf.c
 *
 * Revision 1.24  2012-04-26 18:04:30+05:30  Cprogrammer
 * fix SIGSEGV in dns_txt() function
 *
 * Revision 1.23  2011-07-29 09:28:15+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.22  2009-05-31 09:32:09+05:30  Cprogrammer
 * added fix to disable Verisign Wildcard Feature
 *
 * Revision 1.21  2009-03-14 08:47:31+05:30  Cprogrammer
 * removed function dnsText()
 *
 * Revision 1.20  2008-08-03 18:25:00+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.19  2008-07-25 16:50:35+05:30  Cprogrammer
 * port for darwin
 *
 * Revision 1.18  2005-08-23 17:29:58+05:30  Cprogrammer
 * gcc 4 compliance
 * ipv6 corrections
 *
 * Revision 1.17  2005-06-17 21:47:49+05:30  Cprogrammer
 * replaced struct ip_address and struct ip6_address with shorter typedefs
 *
 * Revision 1.16  2005-06-15 22:55:50+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.15  2005-06-11 21:28:11+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.14  2004-10-22 20:24:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.13  2004-10-21 21:54:42+05:30  Cprogrammer
 * free allocated string
 *
 * Revision 1.12  2004-10-20 01:05:53+05:30  Cprogrammer
 * added dnsText() function
 * corrected bug with getting short txt records
 *
 * Revision 1.11  2004-08-15 20:06:44+05:30  Cprogrammer
 * free local stralloc variables
 *
 * Revision 1.10  2004-08-14 02:36:36+05:30  Cprogrammer
 * added prototype for dns_txtplus()
 *
 * Revision 1.9  2004-08-14 02:16:52+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.8  2004-07-30 17:59:32+05:30  Cprogrammer
 * new code from Fredrik Vermeulen for TLS code
 *
 * Revision 1.7  2003-12-20 12:55:31+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#ifdef DARWIN
#include <nameser8_compat.h>
#endif
#include <arpa/nameser.h>
#include <resolv.h>
#include <errno.h>
#include "ip.h"
#include "byte.h"
#include "ipalloc.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif
#include "fmt.h"
#include "alloc.h"
#include "str.h"
#include "stralloc.h"
#include "dns.h"
#include "case.h"

/*-
 * As of a little while ago (it is around 7:45 PM US Eastern on Mon 15 Sep 2003 as I write this),
 * VeriSign added a wildcard A record to the .COM and .NET TLD DNS zones. The IP address
 * returned is 64.94.110.11, which reverses to sitefinder.verisign.com. What that means in plain
 * English is that most mis-typed domain names that would formerly have resulted in a helpful
 * error message now results in a VeriSign advertising opportunity.
 *
 * Fix Versign Breakage.
 * With this patch we treat wildcard responses (*.com) from the GTLD servers as NX_DOMAIN,
 * like the DNS system did before Verisign broke it for us all.
 * To the hell with these greedy bastards!
 * patch taken from http://www.nrg4u.com/
 *
 * With Verisigns wildcard match for any unregistered domains they broke
 * the DNS in many ways. One is that return MX checks won't work anymore
 * and if someone mistypes a mail recipients domain the message will end
 * up on Versigns dummy server. Today it is rejecting that stuff, but for
 * how long given their track record? I bet they'll use it soon to grab
 * mail froms for their spam list.
 *
 * Since it is not possible to directly detect whether we get a faked
 * wildcard response, we first do a "*.tld" lookup (tld is dynamic from
 * the lookup domain). If we get a response for that, remember its IP
 * address. Now we proceed to the true and full MX/IP lookup. Then we
 * check if one of the IP addresses we get this time is the same as the
 * one we remembered from the wildcard lookup. If yes, we have been
 * tricked and skip over it. If it was the only one, well, then it's in
 * reality a non-existent domain.
 *
 * The advantage of this way of doing it (instead of statically blocking
 * Versigns IP address) is of course that it adjusts itself dynamically
 * when Verisign changes it's setup. In one of their papers Verisign
 * cites some other TLDs who do the same. We kill them too.
 *
 * The disadvantage is that we always do one more DNS lookup for "*.tld".
 *
 */

/*-
#define NO_VERISIGN_WILDCARD 1
*/

#ifdef NO_VERISIGN_WILDCARD
static stralloc tld = {0};
#endif

static unsigned short
getshort(c)
	unsigned char  *c;
{
	unsigned short  u;
	u = c[0];
	return (u << 8) + c[1];
}

static struct
{
	unsigned char  *buf;
} response;
static int      responsebuflen = 0;
static int      responselen;
static unsigned char *responseend;
static unsigned char *responsepos;
static u_long   saveresoptions;

static int      numanswers;
static char     name[MAXDNAME];
static ip_addr  ip;
#ifdef IPV6
static ip6_addr ip6;
#endif
unsigned short  pref;

static stralloc glue = { 0 };
static stralloc tmpsa = { 0 };

static int      (*lookup) () = res_query;
#ifdef IPV6
static int      iaafmt6(char *, ip6_addr *, char *);
#endif
static int      dns_ptrplus(strsalloc *, ip_addr *);

static int
resolve(domain, type)
	stralloc       *domain;
	int             type;
{
	int             n;
	int             i;

	errno = 0;
	if (!stralloc_copy(&glue, domain))
		return DNS_MEM;
	if (!stralloc_0(&glue))
		return DNS_MEM;
	if (!responsebuflen)
	{
		if ((response.buf = (unsigned char *) alloc(PACKETSZ + 1)))
			responsebuflen = PACKETSZ + 1;
		else
			return DNS_MEM;
	}
	responselen = lookup(glue.s, C_IN, type, response.buf, responsebuflen);
	if ((responselen >= responsebuflen) || (responselen > 0 && (((HEADER *) response.buf)->tc)))
	{
		if (responsebuflen < 65536)
		{
			if (alloc_re((char *) &response.buf, responsebuflen, 65536))
				responsebuflen = 65536;
			else
				return DNS_MEM;
		}
		saveresoptions = _res.options;
		_res.options |= RES_USEVC;
		responselen = lookup(glue.s, C_IN, type, response.buf, responsebuflen);
		_res.options = saveresoptions;
	}
	if (responselen <= 0)
	{
		if (errno == ECONNREFUSED)
			return DNS_SOFT;
		if (h_errno == TRY_AGAIN)
			return DNS_SOFT;
		return DNS_HARD;
	}
	responseend = response.buf + responselen;
	responsepos = response.buf + sizeof(HEADER);
	n = ntohs(((HEADER *) response.buf)->qdcount);
	while (n-- > 0)
	{
		if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
			return DNS_SOFT;
		responsepos += i;
		i = responseend - responsepos;
		if (i < QFIXEDSZ)
			return DNS_SOFT;
		responsepos += QFIXEDSZ;
	}
	numanswers = ntohs(((HEADER *) response.buf)->ancount);
	return 0;
}

static int
findname(wanttype)
	int             wanttype;
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME);
	if (i < 0)
		return DNS_SOFT;
	responsepos += i;
	i = responseend - responsepos;
	if (i < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype)
	{
		if (dn_expand(response.buf, responseend, responsepos, name, MAXDNAME) < 0)
			return DNS_SOFT;
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

static int
findip(wanttype)
	int             wanttype;
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
		return DNS_SOFT;
	responsepos += i;
	i = responseend - responsepos;
	if (i < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype)
	{
		if (rrdlen < 4)
			return DNS_SOFT;
		ip.d[0] = responsepos[0];
		ip.d[1] = responsepos[1];
		ip.d[2] = responsepos[2];
		ip.d[3] = responsepos[3];
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

#ifdef IPV6
static int
findip6(wanttype)
	int             wanttype;
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
		return DNS_SOFT;
	responsepos += i;
	i = responseend - responsepos;
	if (i < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype)
	{
		if (rrdlen < 16)
			return DNS_SOFT;
		byte_copy((char *) &ip6.d, 16, (char *) &responsepos[0]);
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}
#endif

static int
findmx(wanttype)
	int             wanttype;
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
		return DNS_SOFT;
	responsepos += i;
	i = responseend - responsepos;
	if (i < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype)
	{
		if (rrdlen < 3)
			return DNS_SOFT;
		pref = (responsepos[0] << 8) + responsepos[1];
		if (dn_expand(response.buf, responseend, responsepos + 2, name, MAXDNAME) < 0)
			return DNS_SOFT;
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

void
dns_init(flagsearch)
	int             flagsearch;
{
	res_init();
	if (flagsearch)
		lookup = res_search;
}

int
dns_cname(sa)
	stralloc       *sa;
{
	int             r;
	int             loop;

	for (loop = 0; loop < 10; ++loop)
	{
		if (!sa->len)
			return loop;
		if (sa->s[sa->len - 1] == ']')
			return loop;
		if (sa->s[sa->len - 1] == '.')
		{
			--sa->len;
			continue;
		}
		switch (resolve(sa, T_ANY))
		{
		case DNS_MEM:
			return DNS_MEM;
		case DNS_SOFT:
			return DNS_SOFT;
		case DNS_HARD:
			return loop;
		default:
			while ((r = findname(T_CNAME)) != 2)
			{
				if (r == DNS_SOFT)
					return DNS_SOFT;
				if (r == 1)
				{
					if (!stralloc_copys(sa, name))
						return DNS_MEM;
					break;
				}
			}
			if (r == 2)
				return loop;
		}
	}
	return DNS_HARD; /*- alias loop */
}

#define FMT_IAA 40

static int
iaafmt(s, ip, dom)
	char           *s;
	ip_addr        *ip;
	const char     *dom;
{
	unsigned int    i;
	unsigned int    len;

	len = 0;
	i = fmt_ulong(s, (unsigned long) ip->d[3]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[2]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[1]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip->d[0]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, (char *) dom);
	len += i;
	if (s)
		s += i;
	return len;
}

#if defined(USE_SPF)
static int
findtxt(sa, domain)
	stralloc       *sa;
	stralloc       *domain;
{
	u_char          response[PACKETSZ + 1];	/*- response */
	int             responselen;			/*- buffer length */
	int             rc;						/*- misc variables */
	int             ancount, qdcount;		/*- answer count and query count */
	u_short         type, rdlength;			/*- fields of records returned */
	u_char         *eom, *cp;
	u_char          buf[PACKETSZ + 1];		/*- we're storing a TXT record here, not just a DNAME */
	u_char         *bufptr;

	if (!stralloc_copy(&glue, domain))
		return DNS_MEM;
	if (!stralloc_0(&glue))
		return DNS_MEM;
	if ((responselen = res_query(glue.s, C_IN, T_TXT, response, sizeof(response))) < 0)
	{
		if (h_errno == TRY_AGAIN)
			return DNS_SOFT;
		else
			return DNS_HARD;
	}
	qdcount = getshort(response + 4);	/*- http://crynwr.com/rfc1035/rfc1035.html#4.1.1.  */
	ancount = getshort(response + 6);
	eom = response + responselen;
	cp = response + HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom)
	{
		if ((rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME)) < 0)
			return DNS_HARD;
		cp += rc + QFIXEDSZ;
	}
	while (ancount-- > 0 && cp < eom)
	{
		if ((rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME)) < 0)
			return DNS_HARD;
		cp += rc;
		if (cp + RRFIXEDSZ >= eom)
			return DNS_HARD;
		type = getshort(cp + 0); /*- http://crynwr.com/rfc1035/rfc1035.html#4.1.3.  */
		rdlength = getshort(cp + 8);
		cp += RRFIXEDSZ;
		if (type != T_TXT)
		{
			cp += rdlength;
			continue;
		}
		bufptr = buf;
		while (rdlength && cp < eom)
		{
			int             cnt;

			cnt = *cp++; /*- http://crynwr.com/rfc1035/rfc1035.html#3.3.14.  */
			if (bufptr - buf + cnt + 1 >= PACKETSZ)
				return DNS_HARD;
			if (cp + cnt > eom)
				return DNS_HARD;
			byte_copy((char *) bufptr, cnt, (char *) cp);
			rdlength -= cnt + 1;
			bufptr += cnt;
			cp += cnt;
			*bufptr = '\0';
		}
		if (!stralloc_copys(sa, (char *) buf))
			return DNS_MEM;
		return (0);
	}
	return DNS_HARD;
}
#endif

#ifdef USE_SPF
int
dns_txt(ssa, domain)
	strsalloc      *ssa;
	stralloc       *domain;
{
	int             r;

	ssa->len = 0;
	if ((r = findtxt(&tmpsa, domain)) < 0)
		return r;
	if (!strsalloc_append(ssa, &tmpsa))
		return DNS_MEM;
	return (0);
}

int
dns_ptr(ssa, ip)
	strsalloc      *ssa;
	ip_addr        *ip;
{
	int             r;

	ssa->len = 0;
	if ((r = dns_ptrplus(ssa, ip)) < 0)
		ssa->len = 0;
	return r;
}

static int
dns_ptrplus(ssa, ip)
	strsalloc      *ssa;
	ip_addr        *ip;
{
	int             r;

	if (!stralloc_ready(&tmpsa, iaafmt((char *) 0, ip, ".in-addr.arpa.")))
		return DNS_MEM;
	tmpsa.len = iaafmt(tmpsa.s, ip, ".in-addr.arpa.");
	switch ((r = resolve(&tmpsa, T_PTR)))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2)
	{
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1)
		{
			if (!stralloc_copys(&tmpsa, name))
				return DNS_MEM;
			if (!strsalloc_append(ssa, &tmpsa))
				return DNS_MEM;
		}
	}
	if (ssa->len)
		return 0;
	return DNS_HARD;
}
#else
int
dns_ptr(sa, ip)
	stralloc       *sa;
	ip_addr        *ip;
{
	int             r;

	if (!stralloc_ready(sa, iaafmt((char *) 0, ip, ".in-addr.arpa.")))
		return DNS_MEM;
	sa->len = iaafmt(sa->s, ip, ".in-addr.arpa.");
	switch (resolve(sa, T_PTR))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2)
	{
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1)
		{
			if (!stralloc_copys(sa, name))
				return DNS_MEM;
			return 0;
		}
	}
	return DNS_HARD;
}
#endif

#ifdef IPV6
static int
iaafmt6(s, ip, dom)
	char           *s;
	ip6_addr       *ip;
	char           *dom;
{
	int             j;
	static char     data[] = "0123456789abcdef";

	if (s)
	{
		for (j = 15; j >= 0; j--)
		{
			*s++ = data[ip->d[j] & 0x0f];
			*s++ = '.';
			*s++ = data[(ip->d[j] >> 4) & 0x0f];
			*s++ = '.';
		}
		str_copy(s, dom);
	}
	return 71;
	/*
	 * 1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.int 
	 */
}

int
dns_ptr6(ssa, ip)
#ifdef USE_SPF
	strsalloc      *ssa;
#else
	stralloc       *ssa;
#endif
	ip6_addr       *ip;
{
	int             r;

	if (!stralloc_ready(&tmpsa, iaafmt6((char *) 0, ip, "ip6.int")))
		return DNS_MEM;
	tmpsa.len = iaafmt6(tmpsa.s, ip, "ip6.int");
	switch (resolve(tmpsa, T_PTR))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2)
	{
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1)
		{
#ifdef USE_SPF
			if (!stralloc_copys(&tmpsa, name))
				return DNS_MEM;
			if (!strsalloc_append(ssa, &tmpsa))
				return DNS_MEM;
#else
			if (!stralloc_copys(&ssa, name))
				return DNS_MEM;
			return 0;
#endif
		}
	}
#ifdef USE_SPF
	if (ssa->len)
		return 0;
#endif
	return DNS_HARD;
}
#endif

static int
dns_ipplus(ia, sa, pref)
	ipalloc        *ia;
	stralloc       *sa;
	int             pref;
{
	int             r, err4 = 0;
#ifdef IPV6
	int             err6 = 0;
#endif
#ifdef TLS
	struct ip_mx    ix = {0, {{"\0"}}, 0, 0};
#else
	struct ip_mx    ix = {0, {{"\0"}}, 0};
#endif
#ifdef NO_VERISIGN_WILDCARD
	int             j;
#ifdef IPV6
	ip6_addr        tldip;
#else
	ip_addr         tldip;
#endif
#endif

	if (!stralloc_copy(&glue, sa))
		return DNS_MEM;
	if (!stralloc_0(&glue))
		return DNS_MEM;
	if (glue.s[0])
	{
		ix.pref = 0;
		ix.af = AF_INET;
		if (!glue.s[ip_scan(glue.s, &ix.addr.ip)] || !glue.s[ip_scanbracket(glue.s, &ix.addr.ip)])
		{
			if (!ipalloc_append(ia, &ix))
				return DNS_MEM;
			return 0;
		}
	}
#ifdef NO_VERISIGN_WILDCARD
	j = byte_rchr(sa->s,sa->len,'.');
	if (j + 2 < sa->len)
	{
		if(!stralloc_copys(&tld, "*"))
			return DNS_MEM;
		if(!stralloc_catb(&tld, sa->s+j, sa->len-j))
			return DNS_MEM;
		switch(resolve(&tld, T_A))
		{
		case DNS_HARD:
			byte_zero((char *) &tldip, sizeof(tldip));
			break;
		case DNS_MEM:
			return DNS_MEM;
		case DNS_SOFT:
			return DNS_SOFT;
		default:
			while ((r = findip(T_A)) != 2)
			{
				if (r == DNS_SOFT)
					return DNS_SOFT;
				if (r == 1)
#ifdef IPV6
					tldip = ip6;
#else
					tldip = ip;
#endif
			}
		}
	}
#endif
#ifdef IPV6
	switch (resolve(sa, T_AAAA))
	{
	case DNS_MEM:
		err6 = DNS_MEM;
		break;
	case DNS_SOFT:
		err6 = DNS_SOFT;
		break;
	case DNS_HARD:
		err6 = DNS_HARD;
		break;
	default:
		while ((r = findip6(T_AAAA)) != 2)
		{
			ix.af = AF_INET6;
			ix.addr.ip6 = ip6;
			ix.pref = pref;
			if (r == DNS_SOFT)
			{
				err6 = DNS_SOFT;
				break;
			}
			if (r == 1)
			{
#ifdef NO_VERISIGN_WILDCARD
				if (!byte_diff((char *) &tldip, sizeof(tldip), (char *) &ip6))
					continue;
#endif
#ifdef TLS
				ix.fqdn = glue.s;
#endif
				if (!ipalloc_append(ia, &ix))
				{
					err6 = DNS_MEM;
					break;
				}
			}
		}
		break;
	}
#endif
	switch (resolve(sa, T_A))
	{
	case DNS_MEM:
		err4 = DNS_MEM;
		break;
	case DNS_SOFT:
		err4 = DNS_SOFT;
		break;
	case DNS_HARD:
		err4 = DNS_HARD;
		break;
	default:
		while ((r = findip(T_A)) != 2)
		{
			ix.af = AF_INET;
			ix.addr.ip = ip;
			ix.pref = pref;
			if (r == DNS_SOFT)
			{
				err4 = DNS_SOFT;
				break;
			}
			if (r == 1)
			{
#ifdef NO_VERISIGN_WILDCARD
				if (!byte_diff((char *) &tldip, sizeof(tldip), (char *) &ip))
					continue;
#endif
#ifdef TLS
				ix.fqdn = glue.s;
#endif
				if (!ipalloc_append(ia, &ix))
				{
					err4 = DNS_MEM;
					break;
				}
			}
		}
		break;
	} /*- switch(resolve(sa,T_A)) */
#ifdef TLS
	if (glue.s)
	{
		alloc_free(glue.s);
		glue.s = 0;
	}
#endif
#ifdef IPV6
	if (err4 != 0 && err6 != 0)
		return err4;
	return 0;
#else
	return err4;
#endif
}

int
dns_ip(ia, sa)
	ipalloc        *ia;
	stralloc       *sa;
{
	if (!ipalloc_readyplus(ia, 0))
		return DNS_MEM;
	ia->len = 0;
	return dns_ipplus(ia, sa, 0);
}

int
dns_mxip(ia, sa, random)
	ipalloc        *ia;
	stralloc       *sa;
	unsigned long   random;
{
	int             r;
	struct mx
	{
		stralloc        sa;
		unsigned short  p;
	} *mx;
#ifdef TLS
	struct ip_mx    ix = {0, {{"\0"}}, 0, 0};
#else
	struct ip_mx    ix = {0, {{"\0"}}, 0};
#endif
	int             nummx;
	int             i;
	int             j;
	int             flagsoft;

	if (!ipalloc_readyplus(ia, 0))
		return DNS_MEM;
	ia->len = 0;

	if (!stralloc_copy(&glue, sa))
		return DNS_MEM;
	if (!stralloc_0(&glue))
		return DNS_MEM;
	if (glue.s[0])
	{
		ix.af = AF_INET;
		ix.pref = 0;
		if (!glue.s[ip_scan(glue.s, &ix.addr.ip)] || !glue.s[ip_scanbracket(glue.s, &ix.addr.ip)])
		{
			if (!ipalloc_append(ia, &ix))
				return DNS_MEM;
			return 0;
		}
	}
	switch (resolve(sa, T_MX))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return dns_ip(ia, sa);
	}
	if (!(mx = (struct mx *) alloc(numanswers * sizeof(struct mx))))
		return DNS_MEM;
	nummx = 0;
	while ((r = findmx(T_MX)) != 2)
	{
		if (r == DNS_SOFT)
		{
			alloc_free((char *) mx);
			return DNS_SOFT;
		}
		if (r == 1)
		{
			mx[nummx].p = pref;
			mx[nummx].sa.s = 0;
			if (!stralloc_copys(&mx[nummx].sa, name))
			{
				while (nummx > 0)
					alloc_free(mx[--nummx].sa.s);
				alloc_free((char *) mx);
				return DNS_MEM;
			}
			++nummx;
		}
	}
	if (!nummx)
		return dns_ip(ia, sa);	/*- e.g., CNAME -> A */
	flagsoft = 0;
	while (nummx > 0)
	{
		unsigned long   numsame;

		i = 0;
		numsame = 1;
		for (j = 1; j < nummx; ++j)
		{
			if (mx[j].p < mx[i].p)
			{
				i = j;
				numsame = 1;
			} else
			if (mx[j].p == mx[i].p)
			{
				++numsame;
				random = random * 69069 + 1;
				if ((random / 2) < (2147483647 / numsame))
					i = j;
			}
		}
		switch (dns_ipplus(ia, &mx[i].sa, mx[i].p))
		{
		case DNS_MEM:
		case DNS_SOFT:
			flagsoft = 1;
			break;
		}
		alloc_free(mx[i].sa.s);
		mx[i] = mx[--nummx];
	}
	alloc_free((char *) mx);
	return flagsoft;
}

static int
findstring(wanttype)
	int             wanttype;
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME);
	if (i < 0)
		return DNS_SOFT;
	responsepos += i;
	i = responseend - responsepos;
	if (i < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype)
	{
		i = *responsepos;
		if (i > MAXDNAME - 1)
			return DNS_SOFT;
		if (responsepos + i > responseend)
			return DNS_SOFT;
		byte_copy(name, i, (char *) (responsepos + 1));
		name[i] = '\0';
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

int
dns_maps(sa, ip, suffix)
	stralloc       *sa;
#ifdef IPV6
	ip6_addr       *ip;
#else
	ip_addr        *ip;
#endif
	char           *suffix;
{
	int             r;

	/*
	 * First, look for a TXT type.  If it is found, we will skip looking
	 * for an A record.
	 */
#ifdef IPV6
	if (!stralloc_ready(sa, iaafmt6(NULL, ip, suffix)))
#else
	if (!stralloc_ready(sa, iaafmt(NULL, ip, suffix)))
#endif
		return DNS_MEM;
#ifdef IPV6
	sa->len = iaafmt6(sa->s, ip, suffix);
#else
	sa->len = iaafmt(sa->s, ip, suffix);
#endif
	switch (resolve(sa, T_TXT))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findstring(T_TXT)) != 2)
	{
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1)
		{
			if (!stralloc_copys(sa, name))
				return DNS_MEM;
			return 0;
		}
	}
	/*
	 * ok, look for an CNAME or A record, and return a more generic message.
	 * Yick.
	 */
	switch (resolve(sa, T_A))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findip(T_A)) != 2)
	{
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1)
		{
			if (!stralloc_copys(sa, "This host is blackholed.  No further information is known. "))
				return DNS_MEM;
			if (!stralloc_cats(sa, "["))
				return DNS_MEM;
			if (!stralloc_cats(sa, suffix))
				return DNS_MEM;
			if (!stralloc_cats(sa, "]"))
				return DNS_MEM;
			return 0;
		}
	}
	return DNS_HARD;
}

void
getversion_dns_c()
{
	static char    *x = "$Id: dns.c,v 1.27 2012-10-09 18:09:20+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
