/*
 * $Log: $
 */
#include <unistd.h>
#include <errno.h>
#include "error.h"
#include "constmap.h"
#include "auto_qmail.h"
#include "stralloc.h"
#include "env.h"
#include "control.h"
#include "strerr.h"
#include "substdio.h"
#include "getln.h"
#include "byte.h"
#include "mess822.h"

#define FATAL "surblfilter: fatal: "

stralloc        line = { 0 };
int             match;
/*
 * SURBL: true if a whitelisted address was found in RCPT TO. 
 */
int             surblwhite = 0;

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

static int      cachelifetime = 300;
stralloc        whitelist = { 0 };

int
main(int argc, char **argv)
{
	char           *x, *srwFn;
	stralloc        domain = { 0 };
	int             in_header = 1;

	if (!(x = env_get("SURBL")))
		return (0);

	if (chdir(auto_qmail) == -1)
		die_control();
	if ((srwok = control_readfile(&srw, srwFn = ((x = env_get("SURBLRCPT")) && *x ? x : "surblrcpt"), 0)) == -1)
		die_control();
	if (srwok && !constmap_init(&mapsrw, srw.s, srw.len, 0))
		die_nomem();
	if (control_readline(&domain, "surbldomain") == -1)
		die_control();
	if (control_readint(&cachelifetime, "cachetime") == -1)
		die_control();
	if (control_readfile(&whitelist, "surbldomainwhite", 0) == -1)
		die_control();
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("qmail-cat: read", 0, 2);
		if (!match && line.len == 0)
			break;
		if (in_header && !mess822_ok(&line))
			in_header = 0;
		if (!in_header)
		{
			if (substdio_put(&ssout, line.s, line.len))
				die_write();
			continue;
		}
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	return (0);
}
/*-------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>

#ifdef SURBLTEST
#include <dns.h>
#else
#include "ipalloc.h"
#include "stralloc.h"
#include "strsalloc.h"
#include "dns.h"
#endif

#include "surbl.h"

#define DEF_SURBL_DOMAIN "multi.surbl.org"

#define SURBL_ROOT_DIR "surbl"

#define SURBL_STORE_DIR SURBL_ROOT_DIR "/store"
#define SURBL_STORE_FILE SURBL_STORE_DIR "/.msgnum"
#define SURBL_STORE_LOCK SURBL_STORE_DIR "/.lock"

#define SURBL_CACHE_DIR SURBL_ROOT_DIR "/cache"
#define SURBL_CACHE_FILE ".cached"
#define SURBL_CACHE_LOCK SURBL_CACHE_DIR "/.lock"

static char     domain[512] = DEF_SURBL_DOMAIN;
static int      cachelifetime = 300;
static stralloc whitelistfile = { 0, 0, 0 };

static char   **whitelist;

static void     store_move(int blacklisted, char *filename, char *msgnumstr);
static int      checkuri(char **ouri, char **text, size_t textlen);
static int      iptrans(char *uri, char *ip);
static int      initwhitelist(void);
static int      getdnsip(stralloc * ip, stralloc * domain);
static int      checkwhitelist(char *hostname);
static int      checksurbl(char *hostname, char *domain, char **text);
static char    *uri_decode(char *str, size_t str_len, char **strend);
static void     snipdomain(char **ouri, size_t urilen);
static int      cacheget(char *uri, size_t urilen, char **text);
static int      cacheadd(char *uri, size_t urilen, char *text);
static int      header_base64(char *header);
static int      filecreate(char *file, char *text);
static int      fileread(char *file, char **buf, size_t * buflen);
static int      fileexist(char *file);
static int      getmodtime(char *file, time_t * modtime);
static int      file_lock(char *lockfile, int excl, int block);
static int      file_unlock(int fd);
static unsigned long revl(unsigned long r4);
static int      bt_str64dec(char *b64, char **endptr, char **text, size_t * textlen, char *ignore);
static int      casencmp(char *str1, char *str2, unsigned long len);


/*
 * Returns -1 on hard error,
 * *          0 if message is not to be blocked,
 * *          1 if message is to be blocked.
 */
