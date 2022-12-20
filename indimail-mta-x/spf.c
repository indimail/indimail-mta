/*
 * $Id: spf.c,v 1.21 2022-12-20 22:59:58+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef USE_SPF
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "stralloc.h"
#include "strsalloc.h"
#include "alloc.h"
#include "ip.h"
#include "ipalloc.h"
#include "ipme.h"
#include "str.h"
#include "fmt.h"
#include "scan.h"
#include "byte.h"
#include "now.h"
#include "dns.h"
#include "case.h"
#include "spf.h"

#define SPF_EXT    -1
#define SPF_SYNTAX -2

#define WSPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n')
#define NXTOK(b, p, a) do { (b) = (p); \
          while((p) < (a)->len && !WSPACE((a)->s[(p)])) ++(p); \
          while((p) < (a)->len && WSPACE((a)->s[(p)])) (a)->s[(p)++] = 0; \
        } while(0)

/*
 * this table and macro came from wget more or less 
 * and was in turn stolen by me from libspf as is :) 
 */
const static unsigned char urlchr_table[256] = {
	1, 1, 1, 1, 1, 1, 1, 1,		/*- NUL SOH STX ETX  EOT ENQ ACK BEL */
	1, 1, 1, 1, 1, 1, 1, 1,		/*- BS  HT  LF  VT   FF  CR  SO  SI  */
	1, 1, 1, 1, 1, 1, 1, 1,		/*- DLE DC1 DC2 DC3  DC4 NAK SYN ETB */
	1, 1, 1, 1, 1, 1, 1, 1,		/*- CAN EM  SUB ESC  FS  GS  RS  US  */
	1, 0, 1, 1, 0, 1, 1, 0,		/*- SP  !   "   #    $   %   &   '   */
	0, 0, 0, 1, 0, 0, 0, 1,		/*- (   )   *   +    ,   -   .   /   */
	0, 0, 0, 0, 0, 0, 0, 0,		/*- 0   1   2   3    4   5   6   7   */
	0, 0, 1, 1, 1, 1, 1, 1,		/*- 8   9   :   ;    <   =   >   ?   */
	1, 0, 0, 0, 0, 0, 0, 0,		/*- @   A   B   C    D   E   F   G   */
	0, 0, 0, 0, 0, 0, 0, 0,		/*- H   I   J   K    L   M   N   O   */
	0, 0, 0, 0, 0, 0, 0, 0,		/*- P   Q   R   S    T   U   V   W   */
	0, 0, 0, 1, 1, 1, 1, 0,		/*- X   Y   Z   [    \   ]   ^   _   */
	1, 0, 0, 0, 0, 0, 0, 0,		/*- `   a   b   c    d   e   f   g   */
	0, 0, 0, 0, 0, 0, 0, 0,		/*- h   i   j   k    l   m   n   o   */
	0, 0, 0, 0, 0, 0, 0, 0,		/*- p   q   r   s    t   u   v   w   */
	0, 0, 0, 1, 1, 1, 1, 1,		/*- x   y   z   {    |   }   ~   DEL */

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};


strsalloc       ssa = { 0 };
static stralloc sa = { 0 };
static stralloc domain = { 0 };
static ipalloc  ia = { 0 };

extern stralloc addr;
extern stralloc helohost;
extern char    *localhost;

extern stralloc spflocal;
extern stralloc spfguess;
extern stralloc spfexp;

static stralloc sender_fqdn = { 0 };
static stralloc explanation = { 0 };
static stralloc expdomain = { 0 };
static stralloc errormsg = { 0 };
static char    *received;

static int      recursion;
static ip_addr  ip;
#ifdef IPV6
static ip6_addr ip6;
static int      ipv6use;
#endif

static int      spf_ptr(char *spec, char *mask);
static int      spflookup(stralloc * domain);

static void
hdr_pass()
{
	received = "pass (%{xr}: %{xs} designates %{i} as permitted sender)";
}

static void
hdr_softfail()
{
	received = "softfail (%{xr}: transitioning %{xs} does not designate %{i} as permitted sender)";
}

static void
hdr_fail()
{
	received = "fail (%{xr}: %{xs} does not designate %{i} as permitted sender)";
}

static void
hdr_unknown()
{
	received = "unknown (%{xr}: domain at %{d} does not designate permitted sender hosts)";
}

static void
hdr_neutral()
{
	received = "neutral (%{xr}: %{i} is neither permitted nor denied by %{xs})";
}

static void
hdr_none()
{
	received = "none (%{xr}: domain at %{d} does not designate permitted sender hosts)";
}

static void
hdr_unknown_msg(e)
	char           *e;
{
	stralloc_copys(&errormsg, e);
	received = "unknown (%{xr}: %{xe})";
}

static void
hdr_ext(e)
	char           *e;
{
	stralloc_copys(&errormsg, e);
	received = "unknown %{xe} (%{xr}: %{xs} uses mechanism not recognized by this client)";
}

static void
hdr_syntax()
{
	received = "unknown (%{xr}: parse error in %{xs})";
}

static void
hdr_error(e)
	char           *e;
{
	stralloc_copys(&errormsg, e);
	received = "error (%{xr}: error in processing during lookup of %{d}: %{xe})";
}

static void
hdr_dns()
{
	hdr_error("DNS problem");
}

