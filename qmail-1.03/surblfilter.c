/*
 * $Log: $
 */
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "error.h"
#include "str.h"
#include "case.h"
#include "constmap.h"
#include "auto_qmail.h"
#include "stralloc.h"
#include "env.h"
#include "control.h"
#include "strerr.h"
#include "substdio.h"
#include "getln.h"
#include "byte.h"
#include "dns.h"
#include "ip.h"
#include "ipalloc.h"
#include "mess822.h"

#define FATAL "surblfilter: fatal: "

char           *dns_text(char *);

stralloc        line = { 0 };
int             match, debug;
static int      cachelifetime = 300;
stralloc        whitelist = { 0 };
stralloc        surbldomain = { 0 };

/*
 * SURBL: RCPT whitelist. 
 */
stralloc        srw = { 0 };

int             srwok = 0;
struct constmap mapsrw;

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(&ssout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
print_debug(char *arg1, char *arg2, char *arg3)
{
	if (arg1 && substdio_puts(&sserr, arg1) == -1)
		_exit(1);
	if (arg2 && substdio_puts(&sserr, arg2) == -1)
		_exit(1);
	if (arg3 && substdio_puts(&sserr, arg3) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
die_write()
{
	strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(&ssout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2)
	{
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

void
die_nomem()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: out of memory\n");
	substdio_flush(&sserr);
	_exit(1);
}

void
die_soft()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: DNS temporary failure\n");
	_exit(1);
}

void
die_hard()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: DNS permanent failure\n");
	_exit(1);
}

void
die_control()
{
	substdio_flush(&ssout);
	substdio_puts(&sserr, "surblfilter: unable to read controls\n");
	substdio_flush(&sserr);
	_exit(1);
}

/*
 * SURBL: Check surbl rcpt whitelist. 
 */
int
srwcheck(char *arg, int len)
{
	int             j;

	if (!srwok)
		return 0;
	if (constmap(&mapsrw, arg, len))
		return 1;
	if ((j = byte_rchr(arg, len, '@')) < (len - 1))
	{
		if (constmap(&mapsrw, arg + j, len - j))
			return 1;
	}
	return 0;
}

static char    *
uri_decode(char *str, size_t str_len, char **strend)
{
	size_t          i = 0, j = 0, found;
	int             pasthostname = 0;
	char           *str_bits = "\r\n\t \'\"<>()";

	for (i = 0; i < str_len; i++, j++) {
		if (str[i] == '%' || (!pasthostname && str[i] == '=')) {
			if (i + 2 < str_len) {
				if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
					int             c1 = str[i + 1];
					int             c2 = str[i + 2];
					int             num = (	/* first character */
											  ((c1 & 0xF)	/* take right half */
											   +(9 * (c1 >> 6)))	/* add 9 if character is a-f or A-F */
											  <<4	/* pack into the left half of the byte */
						) | (	/* second character */
								(c2 & 0xF)
								+ (9 * (c2 >> 6))
						);		/* leave it as the left half */
					str[j] = tolower(num);
					i += 2;
					continue;
				}
			}
		}
		if (!pasthostname && (str[i] == '?' || str[i] == '/' || str[i] == '\\'))
			pasthostname = 1;
		if (i + 1 < str_len) {
			if (str[i] == '=' && str[i + 1] == '\n') {
				j -= 1;
				i += 1;
				continue;
			}
		}
		if (i + 2 < str_len) {
			if (str[i] == '=' && str[i + 1] == '\r' && str[i + 2] == '\n') {
				j -= 1;
				i += 2;
				continue;
			}
		}
		found = str_chr(str_bits, str[i]);
		if (str_bits[found])
			break;
		str[j] = tolower(str[i]);
	}

	str[j] = '\0';
	*strend = str + j + 1;
	return str;
}

/*
 * Chose this fairly inefficient method (compared to, for instance, a hash table
 * * or a binary search) because it makes the cctld list easy to adapt. 
 */
int
cctld(char *tld)
{
	static const char cctlds[] =
		".ac.ae.ar.at.au.az.bb.bm.br.bs.ca.cn.co.cr.cu.cy.do.ec.eg.fj.ge.gg.gu.hk.hu.id.il.im.in.je.jo.jp.kh.kr.la.lb.lc.lv.ly.mm.mo.mt.mx.my.na.nc.ni.np.nz.pa.pe.ph.pl.py.ru.sg.sh.sv.sy.th.tn.tr.tw.ua.ug.uk.uy.ve.vi.yu.za";
	const char     *ptr;

	for (ptr = cctlds;*ptr;ptr++) {
		if (*ptr == '.') {
			if (!str_diffn((char *) ptr, tld, 3))
				return 1;
		}
	}
	return 0;
}

static void
snipdomain(char **ouri, size_t urilen)
{
	char           *uri = *ouri;
	int             parts = 2;
	size_t          uripos = urilen;
	int             partsreceived = 0;

	while (uripos-- > 0) {
		if (uri[uripos] == '.') {

			if (partsreceived == 0) {
				if (cctld(&uri[uripos]))
					parts = 3;
			}
			partsreceived++;
			if (partsreceived >= parts) {
				uri = uri + uripos + 1;
				break;
			}
		}
	}
	*ouri = uri;
}

/*
 * Returns:
 * -1 on error
 *  0 if domain wasn't cached
 *  1 if domain was cached, and not blacklisted
 *  2 if domain was cached, and blacklisted.
 */
static int
cacheget(char *uri, size_t urilen, char **text)
{
	return (0);
}

/*
 * Returns 0 on success, -1 on error.
 *
 * text == NULL: host not blacklisted
 * text != NULL: host blacklisted, text == reason.
 */
static int
cacheadd(char *uri, size_t urilen, char *text)
{
	return (0);
}

/*
 * I desperately want the same interface for djbdns and the libresolv wrapper.
 * I chose the djbdns interface. 
 */
static int
getdnsip(stralloc *ip, stralloc *domain)
{
	char            x[IPFMT];
	ipalloc         tip = { 0 };
	int             len;

	if (stralloc_copys(ip, "") == 0)
		return -1;
	switch (dns_ip(&tip, domain))
	{
	case DNS_MEM:
		die_nomem();
	case DNS_SOFT:
		die_soft();
	case DNS_HARD:
		die_hard(); /*- bruce willis in action */
	case 1:
		if (tip.len <= 0)
			die_soft();
	}
	switch (tip.ix->af)
	{
#ifdef IPV6
	case AF_INET6:
		len = ip6_fmt(x, &tip.ix->addr.ip6);
		break;
#endif
	case AF_INET:
		len = ip_fmt(x, &tip.ix->addr.ip);
		break;
	default:
		return -1;
	}
	if (!stralloc_copyb(ip, x, len))
		return -1;
	return 0;
}

/*
 * Returns -1 on error.
 * Returns 0 if host does not exist.
 * Returns 1 if host exists.
 */
static int
checkwhitelist(char *hostname)
{
	static stralloc ip = { 0 };
	static stralloc host = { 0 };
	int             token_len, len, hostlen;
	char           *ptr;

	if (!whitelist.len)
		return 0;
	if (!stralloc_copys(&host, hostname))
		die_nomem();
	if (!stralloc_append(&host, "."))
		die_nomem();
	hostlen = host.len;
	for (ptr = whitelist.s, len = 0;len < whitelist.len;)
	{
		len += ((token_len = str_len(ptr)) + 1);
		if (!stralloc_catb(&host, ptr, token_len))
			die_nomem();
		if (!stralloc_0(&host))
			die_nomem();
		if (debug)
			print_debug("checking whitelist: ", host.s, 0);
		host.len--;
		if (getdnsip(&ip, &host) == -1)
			return -1;
		if (ip.len >= 4)
			return 1;
		host.len = hostlen; /*- reset to where we started */
		ptr = whitelist.s + len;
	}
	return 0;
}

static int
checksurbl(char *uri, char *surbldomain, char **text)
{
	static stralloc ip = { 0 };
	static stralloc host = { 0 };

	if (checkwhitelist(uri) == 1)
		return 0;
	if (stralloc_copys(&host, uri) == 0)
		die_nomem();
	if (stralloc_append(&host, ".") == 0)
		die_nomem();
	if (stralloc_cats(&host, surbldomain) == 0)
		die_nomem();
	if (!stralloc_0(&host))
		die_nomem();
	if (debug)
		print_debug("checking blacklist: ", host.s, 0);
	host.len--;
	if (getdnsip(&ip, &host) == -1)
		return -1;
	if (ip.len > 0) {
		if (text != NULL) {
			if ((*text = dns_text(host.s)))
				return 2;
		}
		return 1;
	}
	return 0;
}

/*
 * Returns 0 if URI was erronous.
 *         1 if URI was not blacklisted.
 *         2 if URI was blacklisted.
 */
static int
checkuri(char **ouri, char **text, size_t textlen)
{
	char           *uri = *ouri;
	char           *uriend;
	size_t          urilen = 0;
	ip_addr         ip;
	char            ipuri[IPFMT];
	int             cached, blacklisted, i;

	if (case_diffb(uri, 4, "http"))
		return 0;
	uri += 4;

	/*- Check and skip http[s]?:[/\\][/\\]?  */
	if (*uri == 's')
		uri++;
	if (*uri == ':' && (uri[1] == '/' || uri[1] == '\\'))
		uri += 2;
	else 
		return 0;
	if (*uri == '/' || *uri == '\\')
		uri++;
	if (!isalpha(*uri) && !isdigit(*uri)) {
		return 0;
	}
	uri_decode(uri, textlen, &uriend);
	*ouri = uriend;
	if (debug)
		print_debug("Full URI = ", uriend, 0);
	uri[(urilen = str_cspn(uri, "/\\?"))] = '\0';
	if (uri[i = str_chr(uri, '@')])
		uri += (i + 1);
	uri[i = str_chr(uri, ':')] = 0;
	urilen = str_len(uri);
	if (ip_scan(uri, &ip)) {
		ip_fmt(ipuri, &ip);
		uri = ipuri;
		if (debug)
			print_debug("Proper IP: ", uri, 0);
	} else 
	{
		if (debug)
			print_debug("Full domain: ", uri, 0);
		snipdomain(&uri, urilen);
		if (debug)
			print_debug("       Part: ", uri, 0);
	}
	urilen = str_len(uri);
	cached = 1;
	blacklisted = 0;
	switch (cacheget(uri, urilen, text))
	{
	case 0:
		cached = 0;
		break;
	case 1:
		blacklisted = 0;
		break;
	case 2:
		blacklisted = 1;
		break;
	}
	if (cached == 0) {
		switch (checksurbl(uri, surbldomain.s, text))
		{
		case -1:
			return -1;
		case 0:
			blacklisted = 0;
			break;
		case 1:
			*text = NULL;
			/*- flow through */
		case 2:
			blacklisted = 1;
			break;
		}

		if (*text == NULL && blacklisted)
			*text = "No reason given";
		cacheadd(uri, urilen, *text);
	}
	return (0);
}

#define DEF_SURBL_DOMAIN "multi.surbl.org"

int
main(int argc, char **argv)
{
	char           *x, *srwFn, *reason;
	int             in_header = 1, i, blacklisted;

	if (!(x = env_get("SURBL")))
		return (0);

	if (chdir(auto_qmail) == -1)
		die_control();
	if ((srwok = control_readfile(&srw, srwFn = ((x = env_get("SURBLRCPT")) && *x ? x : "surblrcpt"), 0)) == -1)
		die_control();
	if (srwok && !constmap_init(&mapsrw, srw.s, srw.len, 0))
		die_nomem();
	switch (control_readline(&surbldomain, "surbldomain"))
	{
	case -1:
		die_control();
	case 0:
		if (!stralloc_copys(&surbldomain, DEF_SURBL_DOMAIN))
			die_nomem();
		/*- flow through */
	case 1:
		if (!stralloc_0(&surbldomain))
			die_nomem();
	}
	if (control_readint(&cachelifetime, "cachetime") == -1)
		die_control();
	if (control_readfile(&whitelist, "surbldomainwhite", 0) == -1)
		die_control();
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("getln: ", 0, 1);
		if (!match && line.len == 0)
			break;
		if (!in_header)
		{
			blacklisted = -1;
			for (i = 0;i < line.len; i++)
			{
				if (case_startb(line.s + i, 4, "http:")) {
					x = line.s + i;
					switch (checkuri(&x, &reason, line.len - i))
					{
					case -1:
						my_error("checkuri: ", 0, 1);
					case 0: /*- no valid uri in line */
						continue;
						break;
					case 1:
						blacklisted = 0;
						break;
					case 2:
						if (debug)
							print_debug("blacklisted", line.s, reason);
						blacklisted = 1;
						break;
					}
				}
				if (blacklisted == 1)
					break;
			}
			if (substdio_put(&ssout, line.s, line.len))
				die_write();
			continue;
		}
		if (in_header && !mess822_ok(&line))
			in_header = 0;
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	return (0);
}