int
surblfilter(char *text, size_t textlen, char **msg)
{
	int             tmpi;
	char           *dec = NULL;
	char           *header;
	size_t          headerlen;
	char           *tmp = NULL;
	size_t          tmplen = 0;
	size_t          tmpstrlen;
	int             blacklisted = 0;

	static stralloc filename = { 0, 0, 0 };
	char            msgnumstr[9] = "00000000";
	int             storelock = 0;
	int             storemail = 0;

	if (fileread("control/surbldomain", &tmp, &tmplen) == 0) {
		tmplen = strcspn(tmp, "\r\n\t ");
		tmp[tmplen] = '\0';
		tmp[sizeof domain - 1] = '\0';
		strcpy(domain, tmp);
	}

	if (fileread("control/surblcachetime", &tmp, &tmplen) == 0) {
		char           *tmp2 = NULL;
		long            tmplong = strtol(tmp, &tmp2, 10);
		if ((*tmp2 == '\0' || *tmp2 == '\n') && tmplong > 0)
			cachelifetime = tmplong;
	}

	do {
		if (fileexist(SURBL_STORE_DIR "/busy") <= 0)
			break;
		if (stralloc_copys(&filename, SURBL_STORE_FILE) == 0)
			break;
		if (stralloc_0(&filename) == 0)
			break;

		storelock = file_lock(SURBL_STORE_LOCK, 0, 1);
		if (storelock == -1)
			break;
		tmpi = fileread(filename.s, &tmp, &tmplen);
		if (file_unlock(storelock) == -1)
			break;

		if (tmpi == 0) {
			char           *tmp2 = NULL;
			long            msgnum = strtol(tmp, &tmp2, 10);
			if ((*tmp2 == '\0' || *tmp2 == '\n')
				&& msgnum >= 0 && msgnum <= 99999999)
				sprintf(msgnumstr, "%08ld", msgnum + 1);
		}

		storelock = file_lock(SURBL_STORE_LOCK, 1, 1);
		if (storelock == -1)
			break;
		if (filecreate(filename.s, msgnumstr) == -1)
			break;
		if (file_unlock(storelock) == -1)
			break;

		if (stralloc_copys(&filename, SURBL_STORE_DIR "/busy/") == 0)
			break;
		if (stralloc_cats(&filename, msgnumstr) == 0)
			break;
		if (stralloc_0(&filename) == 0)
			break;

		filecreate(filename.s, text);
		storemail = 1;
	} while (0);

	tmp = text;
	tmplen = textlen;

	if (whitelist == NULL) {
		initwhitelist();
	}

/*
 * Replace all premature '\0' with ' ' 
 */
	while ((tmpstrlen = strlen(tmp)) < tmplen) {
		tmp[tmpstrlen] = ' ';
		tmp += tmpstrlen;
		tmplen -= tmpstrlen;
	}

	header = text;
/*
 * Find header-body seperator 
 */
	if ((tmp = strstr(text, "\n\n")) == NULL && (tmp = strstr(text, "\r\n\r\n")) == NULL) {
	/*
	 * printf ("No mail body found?\n");
	 */
		if (storemail)
			store_move(blacklisted, filename.s, msgnumstr);
		return 0;
	}

/*
 * Store length of the header. 
 */
	headerlen = tmp - text;
	*tmp++ = '\0';
/*
 * Skip all newlines leading the body. 
 */
	tmp += strspn(tmp, "\r\n");
/*
 * Store length of the body. 
 */
	textlen -= tmp - text;
/*
 * Store the body in text. 
 */
	text = tmp;

	if (header_base64(header)) {
		char           *bodyend = NULL;
		size_t          declen = 0;
		if (bt_str64dec(text, &bodyend, &dec, &declen, "\r\n") == -1) {
			if (storemail)
				store_move(blacklisted, filename.s, msgnumstr);
			return -1;
		}
		text = dec;
		textlen = declen;
	}

	while (!blacklisted && (text = strstr(text, "http")) != NULL) {
		char           *reason = NULL;
		switch (checkuri(&text, &reason, textlen)) {
		case 0:
			text++;
		case 1:
			break;
		case 2:				/*printf ("blacklisted: %s.\n", reason); */
			*msg = reason;
			blacklisted = 1;
			break;
		}
		if (!blacklisted)
			free(reason);
	}

	if (storemail)
		store_move(blacklisted, filename.s, msgnumstr);

	return blacklisted;
}

/*
 * Move mail to it's proper place. (spam/ or nospam/)
 */