#ifdef IPV6
static void
hdr_addr_family()
{
	hdr_error("unknown address family");
}
#endif


#ifdef IPV6
static int
matchip6(ip6_addr *net, int mask, ip6_addr *i6)
{
	int             j;
	int             bytemask;

	for (j = 0; j < 16 && mask > 0; ++j) {
		if (mask > 8)
			bytemask = 8;
		else
			bytemask = mask;
		mask -= bytemask;
		if ((net->d[j] ^ i6->d[j]) & (0x100 - (1 << (8 - bytemask))))
			return 0;
	}
	return 1;
}

static int
getipmask(char *mask, int *ip4mask, int *ip6mask)
{
	unsigned long   r;
	int             pos;

	if (!mask) {
		*ip4mask = 32;
		*ip6mask = 128;
		return 0;
	}
	pos = scan_ulong(mask, &r);
	if (!pos || (mask[pos] && !(mask[pos] == '/')))
		return -1;
	if (r > 32)
		return -1;
	*ip4mask = r;
	if (mask[pos] && mask[pos] == '/') {
		pos = scan_ulong(mask + pos + 1, &r);
		if (!pos)
			return -1;
		if (r > 128)
			return -1;
		*ip6mask = r;
	}
	return r;
}

static int
getip4mask(char *mask)
{
	unsigned long   r;
	int             pos;

	if (!mask)
		return 32;
	pos = scan_ulong(mask, &r);
	if (!pos)
		return -1;
	if (r > 32)
		return -1;
	return r;
}

static int
getip6mask(char *mask)
{
	unsigned long   r;
	int             pos;

	if (!mask)
		return 128;
	pos = scan_ulong(mask, &r);
	if (!pos)
		return -1;
	if (r > 128)
		return -1;
	return r;
}
#else
static int
getipmask(char *mask, int ipv6)
{
	unsigned long   r;
	int             pos;

	if (!mask)
		return 32;

	pos = scan_ulong(mask, &r);
	if (!pos || (mask[pos] && !(mask[pos] == '/' && ipv6)))
		return -1;
	if (r > 32)
		return -1;

	return r;
}
#endif

static int
matchip(ip_addr *net, int mask, ip_addr *ipaddr)
{
	int             j;
	int             bytemask;

	for (j = 0; j < 4 && mask > 0; ++j) {
		if (mask > 8)
			bytemask = 8;
		else
			bytemask = mask;
		mask -= bytemask;
		if ((net->d[j] ^ ipaddr->d[j]) & (0x100 - (1 << (8 - bytemask))))
			return 0;
	}
	return 1;
}

static void
ssa_free()
{
	int             j;

	for (j = 0;j < ssa.len;++j) {
		if (ssa.sa[j].a) {
			ssa.sa[j].a = 0;
			alloc_free(ssa.sa[j].s);
		}
	}
	ssa.len = 0;
}

int
spfget(stralloc *spf, stralloc *domain_v)
{
	int             j, begin, pos, i, r = SPF_NONE;

	spf->len = 0;
	ssa.len = 0;
	switch (dns_txt(&ssa, domain_v))
	{
	case DNS_MEM:
		return SPF_NOMEM;
	case DNS_SOFT:
		hdr_dns();
		return SPF_ERROR;
	case DNS_HARD:
		return SPF_NONE;
	}
	for (j = 0; j < ssa.len; ++j) {
		pos = 0;
		NXTOK(begin, pos, &ssa.sa[j]);
		if (str_len(ssa.sa[j].s + begin) < 6)
			continue;
		if (!byte_equal(ssa.sa[j].s + begin, 6, "v=spf1"))
			continue;
		if (ssa.sa[j].s[begin + 6]) {
			/*- check for subversion */
			if (ssa.sa[j].s[begin + 6] != '.')
				continue;
			for (i = begin + 7;; ++i)
				if (!(ssa.sa[j].s[i] >= '0' && ssa.sa[j].s[i] <= '9'))
					break;
			if (i == (begin + 7))
				continue;
			if (ssa.sa[j].s[i])
				continue;
		}
		if (spf->len > 0) {
			spf->len = 0;
			hdr_unknown_msg("Multiple SPF records returned");
			r = SPF_UNKNOWN;
			break;
		}
		if (!stralloc_0(&ssa.sa[j]) ||
				!stralloc_copys(spf, ssa.sa[j].s + pos)) {
			ssa_free();
			return SPF_NOMEM;
		}
		r = SPF_OK;
	}
	ssa_free();
	return r;
}

