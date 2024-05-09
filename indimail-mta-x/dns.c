/*
 * $Id: dns.c,v 1.39 2022-12-21 12:21:46+05:30 Cprogrammer Exp mbhangui $
 * RCS log at bottom
 */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <errno.h>
#include "ip.h"
#include "byte.h"
#include "ipalloc.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif
#ifdef HASTLSA
#include "tlsarralloc.h"
#endif
#include "fmt.h"
#include "alloc.h"
#include "str.h"
#include "stralloc.h"
#include "dns.h"
#include "case.h"

#define MAX_EDNS_RESPONSE_SIZE 65536

/*-
 * https://slashdot.org/story/03/09/16/0034210/resolving-everything-verisign-adds-wildcards
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
getshort(unsigned char *c)
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
static int      iaafmt6(char *, ip6_addr *, const char *);
#endif
#ifdef HASTLSA
static tlsarr   tlsaRR;
ns_rr           rr;
ns_msg          msg;
#endif

static int
resolve(stralloc *domain, int type)
{
	int             n;
	int             i;

	errno = 0;
	if (!stralloc_copy(&glue, domain))
		return DNS_MEM;
	if (!stralloc_0(&glue))
		return DNS_MEM;
	if (!responsebuflen) {
		if ((response.buf = (unsigned char *) alloc(PACKETSZ + 1)))
			responsebuflen = PACKETSZ + 1;
		else
			return DNS_MEM;
	}
	responselen = lookup(glue.s, C_IN, type, response.buf, responsebuflen);
#ifdef HASTLSA
	ns_initparse(response.buf, responselen, &msg);
#endif
	if ((responselen >= responsebuflen) || (responselen > 0 && (((HEADER *) response.buf)->tc))) {
		if (responsebuflen < MAX_EDNS_RESPONSE_SIZE) {
			if (alloc_re((char *) &response.buf, responsebuflen, MAX_EDNS_RESPONSE_SIZE))
				responsebuflen = MAX_EDNS_RESPONSE_SIZE;
			else
				return DNS_MEM;
		}
		saveresoptions = _res.options;
		_res.options |= RES_USEVC;
		responselen = lookup(glue.s, C_IN, type, response.buf, responsebuflen);
		_res.options = saveresoptions;
	}
	if (responselen <= 0) {
		if (errno == ECONNREFUSED)
			return DNS_SOFT;
		if (h_errno == TRY_AGAIN)
			return DNS_SOFT;
		return DNS_HARD;
	}
	responseend = response.buf + responselen;
	responsepos = response.buf + sizeof(HEADER);
	n = ntohs(((HEADER *) response.buf)->qdcount);
	while (n-- > 0) {
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
findname(int wanttype)
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
	if (rrtype == wanttype) {
		if (dn_expand(response.buf, responseend, responsepos, name, MAXDNAME) < 0)
			return DNS_SOFT;
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

static int
findip(int wanttype)
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
	if (rrtype == wanttype) {
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
findip6(int wanttype)
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
	if (rrtype == wanttype) {
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
findmx(int wanttype)
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
	if ((i = responseend - responsepos) < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype) {
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
dns_init(int flagsearch)
{
	res_init();
	if (flagsearch)
		lookup = res_search;
}

int
dns_cname(stralloc *sa)
{
	int             r;
	int             loop;

	for (loop = 0; loop < 10; ++loop) {
		if (!sa->len)
			return loop;
		if (sa->s[sa->len - 1] == ']')
			return loop;
		if (sa->s[sa->len - 1] == '.') {
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
			while ((r = findname(T_CNAME)) != 2) {
				if (r == DNS_SOFT)
					return DNS_SOFT;
				if (r == 1) {
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
iaafmt(char *s, ip_addr *ip4, const char *dom)
{
	unsigned int    i;
	unsigned int    len;

	len = 0;
	i = fmt_ulong(s, (unsigned long) ip4->d[3]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip4->d[2]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip4->d[1]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, (unsigned long) ip4->d[0]);
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

static stralloc txt = { 0 };

static int
findtxt(int wanttype)
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
	if ((i = responseend - responsepos) < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype) {
		unsigned short  txtpos;
		unsigned char   txtlen;

		txt.len = 0;
		for (txtpos = 0; txtpos < rrdlen; txtpos += txtlen) {
			txtlen = responsepos[txtpos++];
			if (txtlen > rrdlen - txtpos)
				txtlen = rrdlen - txtpos;
			if (!stralloc_catb(&txt, (char *) &responsepos[txtpos], txtlen))
				return DNS_MEM;
		}
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

int
dns_txt(strsalloc *ssa, stralloc *domain)
{
	int             r;

	switch (resolve(domain, T_TXT))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findtxt(T_TXT)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
			stralloc        tmp = {0};

			if (!stralloc_copy(&tmp, &txt) ||
					!strsalloc_append(ssa, &tmp))
				return DNS_MEM;
		}
	}
	if (ssa->len)
		return 0;
	return DNS_HARD;
}

int
dns_ptr(strsalloc *ssa, ip_addr *ip4)
{
	int             r;

	if (!stralloc_ready(&tmpsa, iaafmt((char *) 0, ip4, ".in-addr.arpa.")))
		return DNS_MEM;
	tmpsa.len = iaafmt(tmpsa.s, ip4, ".in-addr.arpa.");
	switch ((r = resolve(&tmpsa, T_PTR)))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
			stralloc        tmp = {0};

			if (!stralloc_copys(&tmp, name) ||
					!strsalloc_append(ssa, &tmp))
				return DNS_MEM;
		}
	}
	if (ssa->len)
		return 0;
	return DNS_HARD;
}
#else
int
dns_ptr(stralloc *sa, ip_addr *ip4)
{
	int             r;

	if (!stralloc_ready(sa, iaafmt((char *) 0, ip4, ".in-addr.arpa.")))
		return DNS_MEM;
	sa->len = iaafmt(sa->s, ip4, ".in-addr.arpa.");
	switch (resolve(sa, T_PTR))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
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
iaafmt6(char *s, ip6_addr *i6, const char *dom)
{
	int             j;
	static char     data[] = "0123456789abcdef";

	if (s) {
		for (j = 15; j >= 0; j--) {
			*s++ = data[i6->d[j] & 0x0f];
			*s++ = '.';
			*s++ = data[(i6->d[j] >> 4) & 0x0f];
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
#ifdef USE_SPF
dns_ptr6(strsalloc *ssa, ip6_addr *i6)
#else
dns_ptr6(stralloc *ssa, ip6_addr *i6)
#endif
{
	int             r;

	if (!stralloc_ready(&tmpsa, iaafmt6((char *) 0, i6, "ip6.int")))
		return DNS_MEM;
	tmpsa.len = iaafmt6(tmpsa.s, i6, "ip6.int");
	switch (resolve(&tmpsa, T_PTR))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findname(T_PTR)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
#ifdef USE_SPF
			stralloc        tmp = {0};
			if (!stralloc_copys(&tmp, name))
				return DNS_MEM;
			if (!strsalloc_append(ssa, &tmp))
				return DNS_MEM;
#else
			if (!stralloc_copys(ssa, name))
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
dns_ipplus(ipalloc *ia, stralloc *sa, int prefv)
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
	if (glue.s[0]) {
		ix.pref = 0;
		ix.af = AF_INET;
		if (!glue.s[ip4_scan(glue.s, &ix.addr.ip)] || !glue.s[ip4_scanbracket(glue.s, &ix.addr.ip)]) {
			if (!ipalloc_append(ia, &ix))
				return DNS_MEM;
			return 0;
		}
	}
#ifdef NO_VERISIGN_WILDCARD
	j = byte_rchr(sa->s,sa->len,'.');
	if (j + 2 < sa->len) {
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
			while ((r = findip(T_A)) != 2) {
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
		while ((r = findip6(T_AAAA)) != 2) {
			ix.af = AF_INET6;
			ix.addr.ip6 = ip6;
			ix.pref = prefv;
			if (r == DNS_SOFT) {
				err6 = DNS_SOFT;
				break;
			}
			if (r == 1) {
#ifdef NO_VERISIGN_WILDCARD
				if (!byte_diff((char *) &tldip, sizeof(tldip), (char *) &ip6))
					continue;
#endif
#ifdef TLS
				if (!(ix.fqdn = (char *) alloc(sizeof(char) * glue.len)))
					return DNS_MEM;
				byte_copy(ix.fqdn, glue.len, glue.s);
#endif
				if (!ipalloc_append(ia, &ix)) {
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
		while ((r = findip(T_A)) != 2) {
			ix.af = AF_INET;
			ix.addr.ip = ip;
			ix.pref = prefv;
			if (r == DNS_SOFT) {
				err4 = DNS_SOFT;
				break;
			}
			if (r == 1) {
#ifdef NO_VERISIGN_WILDCARD
				if (!byte_diff((char *) &tldip, sizeof(tldip), (char *) &ip))
					continue;
#endif
#ifdef TLS
				if (!(ix.fqdn = (char *) alloc(sizeof(char) * glue.len)))
					return DNS_MEM;
				byte_copy(ix.fqdn, glue.len, glue.s);
#endif
				if (!ipalloc_append(ia, &ix)) {
					err4 = DNS_MEM;
					break;
				}
			}
		}
		break;
	} /*- switch(resolve(sa,T_A)) */