static void
store_move(int blacklisted, char *filename, char *msgnumstr)
{
	static stralloc filenametmp = { 0, 0, 0 };
	do {
		if (stralloc_copys(&filenametmp, SURBL_STORE_DIR) == 0)
			break;
		if (blacklisted) {
			if (stralloc_cats(&filenametmp, "/spam/") == 0)
				break;
		} else {
			if (stralloc_cats(&filenametmp, "/nospam/") == 0)
				break;
		}
		if (stralloc_cats(&filenametmp, msgnumstr) == 0)
			break;
		if (stralloc_0(&filenametmp) == 0)
			break;
		link(filename, filenametmp.s);
	} while (0);
	unlink(filename);
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
	char            ipuri[64];
	int             cached, blacklisted;
	char           *tmp;

	if (strncmp(uri, "http", 4) != 0) {
		return 0;
	}

	uri += 4;

/*
 * Check and skip http[s]?:[/\\][/\\]? 
 */
	if (*uri == 's')
		uri++;
	if (*uri == ':' && (uri[1] == '/' || uri[1] == '\\'))
		uri += 2;
	else {
		return 0;
	}
	if (*uri == '/' || *uri == '\\')
		uri++;
	if (!isalpha(*uri) && !isdigit(*uri)) {
		return 0;
	}

	uri_decode(uri, textlen, &uriend);

	*ouri = uriend;

/*
 * printf ("\nFull URI   : %s\n", uri);
 */

	uri[(urilen = strcspn(uri, "/\\?"))] = '\0';

	tmp = strchr(uri, '@');
	if (tmp != NULL)
		uri = tmp + 1;

	uri[strcspn(uri, ":")] = '\0';

	urilen = strlen(uri);

	if (iptrans(uri, ipuri)) {
		uri = ipuri;
	/*
	 * printf ("Proper IP  : '%s'\n", uri);
	 */
	} else {
	/*
	 * printf ("Full domain: %s\n", uri);
	 */
		snipdomain(&uri, urilen);
	/*
	 * printf ("       Part: %s\n", uri);
	 */
	}

	urilen = strlen(uri);
	*text = NULL;
	cached = 1;
	blacklisted = 0;

	switch (cacheget(uri, urilen, text)) {
	case -1:
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
		switch (checksurbl(uri, domain, text)) {
		case -1:
			return -1;
		case 0:
			blacklisted = 0;
			break;
		case 1:
			blacklisted = 1;
			*text = NULL;
			break;
		case 2:
			blacklisted = 1;
		}

		if (*text == NULL && blacklisted) {
			char           *tmp = "No reason given";
			*text = malloc(strlen(tmp) + 1);
			if (*text == NULL)
				return 1;
			strcpy(*text, tmp);
		}
		cacheadd(uri, strlen(uri), *text);
	}

/*
 * printf ("Checked '%s': %s blacklisted.\n", uri, blacklisted?"":"not");
 */

	if (blacklisted)
		return 2;
	else
		return 1;
}

/*
 * Returns:
 * 0 if given URI is not an IP
 * 1 if given URI is an IP
 *
 * Handles dotted-decimal notation, base-10 notation and base-16 notation.
 */
static int
iptrans(char *uri, char *ip)
{
	struct in_addr  addr = { 0 };
	unsigned char  *addrp = (unsigned char *) &addr.s_addr;
	int             rv = inet_aton(uri, &addr);
	if (rv != 0) {
		sprintf(ip, "%u.%u.%u.%u", addrp[3], addrp[2], addrp[1], addrp[0]);
		return 1;
	}
	return 0;
}

static int
initwhitelist(void)
{
	FILE           *wlfp;
	char            buf[256];
	size_t          i;
	char           *wlp;
	unsigned int    whitelists = 0;

	if (stralloc_copys(&whitelistfile, "") == 0)
		return -1;

	wlfp = fopen("control/surbldomainwhite", "r");
	if (wlfp == NULL)
		return -1;
	while (fgets(buf, sizeof buf, wlfp) != NULL) {
		if (stralloc_cats(&whitelistfile, buf) == 0) {
			fclose(wlfp);
			return -1;
		}
	}
	fclose(wlfp);

	if (stralloc_0(&whitelistfile) == 0)
		return -1;

	for (i = 0; i < whitelistfile.len; i++) {
		if (whitelistfile.s[i] == '\n') {
			whitelistfile.s[i] = '\0';
			whitelists++;
		}
	}

	whitelist = malloc(sizeof *whitelist * (whitelists + 1));
	if (whitelist == NULL)
		return -1;

	wlp = whitelistfile.s;
	for (i = 0; i < whitelists; i++) {
		whitelist[i] = wlp;
		wlp += strlen(wlp) + 1;
	}
	whitelist[i] = NULL;

	return 0;
}