int
spfsubst(stralloc *expand, char *spec, char *domain_p)
{
	static char     hexdigits[] = "0123456789abcdef";
	char            ch;
	int             digits = -1;
	int             urlencode = 0;
	int             reverse = 0;
	int             start = expand->len;
	int             i, pos;
	char           *split = ".";

	if (!stralloc_readyplus(&sa, 0))
		return 0;
	if (*spec == 'x') {
		i = 1;
		++spec;
	} else
		i = 0;
	ch = *spec++;
	if (!ch)
		return 1;
	if (ch >= 'A' && ch <= 'Z') {
		ch += 32;
		urlencode = 1;
	}
	if (i)
		ch -= 32;
	while (*spec >= '0' && *spec <= '9') {
		if (digits < 0)
			digits = 0;
		if (digits >= 1000000) {
			digits = 10000000;
			continue;
		}
		digits = (digits * 10) + (*spec - '0');
		spec++;
	}
	while ((*spec >= 'a' && *spec <= 'z') || (*spec >= 'A' && *spec <= 'Z')) {
		if (*spec == 'r')
			reverse = 1;
		spec++;
	}
	if (*spec)
		split = spec;
	switch (ch)
	{
	case 'l':
		pos = byte_rchr(addr.s, addr.len, '@');
		if (pos < addr.len) {
			if (!stralloc_copyb(&sa, addr.s, pos))
				return 0;
		} else
		if (!stralloc_copys(&sa, "postmaster"))
			return 0;
		break;
	case 's':
		if (!stralloc_copys(&sa, addr.s))
			return 0;
		break;
	case 'o':
		pos = byte_rchr(addr.s, addr.len, '@') + 1;
		if (pos > addr.len)
			break;
		if (!stralloc_copys(&sa, addr.s + pos))
			return 0;
		break;
	case 'd':
		if (!stralloc_copys(&sa, domain_p))
			return 0;
		break;
	case 'i':
		if (!stralloc_ready(&sa, IPFMT))
			return 0;
#ifdef IPV6
		if (ipv6use)
			sa.len = ip6_fmt(sa.s, &ip6);
		else
			sa.len = ip4_fmt(sa.s, &ip);
#else
		sa.len = ip4_fmt(sa.s, &ip);
#endif
		break;
	case 't':
		if (!stralloc_ready(&sa, FMT_ULONG))
			return 0;
		sa.len = fmt_ulong(sa.s, (unsigned long) now());
		break;
	case 'p':
		if (!sender_fqdn.len)
			spf_ptr(domain_p, 0);
		if (sender_fqdn.len) {
			if (!stralloc_copy(&sa, &sender_fqdn))
				return 0;
		} else
		if (!stralloc_copys(&sa, "unknown"))
			return 0;
		break;
	case 'v':
		if (!stralloc_copys(&sa, "in-addr"))
			return 0;
		break;
	case 'h':
		if (!stralloc_copys(&sa, helohost.s))
			return 0;			/*- FIXME: FQDN?  */
		break;
	case 'E':
		if (errormsg.len && !stralloc_copy(&sa, &errormsg))
			return 0;
		break;
	case 'R':
		if (!stralloc_copys(&sa, localhost))
			return 0;
		break;
	case 'S':
		if (expdomain.len > 0) {
			if (!stralloc_copys(&sa, "SPF record at "))
				return 0;
			if (!stralloc_cats(&sa, expdomain.s))
				return 0;
		} else {
			if (!stralloc_copys(&sa, "local policy"))
				return 0;
		}
		break;
	}
	if (reverse) {
		for (pos = 0; digits; ++pos) {
			pos += byte_cspn(sa.s + pos, sa.len - pos, split);
			if (pos >= sa.len)
				break;
			if (!--digits)
				break;
		}
		for (; pos > 0; pos = i - 1) {
			i = byte_rcspn(sa.s, pos, split) + 1;
			if (i > pos)
				i = 0;
			if (!stralloc_catb(expand, sa.s + i, pos - i))
				return 0;
			if (i > 0 && !stralloc_append(expand, "."))
				return 0;
		}
	} else {
		for (pos = sa.len; digits; --pos) {
			i = byte_rcspn(sa.s, pos, split) + 1;
			if (i > pos) {
				pos = 0;
				break;
			}
			pos = i;
			if (!--digits)
				break;
		}
		if (!stralloc_catb(expand, sa.s + pos, sa.len - pos))
			return 0;
		if (split[0] != '.' || split[1])
			for (pos = 0; pos < expand->len; pos++) {
				pos += byte_cspn(expand->s + pos, expand->len - pos, split);
				if (pos < expand->len)
					expand->s[pos] = '.';
			}
	}
	if (urlencode) {
		stralloc_copyb(&sa, expand->s + start, expand->len - start);
		expand->len = start;

		for (pos = 0; pos < sa.len; ++pos) {
			ch = sa.s[pos];
			if (urlchr_table[(unsigned char) ch]) {
				if (!stralloc_readyplus(expand, 3))
					return 0;
				expand->s[expand->len++] = '%';
				expand->s[expand->len++] = hexdigits[(unsigned char) ch >> 4];
				expand->s[expand->len++] = hexdigits[(unsigned char) ch & 15];
			} else
			if (!stralloc_append(expand, &ch))
				return 0;
		}
	}
	return 1;
}

int
spfexpand(stralloc *sa_p, char *spec, char *domain_p)
{
	char           *p;
	char            append;
	int             pos;

	if (!stralloc_readyplus(sa_p, 0))
		return 0;
	sa_p->len = 0;

	for (p = spec; *p; p++) {
		append = *p;
		if (*p == '%') {
			p++;
			switch (*p)
			{
			case '%':
				break;
			case '_':
				append = ' ';
				break;
			case '-':
				if (!stralloc_cats(sa_p, "%20"))
					return 0;
				continue;
			case '{':
				pos = str_chr(p, '}');
				if (p[pos] != '}') {
					p--;
					break;
				}
				p[pos] = 0;
				if (!spfsubst(sa_p, p + 1, domain_p))
					return 0;
				p += pos;
				continue;
			default:
				p--;
			}
		}
		if (!stralloc_append(sa_p, &append))
			return 0;
	}
	return 1;
}