#ifdef TLS
	if (glue.s) {
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
dns_ip(ipalloc *ia, stralloc *sa)
{
	if (!ipalloc_readyplus(ia, 0))
		return DNS_MEM;
	ia->len = 0;
	return dns_ipplus(ia, sa, 0);
}

int
dns_mxip(ipalloc *ia, stralloc *sa, unsigned long random)
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
	if (glue.s[0]) {
		ix.af = AF_INET;
		ix.pref = 0;
		if (!glue.s[ip4_scan(glue.s, &ix.addr.ip)] || !glue.s[ip4_scanbracket(glue.s, &ix.addr.ip)]) {
#ifdef TLS
			if (!(ix.fqdn = (char *) alloc(sizeof(char) * glue.len)))
				return DNS_MEM;
			byte_copy(ix.fqdn, glue.len, glue.s);
#endif
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
	while ((r = findmx(T_MX)) != 2) {
		if (r == DNS_SOFT) {
			alloc_free((char *) mx);
			return DNS_SOFT;
		}
		if (r == 1) {
			mx[nummx].p = pref;
			mx[nummx].sa.s = 0;
			if (!stralloc_copys(&mx[nummx].sa, name)) {
				while (nummx > 0)
					alloc_free(mx[--nummx].sa.s);
				alloc_free((char *) mx);
				return DNS_MEM;
			}
			++nummx;
		}
	}
	if (!nummx) {
		alloc_free((char *) mx);
		return dns_ip(ia, sa);	/*- e.g., CNAME -> A */
	}
	flagsoft = 0;
	while (nummx > 0) {
		unsigned long   numsame;

		i = 0;
		numsame = 1;
		for (j = 1; j < nummx; ++j) {
			if (mx[j].p < mx[i].p) {
				i = j;
				numsame = 1;
			} else
			if (mx[j].p == mx[i].p) {
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
findstring(int wanttype)
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
	if ((i = responseend - responsepos) < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype) {
		if ((i = *responsepos) > MAXDNAME - 1)
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
#ifdef IPV6
dns_maps(stralloc *sa, ip6_addr *i6, char *suffix)
#else
dns_maps(stralloc *sa, ip_addr *i4, char *suffix)
#endif
{
	int             r;

	/*-
	 * First, look for a TXT type.  If it is found, we will skip looking
	 * for an A record.
	 */
#ifdef IPV6
	if (!stralloc_ready(sa, iaafmt6(NULL, i6, suffix)))
#else
	if (!stralloc_ready(sa, iaafmt(NULL, i4, suffix)))
#endif
		return DNS_MEM;
#ifdef IPV6
	sa->len = iaafmt6(sa->s, i6, suffix);
#else
	sa->len = iaafmt(sa->s, i4, suffix);
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
	while ((r = findstring(T_TXT)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
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
	while ((r = findip(T_A)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
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

#ifdef HASTLSA
static int
findtlsa(int wanttype)
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;
	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME))  < 0)
		return DNS_SOFT;
	responsepos += i;
	if ((i = responseend - responsepos) < 4 + 3 * 2)
		return DNS_SOFT;
	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;
	if (rrtype == wanttype) {
		if (rrdlen < 4)
			return DNS_SOFT;
		tlsaRR.usage = responsepos[0];
		tlsaRR.selector = responsepos[1];
		tlsaRR.mtype = responsepos[2];
		tlsaRR.data_len = rrdlen - 3;
		if (!(tlsaRR.data = (uint8_t *) alloc(tlsaRR.data_len * sizeof(uint8_t))))
			return DNS_MEM;
		byte_copy((char *) tlsaRR.data, tlsaRR.data_len, (char *) responsepos + 3);
		responsepos += rrdlen;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}
#ifndef T_TLSA
#define T_TLSA 52
#endif

static int
dns_tlsarrplus(tlsarralloc *ta, stralloc *sa)
{
	int             r, i = 0;
	uint32_t        ttl;
	const char     *cp;

	switch (resolve(sa, T_TLSA)) {
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findtlsa(T_TLSA)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		ns_parserr(&msg, ns_s_an, i++, &rr);
		ttl = ns_rr_ttl(rr);
		if (r == 1) {
			cp = ns_rr_name(rr);
			tlsaRR.hostlen = str_len(cp);
			tlsaRR.ttl = ttl;
			if (!(tlsaRR.host = (char *) alloc(tlsaRR.hostlen * sizeof(char))))
				return DNS_MEM;
			byte_copy(tlsaRR.host, tlsaRR.hostlen, cp);
			if (!tlsarralloc_append(ta, &tlsaRR))
				return DNS_MEM;
		} else
			i++;
	}
	return 0;
}

int
dns_tlsarr(tlsarralloc *ta, stralloc *sa)
{
	if (!tlsarralloc_readyplus(ta, 0))
		return DNS_MEM;
	ta->len = 0;
	return dns_tlsarrplus(ta, sa);
}
#endif

void
getversion_dns_c()
{
	const char     *x = "$Id: dns.c,v 1.39 2022-12-21 12:21:46+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: dns.c,v $
 * Revision 1.39  2022-12-21 12:21:46+05:30  Cprogrammer
 * renamed dns_txtplus(), dns_ptrplus() as dns_txt(), dns_ptr()
 *
 * Revision 1.38  2022-12-20 21:38:20+05:30  Cprogrammer
 * fixed double_free with strsalloc_append
 *
 * Revision 1.37  2022-09-29 19:30:38+05:30  Cprogrammer
 * moved RCS log to bottom
 *
 * Revision 1.36  2020-05-11 11:15:49+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.35  2018-06-02 09:58:37+05:30  Cprogrammer
 * fqdn of ip_mx struct was pointing to a location that could be overwritten
 *
 * Revision 1.34  2018-06-01 22:51:10+05:30  Cprogrammer
 * define dns_ptrplus() proto only if USE_SPF is defined
 * set ix.fqdn when quering mx records
 *
 * Revision 1.33  2018-05-28 17:24:20+05:30  Cprogrammer
 * define T_TLSA for older systems where nameser.h doesn'thave T_TLSA defined
 *
 * Revision 1.32  2018-05-26 12:41:25+05:30  Cprogrammer
 * fixed memory leak dns_mxip()
 * added functions for getting TLSA RR.
 *
 * Revision 1.31  2017-05-16 12:30:52+05:30  Cprogrammer
 * refactored dns_txt() code
 *
 * Revision 1.30  2017-05-10 14:59:19+05:30  Cprogrammer
 * increase responselen to 1024 for long text records
 *
 * Revision 1.29  2015-08-24 19:05:12+05:30  Cprogrammer
 * replaced ip_scan() with ip4_scan()
 *
 * Revision 1.28  2014-01-29 13:56:22+05:30  Cprogrammer
 * fix for OS X
 * fixed wrong address passed to resolve()
 *
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