/*
 * I desperately want the same interface for djbdns and the libresolv wrapper.
 * I chose the djbdns interface. 
 */
static int
getdnsip(stralloc *ip, stralloc *domain)
{
#ifdef SURBLTEST
	return dns_ip4(ip, domain);
#else
	char            x[IPFMT];
	ipalloc         tip = { 0 };
	int             len;

	if (stralloc_copys(ip, "") == 0)
		return -1;
	switch (dns_ip(&tip, domain))
	{
	case DNS_MEM:
		temp_nomem();
	case DNS_SOFT:
		temp_dns();
	case DNS_HARD:
		perm_dns();
	case 1:
		if (tip.len <= 0)
			temp_dns();
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
	}
#endif
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
	static stralloc ip = { 0, 0, 0 };
	static stralloc host = { 0, 0, 0 };
	int             hostlen;
	char          **curwl = whitelist;

	if (whitelist == NULL)
		return 0;

	if (stralloc_copys(&host, hostname) == 0)
		return -1;
	if (stralloc_append(&host, ".") == 0)
		return -1;
	hostlen = host.len;
	do {
		host.len = hostlen;
		if (stralloc_cats(&host, *curwl) == 0)
			return -1;
	/*
	 * printf ("Checking whitelist: '%.*s'\n", host.len, host.s);
	 */
		if (getdnsip(&ip, &host) == -1)
			return -1;

		if (ip.len >= 4)
			return 1;
		curwl++;
	} while (*curwl != NULL);

	return 0;

}

/*
 * Returns:
 * -1 on error
 * 0 is domain does not exist
 * 1 if domain exists.
 * 2 if domain exists, and a TXT record could be retrieved.
 */
char           *dns_text(char *);