static int
spf_include(char *spec, char *mask)
{
	int             r;

	if (!stralloc_copys(&sa, spec))
		return SPF_NOMEM;
	switch ((r = spflookup(&sa)))
	{
	case SPF_NONE:
		hdr_unknown();
		r = SPF_UNKNOWN;
		break;
	case SPF_SYNTAX:
		r = SPF_UNKNOWN;
		break;
	case SPF_NEUTRAL:
	case SPF_SOFTFAIL:
	case SPF_FAIL:
		r = SPF_NONE;
		break;
	}
	return r;
}

static int
spf_a(char *spec, char *mask)
{
#ifdef IPV6
	int             ip4mask = 0;
	int             ip6mask = 0;
#else
	int             ipmask;
#endif
	int             r;
	int             j;

#ifdef IPV6
	if ((r = getipmask(mask, &ip4mask, &ip6mask)) < 0)
		return SPF_SYNTAX;
#else
	if ((ipmask = getipmask(mask, 1)) < 0)
		return SPF_SYNTAX;
#endif
	if (!stralloc_copys(&sa, spec))
		return SPF_NOMEM;
	if (!ipalloc_readyplus(&ia, 0))
		return SPF_NOMEM;
	switch (dns_ip(&ia, &sa))
	{
	case DNS_MEM:
		return SPF_NOMEM;
	case DNS_SOFT:
		hdr_dns();
		r = SPF_ERROR;
		break;
	case DNS_HARD:
		r = SPF_NONE;
		break;
	default:
		r = SPF_NONE;
		for (j = 0; j < ia.len; ++j) {
#ifdef IPV6
			if (ia.ix[j].af == AF_INET) {
				if (matchip(&ia.ix[j].addr.ip, ip4mask, &ip)) {
					r = SPF_OK;
					break;
				}
			} else
			if (ia.ix[j].af == AF_INET6) {
				if (matchip6(&ia.ix[j].addr.ip6, ip6mask, &ip6)) {
					r = SPF_OK;
					break;
				}
			} else  { /* else unknown address family */
				hdr_addr_family();
				r = SPF_NONE;
			}
#else
			if (matchip(&ia.ix[j].addr.ip, ipmask, &ip)) {
				r = SPF_OK;
				break;
			}
#endif
		}
	}
	return r;
}

static int
spf_mx(char *spec, char *mask)
{
#ifdef IPV6
	int             ip4mask = 0;
	int             ip6mask = 0;
#else
	int             ipmask = getipmask(mask, 1);
#endif
	int             random = now() + (getpid() << 16);
	int             r;
	int             j;

#ifdef IPV6
	if ((r = getipmask(mask, &ip4mask, &ip6mask)) < 0)
		return SPF_SYNTAX;
#else
	if (ipmask < 0)
		return SPF_SYNTAX;
#endif
	if (!stralloc_copys(&sa, spec))
		return SPF_NOMEM;
	if (!ipalloc_readyplus(&ia, 0))
		return SPF_NOMEM;
	switch (dns_mxip(&ia, &sa, random))
	{
	case DNS_MEM:
		return SPF_NOMEM;
	case DNS_SOFT:
		hdr_dns();
		r = SPF_ERROR;
		break;
	case DNS_HARD:
		r = SPF_NONE;
		break;
	default:
		r = SPF_NONE;
		for (j = 0; j < ia.len; ++j) {
#ifdef IPV6
			if (ia.ix[j].af == AF_INET) {
				if (matchip(&ia.ix[j].addr.ip, ip4mask, &ip)) {
					r = SPF_OK;
					break;
				}
			} else
			if (ia.ix[j].af == AF_INET6) {
				if (matchip6(&ia.ix[j].addr.ip6, ip6mask, &ip6)) {
					r = SPF_OK;
					break;
				}
			}
#else
			if (matchip(&ia.ix[j].addr.ip, ipmask, &ip)) {
				r = SPF_OK;
				break;
			}
#endif
		}
	}
	return r;
}

static int
spf_ptr(char *spec, char *mask)
{
	int             len = str_len(spec);
	int             r;
	int             j, k;
	int             pos;

	/*- we didn't find host with the matching ip before */
	if (sender_fqdn.len == 7 && str_equal(sender_fqdn.s, "unknown"))
		return SPF_NONE;
	/*- the hostname found will probably be the same as before */
	while (sender_fqdn.len) {
		pos = sender_fqdn.len - len;
		if (pos < 0)
			break;
		if (pos > 0 && sender_fqdn.s[pos - 1] != '.')
			break;
		if (case_diffb(sender_fqdn.s + pos, len, spec))
			break;

		return SPF_OK;
	}
	/*- ok, either it's the first test or it's a very weird setup */
	if (!strsalloc_readyplus(&ssa, 0))
		return SPF_NOMEM;
	if (!ipalloc_readyplus(&ia, 0))
		return SPF_NOMEM;
	switch (dns_ptr(&ssa, &ip))
	{
	case DNS_MEM:
		return SPF_NOMEM;
	case DNS_SOFT:
		hdr_dns();
		r = SPF_ERROR;
		break;
	case DNS_HARD:
		r = SPF_NONE;
		break;
	default:
		r = SPF_NONE;
		for (j = 0; j < ssa.len; ++j) {
			switch (dns_ip(&ia, &ssa.sa[j]))
			{
			case DNS_MEM:
				ssa_free();
				return SPF_NOMEM;
			case DNS_SOFT:
				hdr_dns();
				r = SPF_ERROR;
				break;
			case DNS_HARD:
				break;
			default:
				for (k = 0; k < ia.len; ++k) {
#ifdef IPV6
					if (ia.ix[k].af == AF_INET) {
						if (matchip(&ia.ix[k].addr.ip, 32, &ip)) {
							if (!sender_fqdn.len && !stralloc_copy(&sender_fqdn, &ssa.sa[j]))
									return SPF_NOMEM;
							pos = ssa.sa[j].len - len;
							if (pos < 0)
								continue;
							if (pos > 0 && ssa.sa[j].s[pos - 1] != '.')
								continue;
							if (case_diffb(ssa.sa[j].s + pos, len, spec))
								continue;
							if (!stralloc_copy(&sender_fqdn, &ssa.sa[j]))
								return SPF_NOMEM;
							r = SPF_OK;
							break;
						}
					} else
					if (ia.ix[k].af == AF_INET6 ) {
						if (matchip6(&ia.ix[k].addr.ip6, 128, &ip6)) {
							if (!sender_fqdn.len && !stralloc_copy(&sender_fqdn, &ssa.sa[j]))
								return SPF_NOMEM;
							pos = ssa.sa[j].len - len;
							if (pos < 0)
								continue;
							if (pos > 0 && ssa.sa[j].s[pos - 1] != '.')
								continue;
							if (case_diffb(ssa.sa[j].s + pos, len, spec))
								continue;
							if (!stralloc_copy(&sender_fqdn, &ssa.sa[j]))
								return SPF_NOMEM;
							r = SPF_OK;
							break;
						}
					}
#else
					if (matchip(&ia.ix[k].addr.ip, 32, &ip)) {
						if (!sender_fqdn.len && !stralloc_copy(&sender_fqdn, &ssa.sa[j]))
							return SPF_NOMEM;
						pos = ssa.sa[j].len - len;
						if (pos < 0)
							continue;
						if (pos > 0 && ssa.sa[j].s[pos - 1] != '.')
							continue;
						if (case_diffb(ssa.sa[j].s + pos, len, spec))
							continue;
						if (!stralloc_copy(&sender_fqdn, &ssa.sa[j]))
							return SPF_NOMEM;
						r = SPF_OK;
						break;
					}
#endif
				}
			}
			if (r == SPF_ERROR)
				break;
		} /*- for (j = 0; j < ssa.len; ++j) */
	} /*- switch (dns_ptr(&ssa, &ip)) */
	if (!sender_fqdn.len && !stralloc_copys(&sender_fqdn, "unknown"))
		return SPF_NOMEM;
	ssa_free();
	return r;
}

#ifdef IPV6
static int
spf_ip6(char *spec, char *mask)
{
	ip6_addr        net;
	int             ipmask;

	if ((ipmask = getip6mask(mask)) < 0)
		return SPF_SYNTAX;
	if (!ip6_scan(spec, &net))
		return SPF_SYNTAX;
	if (matchip6(&net, ipmask, &ip6))
		return SPF_OK;
	return SPF_NONE;
}

static int
spf_ip(char *spec, char *mask)
{
	ip_addr         net;
	int             ipmask;

	if ((ipmask = getip4mask(mask)) < 0)
		return SPF_SYNTAX;
	if (!ip4_scan(spec, &net))
		return SPF_SYNTAX;
	if (matchip(&net, ipmask, &ip))
		return SPF_OK;
	return SPF_NONE;
}
#else
static int
spf_ip(char *spec, char *mask)
{
	ip_addr         net;
	int             ipmask = getipmask(mask, 0);

	if (ipmask < 0)
		return SPF_SYNTAX;
	if (!ip4_scan(spec, &net))
		return SPF_SYNTAX;
	if (matchip(&net, ipmask, &ip))
		return SPF_OK;
	return SPF_NONE;
}
#endif

static int
spf_exists(char *spec, char *mask)
{
	int             r;

	if (!stralloc_copys(&sa, spec))
		return SPF_NOMEM;
	if (!ipalloc_readyplus(&ia, 0))
		return SPF_NOMEM;

	switch (dns_ip(&ia, &sa))
	{
	case DNS_MEM:
		return SPF_NOMEM;
	case DNS_SOFT:
		hdr_dns();
		r = SPF_ERROR;
		break;
	case DNS_HARD:
		r = SPF_NONE;
		break;
	default:
		r = SPF_OK;
	}
	return r;
}