static int
checksurbl(char *hostname, char *domain, char **text)
{
	static stralloc ip = { 0, 0, 0 };
	static stralloc host = { 0, 0, 0 };

	if (checkwhitelist(hostname) == 1)
		return 0;

	if (stralloc_copys(&host, hostname) == 0)
		return -1;
	if (stralloc_append(&host, ".") == 0)
		return -1;
	if (stralloc_cats(&host, domain) == 0)
		return -1;
/*
 * printf ("Checking blacklist: '%.*s'\n", host.len, host.s);
 */
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

static char    *
uri_decode(char *str, size_t str_len, char **strend)
{
	size_t          i = 0, j = 0;
	int             pasthostname = 0;
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
		if (strchr("\r\n\t \'\"<>()", str[i]) != NULL)
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
	if (strstr(cctlds, tld) != NULL)
		return 1;
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
/*
 * puts (uri);
 */
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
	static stralloc path = { 0, 0, 0 };

	size_t          uripos = urilen;
	int             partlen = uripos;
	char           *stext;
	size_t          stextlen;
	time_t          modtime;
	int             cachelock;

	if (fileexist(SURBL_CACHE_DIR) <= 0)
		return 0;

	if (stralloc_copys(&path, SURBL_CACHE_DIR) == 0)
		return -1;

	while (uripos--) {
		if (uri[uripos] == '.') {
			if (stralloc_append(&path, "/") == 0)
				return -1;
			if (stralloc_catb(&path, &uri[uripos + 1], partlen - uripos - 1) == 0)
				return -1;
			partlen = uripos;
		}
	}
	if (stralloc_append(&path, "/") == 0)
		return -1;
	if (stralloc_catb(&path, &uri[uripos + 1], partlen - uripos - 1) == 0)
		return -1;

	if (stralloc_append(&path, "/") == 0)
		return -1;
	if (stralloc_cats(&path, SURBL_CACHE_FILE) == 0)
		return -1;

	if (stralloc_0(&path) == 0)
		return -1;

	if (getmodtime(path.s, &modtime) == -1)
		return -1;

	if (modtime + cachelifetime < time(NULL)) {
	/*
	 * printf ("Cache entry expired: %s.\n", uri);
	 */
		unlink(path.s);
		return 0;
	}

/*
 * Slurp is not atomic, so lock the cache directory.
 * Since we read, we can share the lock.
 */
	if ((cachelock = file_lock(SURBL_CACHE_LOCK, 0, 0)) == -1)
		return -1;
	if (fileread(path.s, &stext, &stextlen)) {
		int             err = errno;
		file_unlock(cachelock);
		errno = err;
		return -1;
	}
	if (file_unlock(cachelock) == -1)
		return -1;

/*
 * printf ("cached: '%.*s' %d\n", stextlen, stext, stextlen);
 */

	if (stextlen > 0) {
		*text = malloc(stextlen + 1);
		if (*text == NULL) {
			return -1;
		}
		strcpy(*text, stext);
		return 2;
	} else {
		*text = NULL;
		return 1;
	}

	return 0;
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
	size_t          uripos = urilen;
	size_t          partlen = uripos;
	int             cachelock;

	static stralloc path = { 0, 0, 0 };

	if (fileexist(SURBL_CACHE_DIR) <= 0)
		return 0;

	if (stralloc_copys(&path, SURBL_CACHE_DIR) == 0)
		return -1;

	while (uripos--) {
		if (uri[uripos] == '.') {
			if (stralloc_append(&path, "/") == 0)
				return -1;
			if (stralloc_catb(&path, &uri[uripos + 1], partlen - uripos - 1) == 0)
				return -1;
		/*
		 * printf ("HOP %.*s\n", path.len, path.s);
		 */

			if (stralloc_0(&path) == 0)
				return -1;
		/*
		 * printf ("MAKING DIR '%s'\n", path.s);
		 */
			mkdir(path.s, 0700);
			path.len--;

			partlen = uripos;
		}
	}

	if (stralloc_append(&path, "/") == 0)
		return -1;
	if (stralloc_catb(&path, &uri[uripos + 1], partlen - uripos - 1) == 0)
		return -1;
/*
 * printf ("HOP %.*s\n", path.len, path.s);
 */

	if (stralloc_0(&path) == 0)
		return -1;
	mkdir(path.s, 0700);
	path.len--;

	if (stralloc_append(&path, "/") == 0)
		return -1;
	if (stralloc_cats(&path, SURBL_CACHE_FILE) == 0)
		return -1;

	if (stralloc_0(&path) == 0)
		return -1;

/*
 * printf ("FULL %s\n", path.s);
 */

/*
 * Creating the cache file is not atomic, so we lock the directory
 * exclusively first.
 */
	if ((cachelock = file_lock(SURBL_CACHE_LOCK, 1, 0)) == -1)
		return -1;
	if (filecreate(path.s, text) == -1) {
		int             err = errno;
		file_unlock(cachelock);
		errno = err;
		return -1;
	}
	if (file_unlock(cachelock) == -1)
		return -1;

	return 0;
}

static int
header_base64(char *header)
{
	char            cteword[] = "Content-Transfer-Encoding: ";
	size_t          ctewordlen = strlen(cteword);
	char           *cte = header;

	while ((cte = strchr(cte, '\n')) != NULL) {
		cte += strspn(cte, "\r\n \t");
		if (casencmp(cte, cteword, ctewordlen)) {
			break;
		}
	}

	if (cte == NULL)
		return 0;
	cte += ctewordlen;
	if (casencmp(cte, "base64", 6))
		return 1;
	return 0;
}

/*
 * Create a file with the given contents.
 */
static int
filecreate(char *file, char *text)
{
	FILE           *fp = fopen(file, "w");
	if (fp == NULL)
		return -1;
	if (text != NULL) {
		if (fputs(text, fp) == EOF) {
			int             err = errno;
			fclose(fp);
			unlink(file);
			errno = err;
			return -1;
		}
	}
	fclose(fp);
	return 0;
}

static int
fileread(char *file, char **buf, size_t * buflen)
{
	static stralloc bufss = { 0, 0, 0 };
	FILE           *fp = fopen(file, "r");
	if (fp == NULL)
		return -1;
	if (stralloc_copys(&bufss, "") == 0) {
		fclose(fp);
		return -1;
	}
	while (1) {
		char            tbuf[256];
		size_t          rv = fread(tbuf, 1, sizeof tbuf, fp);
		if (stralloc_catb(&bufss, tbuf, rv) == 0) {
			fclose(fp);
			return -1;
		}
		if (rv < sizeof tbuf)
			break;
	}
	if (ferror(fp)) {
		fclose(fp);
		return -1;
	}
	fclose(fp);
	if (stralloc_0(&bufss) == 0)
		return -1;
	*buf = bufss.s;
	*buflen = bufss.len - 1;
	return 0;
}

/*
 * Returns 1 if file exists, 0 if it doesn't, -1 if there was an error. 
 */
static int
fileexist(char *file)
{
	struct stat     st;
	int             rv = stat(file, &st);
	if (rv == 0)
		return 1;
	else if (rv == -1 && errno == ENOENT)
		return 0;
	else
		return -1;
}

static int
getmodtime(char *file, time_t * modtime)
{
	struct stat     st;
	if (stat(file, &st) == -1)
		return -1;
	*modtime = st.st_mtime;
	return 0;
}

/*
 * Returns lock descriptor on success, -1 on error, -2 if the file was already
 * locked by another process and block is 0.
 * If excl is 0, a shared lock will be issued. If excl is not 0, an exclusive
 * lock will be issued.
 */
static int
file_lock(char *lockfile, int excl, int block)
{
	int             fd;

	if (excl)
		excl = LOCK_EX;
	else
		excl = LOCK_SH;

	if (!block)
		excl |= LOCK_NB;

	fd = open(lockfile, O_RDWR);
	if (fd == -1) {
		if (errno == ENOENT) {
			fd = open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0600);
			if (fd == -1)
				return -1;
		} else
			return -1;
	}

	if (flock(fd, excl) == -1) {
		if (!block && errno == EWOULDBLOCK)
			return -2;
		else
			return -1;
	}

	return fd;
}