static struct mechanisms
{
	char           *mechanism;
	int             (*func) (char *spec, char *mask);
	unsigned int    takes_spec:1;
	unsigned int    takes_mask:1;
	unsigned int    expands:1;
	unsigned int    filldomain:1;
	int             defresult:4;
} mechanisms[] =
{
	{"all", 0, 0, 0, 0, 0, SPF_OK},
	{"include", spf_include, 1, 0, 1, 0, 0},
	{"a", spf_a, 1, 1, 1, 1, 0},
	{"mx", spf_mx, 1, 1, 1, 1, 0},
	{"ptr", spf_ptr, 1, 0, 1, 1, 0},
	{"ip4", spf_ip, 1, 1, 0, 0, 0},
#ifdef IPV6
	{"ip6", spf_ip6, 1, 1, 0, 0, 0},
#else
	{"ip6", 0, 1, 1, 0, 0, SPF_NONE},
#endif
	{"exists", spf_exists, 1, 0, 1, 0, 0},
	{"extension", 0, 1, 1, 0, 0, SPF_EXT},
	{ 0, 0, 1, 1, 0, 0, SPF_EXT}
};

static int
spfmech(char *mechanism, char *spec, char *mask, char *domain_p)
{
	struct mechanisms *mech;
	int             r;
	int             pos;

	for (mech = mechanisms; mech->mechanism; mech++) {
		if (str_equal(mech->mechanism, mechanism))
			break;
	}
	if (mech->takes_spec && !spec && mech->filldomain)
		spec = domain_p;
	if (!mech->takes_spec != !spec)
		return SPF_SYNTAX;
	if (!mech->takes_mask && mask)
		return SPF_SYNTAX;
	if (!mech->func)
		return mech->defresult;
	if (!stralloc_readyplus(&sa, 0))
		return SPF_NOMEM;
	if (mech->expands && spec != domain_p) {
		if (!spfexpand(&sa, spec, domain_p))
			return SPF_NOMEM;
		for (pos = 0; (sa.len - pos) > 255;) {
			pos += byte_chr(sa.s + pos, sa.len - pos, '.');
			if (pos < sa.len)
				pos++;
		}
		sa.len -= pos;
		if (pos > 0)
			byte_copy(sa.s, sa.len, sa.s + pos);
		stralloc_0(&sa);
		spec = sa.s;
	}
	r = mech->func(spec, mask);
	return r;
}

static struct default_aliases
{
	char           *alias;
	int             defret;
} default_aliases[] =
{
	{"allow", SPF_OK},
	{"pass", SPF_OK},
	{"deny", SPF_FAIL},
	{"softdeny", SPF_SOFTFAIL},
	{"fail", SPF_FAIL},
	{"softfail", SPF_SOFTFAIL},
	{"unknown", SPF_NEUTRAL},
	{0, SPF_UNKNOWN}
};

static int
spflookup(stralloc *domain_s)
{
	stralloc        spf_v = { 0 };
	struct default_aliases *da;
	int             Main = !recursion;
	int             local_pos = -1;
	int             r, q;
	int             begin, pos;
	int             i;
	int             prefix;
	int             done;
	int             guessing = 0;
	char           *p;

	/*- fallthrough result */
	if (Main)
		hdr_none();
	if (!stralloc_readyplus(&sa, 0))
		return SPF_NOMEM;
redirect:
	if (++recursion > 20) {
		hdr_unknown_msg("Maximum nesting level exceeded, possible loop");
		return SPF_SYNTAX;
	}
	if (!stralloc_0(domain_s))
		return SPF_NOMEM;
	if (!stralloc_copy(&expdomain, domain_s))
		return SPF_NOMEM;
	if ((r = spfget(&spf_v, domain_s)) == SPF_NONE) {
		if (!Main) {
			alloc_free(spf_v.s);
			return r;
		}
		if (spfguess.len) {
			/*- try to guess */
			guessing = 1;
			if (!stralloc_copys(&spf_v, spfguess.s)) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
			if (!stralloc_append(&spf_v, " ")) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
		} else
			spf_v.len = 0;
		/*- append local rulest */
		if (spflocal.len) {
			local_pos = spf_v.len;
			if (!stralloc_cats(&spf_v, spflocal.s)) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
		}
		if (!stralloc_0(&spf_v)) {
			alloc_free(spf_v.s);
			return SPF_NOMEM;
		}
		expdomain.len = 0;
	} else
	if (r == SPF_OK) {
		if (!stralloc_0(&spf_v)) {
			alloc_free(spf_v.s);
			return SPF_NOMEM;
		}
		if (Main)
			hdr_neutral();
		r = SPF_NEUTRAL;
		/*- try to add local rules before fail all mechs */
		if (Main && spflocal.len) {
			pos = 0;
			p = (char *) 0;
			while (pos < spf_v.len) {
				NXTOK(begin, pos, &spf_v);
				if (!spf_v.s[begin])
					continue;
				if (p && spf_v.s[begin] != *p)
					p = (char *) 0;
				if (!p && (spf_v.s[begin] == '-' || spf_v.s[begin] == '~' || spf_v.s[begin] == '?'))
					p = &spf_v.s[begin];

				if (p && p > spf_v.s && str_equal(spf_v.s + begin + 1, "all")) {
					/*- ok, we can insert the local rules at p */
					local_pos = p - spf_v.s;
					stralloc_readyplus(&spf_v, spflocal.len);
					p = spf_v.s + local_pos;
					byte_copyr(p + spflocal.len, spf_v.len - local_pos, p);
					byte_copy(p, spflocal.len, spflocal.s);
					spf_v.len += spflocal.len;
					pos += spflocal.len;
					break;
				}
			}
			if (pos >= spf_v.len)
				pos = spf_v.len - 1;
			for (i = 0; i < pos; i++)
				if (!spf_v.s[i])
					spf_v.s[i] = ' ';
		}
	} else {
		alloc_free(spf_v.s);
		return r;
	}
	pos = 0;
	done = 0;
	while (pos < spf_v.len) {
		NXTOK(begin, pos, &spf_v);
		if (!spf_v.s[begin])
			continue;
		/*- in local ruleset?  */
		if (!done && local_pos >= 0 && begin >= local_pos) {
			if (begin < (local_pos + spflocal.len))
				expdomain.len = 0;
			else
			if (!stralloc_copy(&expdomain, domain_s)) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
		}
		for (p = spf_v.s + begin; *p; ++p)
			if (*p == ':' || *p == '/' || *p == '=')
				break;
		if (*p == '=') {
			*p++ = 0;
			/*- modifiers are simply handled here */
			if (str_equal(spf_v.s + begin, "redirect")) {
				if (done)
					continue;
				if (!spfexpand(&sa, p, domain_s->s)) {
					alloc_free(spf_v.s);
					return SPF_NOMEM;
				}
				stralloc_copy(domain_s, &sa);
				hdr_unknown();
				r = SPF_UNKNOWN;
				goto redirect;
			} else
			if (str_equal(spf_v.s + begin, "default")) {
				if (done)
					continue;
				for (da = default_aliases; da->alias; ++da)
					if (str_equal(da->alias, p))
						break;
				r = da->defret;
			} else
			if (str_equal(spf_v.s + begin, "exp")) {
				if (!Main)
					continue;
				if (!stralloc_copys(&sa, p)) {
					alloc_free(spf_v.s);
					return SPF_NOMEM;
				}
				switch (dns_txt(&ssa, &sa))
				{
				case DNS_MEM:
					alloc_free(spf_v.s);
					ssa_free();
					return SPF_NOMEM;
				case DNS_SOFT:
					continue; /*- FIXME...  */
				case DNS_HARD:
					continue;
				}
				explanation.len = 0;
				for (i = 0; i < ssa.len; i++) {
					if (!stralloc_cat(&explanation, &ssa.sa[i])) {
						alloc_free(spf_v.s);
						ssa_free();
						return SPF_NOMEM;
					}
					if (i < (ssa.len - 1) && !stralloc_append(&explanation, "\n")) {
						alloc_free(spf_v.s);
						ssa_free();
						return SPF_NOMEM;
					}
				}
				ssa_free();
				if (!stralloc_0(&explanation)) {
					alloc_free(spf_v.s);
					return SPF_NOMEM;
				}
			}	/*- and unknown modifiers are ignored */
		} else
		if (!done) {
			if (!stralloc_copys(&sa, spf_v.s + begin)) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
			if (!stralloc_0(&sa)) {
				alloc_free(spf_v.s);
				return SPF_NOMEM;
			}
			switch (spf_v.s[begin])
			{
			case '-':
				begin++;
				prefix = SPF_FAIL;
				break;
			case '~':
				begin++;
				prefix = SPF_SOFTFAIL;
				break;
			case '+':
				begin++;
				prefix = SPF_OK;
				break;
			case '?':
				begin++;
				prefix = SPF_NEUTRAL;
				break;
			default:
				prefix = SPF_OK;
			}
			if (*p == '/') {
				*p++ = 0;
				q = spfmech(spf_v.s + begin, 0, p, domain_s->s);
			} else {
				if (*p)
					*p++ = 0;
				i = str_chr(p, '/');
				if (p[i] == '/') {
					p[i++] = 0;
					q = spfmech(spf_v.s + begin, p, p + i, domain_s->s);
				} else
				if (i > 0)
					q = spfmech(spf_v.s + begin, p, 0, domain_s->s);
				else
					q = spfmech(spf_v.s + begin, 0, 0, domain_s->s);
			}
			if (q == SPF_OK)
				q = prefix;
			switch (q)
			{
			case SPF_OK:
				hdr_pass();
				break;
			case SPF_NEUTRAL:
				hdr_neutral();
				break;
			case SPF_SYNTAX:
				hdr_syntax();
				break;
			case SPF_SOFTFAIL:
				hdr_softfail();
				break;
			case SPF_FAIL:
				hdr_fail();
				break;
			case SPF_EXT:
				hdr_ext(sa.s);
				break;
			case SPF_ERROR:
				if (!guessing)
					break;
				if (local_pos >= 0 && begin >= local_pos)
					break;
				hdr_none();
				q = SPF_NONE;
				break;
			case SPF_NONE:
				continue;
			}
			r = q;
			done = 1;	/*- we're done, no more mechanisms */
		}
	}
	/*- we fell through, no local rule applied */
	if (!done && !stralloc_copy(&expdomain, domain_s)) {
		alloc_free(spf_v.s);
		return SPF_NOMEM;
	}
	alloc_free(spf_v.s);
	return r;
}