/*
 * Unlock crontab.
 * Descriptor is given through fd.
 */
static int
file_unlock(int fd)
{
/*
 * if (unlink (LOCKFILE) == -1)
 * return -1;
 */
	if (flock(fd, LOCK_UN) == -1)
		return -1;
	if (close(fd) == -1)
		return -1;
	return 0;
}

static unsigned long
revl(unsigned long r4)
{
	return (r4 << 24 & 0xFF000000)
		| (r4 << 8 & 0x00FF0000)
		| (r4 >> 8 & 0x0000FF00)
		| (r4 >> 24 & 0x000000FF);
}

/*
 * 'b64' is a base-64 encoded buffer. bt_str64dec will decode it, and place
 * the result in *text, which is allocated, and the length of the resulting
 * string is stored in *textlen.
 * Decoding will continue until a character not in the base64 scheme is
 * detected. *endptr will contain the place it stopped decoding.
 */
static int
bt_str64dec(char *b64, char **endptr, char **text, size_t * textlen, char *ignore)
{
	char           *b64enc = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char           *tmp;
	int             b64dec[256];
	int             count = 0;
	unsigned long   buf = 0;
	char           *bufp = (char *) &buf;
	struct stralloc textss = { 0, 0, 0 };
	size_t          pos = 0;

	memset(b64dec, 0, sizeof b64dec);

	tmp = b64enc;

	while (*ignore) {
		b64dec[*ignore & 0xFF] = -1;
		ignore++;
	}

	while (*tmp) {
		int            *foo = &b64dec[*tmp & 0xFF];
		if (*foo == -1)
			++ * foo;
		*foo = tmp - b64enc + 1;
		tmp++;
	}
	b64dec['=' & 0xFF] = 1;

	while (1) {
		int             inchar = b64[pos++];

	/*
	 * Ignore ignored characters. 
	 */
		if (b64dec[inchar & 0xFF] == -1)
			continue;
	/*
	 * Die on unknown characters. 
	 */
		if (b64dec[inchar & 0xFF] == 0)
			break;

		buf <<= 6;
		buf |= b64dec[inchar & 0xFF] - 1;
		count++;

	/*
	 * count%4 
	 */
		if ((count & 3) == 0) {
		/*
		 * Reverse word if endianness doesn't cooperate. 
		 */
			if (bufp[3] == 0)
				buf = revl(buf);

		/*
		 * Append to full text buffer. 
		 */
			if (stralloc_catb(&textss, bufp + 1, 3) == -1)
				return -1;

		/*
		 * Reset buffer. 
		 */
			buf = 0;
		}
	}

	*text = textss.s;
	*textlen = textss.len;
	*endptr = &b64[pos - 1];

	return 0;
}

static int
casencmp(char *str1, char *str2, unsigned long len)
{
	unsigned long   i = 0;
	while (i < len) {
		if (tolower(str1[i]) != tolower(str2[i]))
			return 0;
		if (str1[i] == '\0')
			break;
		i++;
	}
	return 1;
}