int
spfcheck(char *remoteip)
{
	int             pos;
	int             r;

	pos = byte_rchr(addr.s, addr.len, '@') + 1;
	if (pos < addr.len) {
		if (!stralloc_copys(&domain, addr.s + pos))
			return SPF_NOMEM;
	} else {
		pos = str_rchr(helohost.s, '@');
		if (helohost.s[pos]) {
			if (!stralloc_copys(&domain, helohost.s + pos + 1))
				return SPF_NOMEM;
		} else
		if (!stralloc_copys(&domain, helohost.s))
			return SPF_NOMEM;
	}
	if (!stralloc_copys(&explanation, spfexp.s))
		return SPF_NOMEM;
	if (!stralloc_0(&explanation))
		return SPF_NOMEM;
	recursion = 0;
#ifdef IPV6
	if (!remoteip) {
		hdr_unknown_msg("No IP address in conversation");
		return SPF_UNKNOWN;
	}
	ipv6use = 0;
	if (!ip4_scan(remoteip, &ip)) {
		ipv6use = 1;
		if (!ip6_scan(remoteip, &ip6)) {
			hdr_unknown_msg("No IP address in conversation");
			return SPF_UNKNOWN;
		}
	}
#else
	if (!remoteip || !ip4_scan(remoteip, &ip)) {
		hdr_unknown_msg("No IP address in conversation");
		return SPF_UNKNOWN;
	}
#endif
	if (!stralloc_readyplus(&expdomain, 0))
		return SPF_NOMEM;
	if (!stralloc_readyplus(&errormsg, 0))
		return SPF_NOMEM;
	expdomain.len = 0;
	errormsg.len = 0;
	sender_fqdn.len = 0;
	received = (char *) 0;
#ifdef IPV6
	if (ipv6use) {
		if (byte_equal((char *) ip6.d, 16, (char *) V6loopback) || ipme_is6(&ip6)) {
			hdr_pass();
			r = SPF_OK;
		} else
			r = spflookup(&domain);
	} else {
		if (byte_equal((char *) ip.d, 4, ip4loopback) || ipme_is(&ip)) {
			hdr_pass();
			r = SPF_OK;
		} else
			r = spflookup(&domain);
	}
#else
	if ((ip.d[0] == 127 && ip.d[1] == 0 && ip.d[2] == 0 && ip.d[3] == 1) || ipme_is(&ip)) {
		hdr_pass();
		r = SPF_OK;
	} else
		r = spflookup(&domain);
#endif
	if (r < 0)
		r = SPF_UNKNOWN;
	return r;
}

int
spfexplanation(stralloc *sa_p)
{
	return spfexpand(sa_p, explanation.s, expdomain.s);
}

int
spfinfo(stralloc *sa_p)
{
	stralloc        tmp = { 0 };

	if (!stralloc_copys(&tmp, received))
		return 0;
	if (!stralloc_0(&tmp))
		return 0;
	if (!spfexpand(sa_p, tmp.s, expdomain.s))
		return 0;
	alloc_free(tmp.s);
	return 1;
}
#endif

void
getversion_spf_c()
{
	static char    *x = "$Id: spf.c,v 1.21 2022-12-20 22:59:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: spf.c,v $
 * Revision 1.21  2022-12-20 22:59:58+05:30  Cprogrammer
 * added ssa_free() to free strsalloc ssa variable
 *
 * Revision 1.20  2022-10-14 22:41:49+05:30  Cprogrammer
 * changed variable name for localhost to localhost
 *
 * Revision 1.19  2022-01-30 09:43:47+05:30  Cprogrammer
 * initialize ip4mask, ip6mask variables
 *
 * Revision 1.18  2020-09-16 19:07:04+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.17  2020-05-11 11:18:47+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.16  2018-08-12 00:30:27+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.15  2018-08-12 00:30:03+05:30  Cprogrammer
 * fixes for double free()
 *
 * Revision 1.14  2017-05-16 20:36:31+05:30  Cprogrammer
 * free strsalloc structure
 *
 * Revision 1.13  2015-08-24 19:12:04+05:30  Cprogrammer
 * removed debugging statement
 *
 * Revision 1.12  2015-08-24 19:09:10+05:30  Cprogrammer
 * replaced ip_scan() with ip4_scan(), replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.11  2013-08-13 21:48:56+05:30  Cprogrammer
 * added error message for unknown address family
 *
 * Revision 1.10  2013-08-06 11:15:51+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.9  2012-06-20 18:47:26+05:30  Cprogrammer
 * fixed memory leak after calling strsalloc()
 *
 * Revision 1.8  2012-04-26 18:05:43+05:30  Cprogrammer
 * remove memory leaks
 *
 * Revision 1.7  2012-04-10 20:37:22+05:30  Cprogrammer
 * added remoteip argument (ipv4) to spfcheck()
 *
 * Revision 1.6  2005-06-17 21:50:56+05:30  Cprogrammer
 * replaced struct ip_address with a shorter typdef ip_addr
 *
 * Revision 1.5  2005-06-11 21:32:42+05:30  Cprogrammer
 * change for ipv6 support
 *
 * Revision 1.4  2004-10-22 20:30:40+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-10 12:07:14+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2004-10-09 23:32:57+05:30  Cprogrammer
 * BUG Fixes - Use ipalloc functions instead of stralloc functions for ipalloc variables
 *
 * Revision 1.1  2004-09-05 00:50:06+05:30  Cprogrammer
 * Initial revision
 *
 */
