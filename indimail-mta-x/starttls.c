/*
 * $Id: starttls.c,v 1.16 2023-01-15 12:42:12+05:30 Cprogrammer Exp mbhangui $
 */
#include "hastlsa.h"
#if defined(HASTLSA) && defined(TLS)
#include "haveip6.h"
#include <unistd.h>
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <alloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <tls.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <stralloc.h>
#include <getln.h>
#include <wait.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <env.h>
#include <case.h>
#include <str.h>
#include <fmt.h>
#include <now.h>
#include <noreturn.h>
#include "socket.h"
#include "timeoutconn.h"
#include "ip.h"
#include "dns.h"
#include "control.h"
#include "variables.h"
#include "tlsacheck.h"
#include "ipalloc.h"
#include "tlsarralloc.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"
#include "dossl.h"
#include "varargs.h"

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define SSL_ST_BEFORE 0x4000
#endif

#define HUGESMTPTEXT  5000

GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)

static int      smtpfd;
static const char *ssl_err_str = NULL;
static struct ip_mx partner;
static char    *partner_fqdn = NULL;
static SSL     *ssl;
static stralloc rhost = { 0 }; /*- host ip to which qmail-remote ultimately connects */
static stralloc smtptext = { 0 };
static saa      ehlokw = { 0 };	/*- list of EHLO keywords and parameters */
static int      maxehlokwlen = 0;
static stralloc hexstring = { 0 };
static stralloc tline = { 0 };
ipalloc         ia = { 0 };
tlsarralloc     ta = { 0 };
stralloc        save = { 0 };
static union v46addr outip;
static stralloc sauninit = { 0 };

extern int      verbose;
extern int      timeoutconn;
extern int      timeoutdata;
static int      smtps;
extern stralloc helohost;

no_return void
ssl_exit(int status)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0)
			usleep(100);
		SSL_free(ssl);
		ssl = NULL;
	}
	_exit(status);
}

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die1sys(111, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die1sys(111, "fatal: write: ");
	return;
}

no_return void
die(int e)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0);
		SSL_free(ssl);
		ssl = NULL;
	}
	ssl_exit(e);
}

void
logerr(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1) {
		strerr_warn1("fatal: write: ", &strerr_sys);
		die(111);
	}
	return;
}

void
logerrf(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1 || substdio_flush(subfderr) == -1) {
		strerr_warn1("fatal: write: ", &strerr_sys);
		die(111);
	}
}

void
die_nomem_child()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "fatal: out of memory\n");
	substdio_flush(subfderr);
	die (2);
}

void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "fatal: out of memory\n");
	substdio_flush(subfderr);
	die (111);
}

void
die_control(char *arg1, char *arg2)
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "fatal: ");
	substdio_puts(subfderr, arg1);
	substdio_puts(subfderr, ": ");
	substdio_puts(subfderr, arg2);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	die (111);
}

void
die_write()
{
	substdio_flush(subfdout);
}

void
outhost()
{
	char            x[IPFMT];
	unsigned int    len;

	switch (partner.af)
	{
#ifdef IPV6
	case AF_INET6:
		len = ip6_fmt(x, &partner.addr.ip6);
		break;
#endif
	case AF_INET:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	default:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	}
	if (!stralloc_copyb(&rhost, x, len) ||
			!stralloc_0(&rhost))
		die_nomem();
	if (substdio_put(subfderr, x, len) == -1) {
		strerr_warn1("fatal: write: ", &strerr_sys);
		die(111);
	}
}

void
dropped()
{
	logerr("Connected to ");
	outhost();
	logerr(" but connection died");
	if (ssl_err_str) {
		logerr(": ");
		logerr((char *) ssl_err_str);
	}
	logerrf("\n");
	die (111);
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	if (ssl) {
		if ((r = ssl_timeoutread(timeoutdata, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = myssl_error_str();
	} else
		r = timeoutread(timeoutdata, smtpfd, buf, len);
	if (r <= 0)
		dropped();
	return r;
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

	if (ssl) {
		if ((r = ssl_timeoutwrite(timeoutdata, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = myssl_error_str();
	} else
		r = timeoutwrite(timeoutdata, smtpfd, buf, len);
	if (r <= 0)
		dropped();
	return r;
}

char            inbuf[1500];
char            smtptobuf[1500];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        smtpto = SUBSTDIO_FDBUF(safewrite, -1, smtptobuf, sizeof smtptobuf);
char            smtpfrombuf[128];
substdio        smtpfrom = SUBSTDIO_FDBUF(saferead, -1, smtpfrombuf, sizeof smtpfrombuf);

void
outsmtptext()
{
	int             i;

	if (smtptext.s && smtptext.len) {
		out("Remote host said: ");
		for (i = 0; i < smtptext.len; ++i) {
			if (!smtptext.s[i])
				smtptext.s[i] = '?';
		}
		if (substdio_put(subfderr, smtptext.s, smtptext.len) == -1) {
			strerr_warn1("fatal: write: ", &strerr_sys);
			die (111);
		}
		smtptext.len = 0;
	}
}

/*
 * die =   0 - success
 * die =  -1 - failure system error
 * die =   1 - failure non-system error
 *
 * code =  xxx - smtp code
 * code =    0 - temporary failure
 */
no_return void
#ifdef  HAVE_STDARG_H
quit(int code, int e, char *prepend, ...)
#else
#include <varargs.h>
quit(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *str;
#ifndef HAVE_STDARG_H
	int             code, e;
	char           *prepend;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, prepend);
#else
	va_start(ap);
	code = va_arg(ap, int);
	e = va_arg(ap, int);
	prepend = va_arg(ap, char *);
#endif
	substdio_putflush(&smtpto, "QUIT\r\n", 6);
	if (verbose == 2)
		substdio_putsflush(subfdout, "Client: QUIT\n");
	logerr(prepend);
	outhost();

	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		logerr(str);
	}
	logerr(".\n");
	va_end(ap);
	outsmtptext();
	die(e == -1 ? 111 : e);
}

no_return void
tls_quit(const char *s1, char *s2, char *s3, char *s4, char *s5, stralloc *peer_sa)
{
	char            ch;
	int             i, state;

#if 0
	logerr((char *) s1);
#endif
	logerr((char *) s2);
	if (s3)
		logerr((char *) s3);
	if (s4)
		logerr((char *) s4);
	if (s5)
		logerr((char *) s5);
	if (peer_sa && peer_sa->len) {
		for (i = 0; i < peer_sa->len; ++i) {
			ch = peer_sa->s[i];
			if (ch < 33)
				ch = '?';
			if (ch > 126)
				ch = '?';
			if (substdio_put(subfdout, &ch, 1) == -1)
				die (0);
		}
	}

	/*-
	 * shouldn't talk to the client unless in an appropriate state
	 * https://mta.openssl.org/pipermail/openssl-commits/2015-October/002060.html
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	state = ssl ? SSL_get_state(ssl) : SSL_ST_BEFORE;
	if ((state & TLS_ST_OK) || (!smtps && (state & SSL_ST_BEFORE))) {
		substdio_putsflush(&smtpto, "QUIT\r\n");
		if (verbose == 2)
			substdio_putsflush(subfdout, "Client: QUIT\n");
	}
#else
	state = ssl ? ssl->state : SSL_ST_BEFORE;
	if ((state & SSL_ST_OK) || (!smtps && (state & SSL_ST_BEFORE))) {
		substdio_putsflush(&smtpto, "QUIT\r\n");
		if (verbose == 2)
			substdio_putsflush(subfdout, "Client: QUIT\n");
	}
#endif
	logerr(ssl ? "; connected to " : "; connecting to ");
	outhost();
	logerrf("\n");
	if (verbose == 2 && ssl) {
		X509           *peercert;

		logerr("STARTTLS proto=");
		logerr((char *) SSL_get_version(ssl));
		logerr("; cipher=");
		logerr((char *) SSL_get_cipher(ssl));

		/*- we want certificate details */
		if ((peercert = SSL_get_peer_certificate(ssl))) {
			char           *str;

			str = X509_NAME_oneline(X509_get_subject_name(peercert), NULL, 0);
			logerr("; subject=");
			logerr(str);
			OPENSSL_free(str);

			str = X509_NAME_oneline(X509_get_issuer_name(peercert), NULL, 0);
			logerr("; issuer=");
			logerr(str);
			OPENSSL_free(str);

			X509_free(peercert);
		}
		logerrf(";\n");
	}
	die (111);
}

void
get(char *ch)
{
	substdio_get(&smtpfrom, ch, 1);
	if (verbose == 2)
		substdio_put(subfdout, ch, 1);
	if (*ch != '\r') {
		if (smtptext.len < HUGESMTPTEXT) {
			if (!stralloc_append(&smtptext, ch))
				die_nomem();
		}
	}
}

unsigned long
smtpcode()
{
	unsigned char   ch;
	unsigned long   code;

	if (!stralloc_copys(&smtptext, ""))
		die_nomem();
	get((char *) &ch);
	code = ch - '0';
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	for (;;) {
		get((char *) &ch);
		if (ch != '-')
			break;
		while (ch != '\n')
			get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
	}
	while (ch != '\n')
		get((char *) &ch);
	return code;
}

int
timeoutconn46(int fd, struct ip_mx *ix, union v46addr *ip, int port, int timeout)
{
	switch (ix->af)
	{
#ifdef IPV6
	case AF_INET6:
		return timeoutconn6(fd, &ix->addr.ip6, ip, port, timeout);
		break;
#endif
	case AF_INET:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
		break;
	default:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
	}
}

void
perm_dns(char *str)
{
	logerr("Sorry, I couldn't find any host named ");
	logerr(str ? str : partner_fqdn);
	logerrf("\n");
	die (5);
}

void
temp_dns()
{
	logerr("no host by that name ");
	logerr(partner_fqdn);
	logerrf("\n");
	die (6);
}

void
temp_oserr()
{
	logerrf("System resources temporarily unavailable\n");
	die (111);
}

stralloc        sa = { 0 };
int
get_tlsa_rr(char *domain, int mxhost, int port)
{
	int             j;
	unsigned long   r;
	char            strnum[FMT_ULONG];

	if (mxhost) {
		if (!stralloc_copyb(&sa, "_", 1) ||
				!stralloc_catb(&sa, strnum, fmt_uint(strnum, port)) ||
				!stralloc_catb(&sa, "._tcp.", 6) ||
				!stralloc_cats(&sa, domain))
			die_nomem();
		dns_init(0);
		r = dns_tlsarr(&ta, &sa);
		switch (r)
		{
		case DNS_HARD:
			if (!stralloc_0(&sa))
				die_nomem();
			perm_dns(sa.s);
		case DNS_SOFT:
			temp_dns();
		case DNS_MEM:
			die_nomem();
		}
	} else {
		if (!stralloc_copys(&sa, domain))
			die_nomem();
		dns_init(0);
		r = now() + getpid();
		r = dns_mxip(&ia, &sa, r);
		switch (r)
		{
		case DNS_HARD:
			perm_dns(0);
		case DNS_SOFT:
			temp_dns();
		case DNS_MEM:
			die_nomem();
		}
		for (j = 0; j < ia.len; ++j) {
			if (j && !str_diff(ia.ix[j].fqdn, ia.ix[j - 1].fqdn))
				continue;
			if (!stralloc_copyb(&sa, "_", 1) ||
					!stralloc_catb(&sa, strnum, fmt_uint(strnum, port)) ||
					!stralloc_catb(&sa, "._tcp.", 6) ||
					!stralloc_cats(&sa, ia.ix[j].fqdn))
				die_nomem();
			r = dns_tlsarr(&ta, &sa);
			switch (r)
			{
			case DNS_HARD:
				if (!stralloc_0(&sa))
					die_nomem();
				perm_dns(sa.s);
			case DNS_SOFT:
				temp_dns();
			case DNS_MEM:
				die_nomem();
			}
		} /*- for (j = 0; j < ia.len; ++j) */
	}
	return (0);
}

unsigned long
ehlo()
{
	stralloc       *sa_ptr;
	char           *s, *e, *p;
	unsigned long   code;

	if (ehlokw.len > maxehlokwlen)
		maxehlokwlen = ehlokw.len;
	ehlokw.len = 0;
	if (substdio_puts(&smtpto, "EHLO ") == -1)
		strerr_die1sys(111, "fatal: write: ");
	else
	if (substdio_put(&smtpto, helohost.s, helohost.len) == -1)
		strerr_die1sys(111, "fatal: write: ");
	else
	if (substdio_puts(&smtpto, "\r\n") == -1)
		strerr_die1sys(111, "fatal: write: ");
	else
	if (substdio_flush(&smtpto) == -1)
		strerr_die1sys(111, "fatal: write: ");
	if (verbose == 2) {
		out("Client: EHLO ");
		substdio_put(subfdout, helohost.s, helohost.len);
		substdio_putsflush(subfdout, "\n");
	}
	if ((code = smtpcode()) != 250)
		return code;
	s = smtptext.s;
	while (*s++ != '\n');		/*- skip the first line: contains the domain */
	e = smtptext.s + smtptext.len - 6;	/*- 250-?\n */
	while (s <= e) {
		int             wasspace = 0;

		if (!saa_readyplus(&ehlokw, 1))
			die_nomem();
		sa_ptr = ehlokw.sa + ehlokw.len++;
		if (ehlokw.len > maxehlokwlen)
			*sa_ptr = sauninit;
		else
			sa_ptr->len = 0;

		/*- smtptext is known to end in a '\n' */
		for (p = (s += 4);; ++p) {
			if (*p == '\n' || *p == ' ' || *p == '\t') {
				if (!wasspace)
					if (!stralloc_catb(sa_ptr, s, p - s) ||
							!stralloc_0(sa_ptr))
						die_nomem();
				if (*p == '\n')
					break;
				wasspace = 1;
			} else
			if (wasspace == 1) {
				wasspace = 0;
				s = p;
			}
		}
		s = ++p;

		/*
		 * keyword should consist of alpha-num and '-'
		 * broken AUTH might use '=' instead of space
		 */
		for (p = sa_ptr->s; *p; ++p) {
			if (*p == '=') {
				*p = '\0';
				break;
			}
		}
	}
	return 250;
}

int
do_dane_validation(char *host, int port)
{
	static ipalloc  ip = { 0 };
    char           *err_str = NULL, *servercert = NULL;
	char            strnum[FMT_ULONG], hex[2];
	int             i, j, tlsa_status, authfullMatch, authsha256, authsha512,
					match0Or512, needtlsauth, usage;
	unsigned long   code;
	unsigned char   ch;
    tlsarr         *rp;

	if (!stralloc_copys(&sa, host))
		die_nomem_child();
	partner_fqdn = host;
	/*- get the ip of the MX host */
	switch (dns_ip(&ip, &sa))
	{
	case DNS_MEM:
		die_nomem_child();
	case DNS_SOFT:
		temp_dns();
	case DNS_HARD:
		perm_dns(0);
	case 1:
		if (ip.len <= 0)
			temp_dns();
	}
	get_tlsa_rr(host, 1, port);
	if (!ta.len) {
		logerrf("dane_query_tlsa: No DANE data were found\n");
		ssl_exit(0);
	}
	/*- print TLSA records */
	for (j = 0, usage = -1; j < ta.len; ++j) {
		rp = &(ta.rr[j]);
		out("TLSARR[");
		strnum[fmt_ulong(strnum, (unsigned long) j)] = '\0';
		out(strnum);
		out("]:");
		out(rp->host);
		out(" IN TLSA ( ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->usage)] = '\0';
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->selector)] = '\0';
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->mtype)] = '\0';
		out(strnum);
		out(" ");
		for (i = hexstring.len = 0; i < rp->data_len; i++) {
			fmt_hexbyte(hex, (rp->data + i)[0]);
			substdio_put(subfdout, hex, 2);
		}
		out(" )\n");
		flush();
	}
#ifdef IPV6
	for (i = 0; i < 16; i++)
		outip.ip6.d[i] = 0;
#else
	for (i = 0; i < 4; i++)
		outip.ip.d[i] = 0;
#endif
	for (i = j = 0; i < ip.len; ++i) {
#ifdef IPV6
		if ((smtpfd = (ip.ix[i].af == AF_INET ? socket_tcp4() : socket_tcp6())) == -1)
#else
		if ((smtpfd = socket_tcp4()) == -1)
#endif
			temp_oserr();
		if (timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconn)) {
			close(smtpfd);
			continue;
		}
		partner = ip.ix[i];
		partner_fqdn = ip.ix[i].fqdn;
		break;
	}
	code = smtpcode();
	if (verbose)
		flush();
	if (code >= 500 && code < 600)
		quit(code, 1, "Connected to ", " but greeting failed", 0);
	else
	if (code >= 400 && code < 500)
		quit(code, 1, "Connected to ", " but greeting failed", 0);
	else
	if (code != 220)
		quit(code, 1, "Connected to ", " but greeting failed", 0);
	if (!smtps)
		code = ehlo();
	match0Or512 = authfullMatch = authsha256 = authsha512 = 0;
	if (!do_tls(&ssl, 0, smtps, smtpfd, &needtlsauth, &servercert, partner_fqdn, NULL, 0, tls_quit,
				die_nomem, die_control, die_write, quit, 0, &ehlokw, verbose)) {/*- tls is needed for DANE */
		logerr("Connected to ");
		outhost();
		logerrf(" but unable to intiate TLS for DANE\n");
		die (111);
	}
	if (verbose)
		flush();
	for (j = 0, usage = -1; j < ta.len; ++j) {
		rp = &(ta.rr[j]);
		if (!rp->mtype || rp->mtype == 2)
			match0Or512 = 1;
		for (i = hexstring.len = 0; i < rp->data_len; i++) {
			fmt_hexbyte(hex, (rp->data + i)[0]);
			if (!stralloc_catb(&hexstring, hex, 2))
				die_nomem_child();
		}
		if (!stralloc_0(&hexstring))
			die_nomem_child();
		if (!rp->usage || rp->usage == 2)
			usage = 2;
		if (!(tlsa_status = tlsa_vrfy_records(ssl, hexstring.s, rp->usage,
					rp->selector, rp->mtype, partner_fqdn, tls_quit,
					die_nomem, 0, out, flush, &err_str, 0))) {
			switch(rp->mtype)
			{
			case 0:
				authfullMatch = 1;
				break;
			case 1:
				authsha256 = 1;
				break;
			case 2:
				authsha512 = 1;
				break;
			}
		}
   } /*- for (j = 0, usage = -1; j < ta.len; ++j) */

	/*-
	 * client SHOULD accept a server public key that
	 * matches either the "3 1 0" record or the "3 1 2" record, but it
	 * SHOULD NOT accept keys that match only the weaker "3 1 1" record.
	 * 9.  Digest Algorithm Agility
	 * https://tools.ietf.org/html/rfc7671
	 */
	if ((!match0Or512 && authsha256) || match0Or512) {
		if (needtlsauth && usage == 2)
			do_pkix(ssl, servercert, partner_fqdn, tls_quit, die_nomem, 0);
	} else { /*- dane validation failed */
		substdio_putsflush(&smtpto, "QUIT\r\n");
		if (verbose == 2)
			substdio_putsflush(subfdout, "Client: QUIT\n");
		logerr("Connected to ");
		outhost();
		logerr(" but recipient failed DANE validation.\n");
		substdio_flush(subfderr);
		die (1);
	}
	substdio_putsflush(&smtpto, "QUIT\r\n");
	if (verbose == 2)
		substdio_putsflush(subfdout, "Client: QUIT\n");
	get((char *) &ch);
	while (ch != '\n')
		get((char *) &ch);
	close(smtpfd); /*- close the SMTP connection */
	if (verbose == 2)
		flush();
	die (0);
	/*- not reached */
	return (0);
}

/*
 * get TLSA RR records and saves them in stralloc variable save
 */
int
get_dane_records(char *host)
{
	char            strnum[FMT_ULONG];
	char           *ptr;
	int             dane_pid, wstat, dane_exitcode, match, len;
	int             pipefd[2];

	if (pipe(pipefd) == -1) {
		strerr_warn1("fatal: unable to create pipe: ", &strerr_sys);
		die(111);
	}
	substdio_fdbuf(&ssin, read, pipefd[0], inbuf, sizeof(inbuf));
	switch ((dane_pid = fork()))
	{
	case -1:
		strerr_warn1("fatal: unable to fork: ", &strerr_sys);
		die(111);
	case 0:
		substdio_discard(subfdout);
		substdio_discard(subfderr);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			die (3);
		if (dup2(pipefd[1], 2) == -1)
			die (3);
		if (pipefd[1] != 1 && pipefd[1] != 2)
			close(pipefd[1]);
		do_dane_validation(host, 25);
		die (4);
	default:
		close(pipefd[1]);
		break;
	}
	for (save.len = 0;;) {
		if (getln(&ssin, &tline, &match, '\n') == -1)
			break;
		if (!match && !tline.len)
			break;
		tline.len--; /*- remove newline */
		if (verbose == 2) {
			if (substdio_put(subfdout, tline.s, tline.len) == -1)
				strerr_die1sys(111, "fatal: write: ");
			if (substdio_put(subfdout, "\n", 1) == -1)
				strerr_die1sys(111, "fatal: write: ");
		}
		if (!str_diffn(tline.s, "TLSA[", 5)) {
			for (len = 0, ptr = tline.s + 5; *ptr; len++, ptr++) {
				if (*ptr == ':') {
					/*- record1\0record2\0...recordn\0 */
					if (!stralloc_catb(&save, ptr + 1, tline.len - (len + 6)) ||
							!stralloc_0(&save))
						die_nomem();
					break;
				}
			}
		} else
		if (!str_diffn(tline.s, "dane_query_tlsa: No DANE data were found", 40)) {
			if (!stralloc_cat(&save, &tline) ||
					!stralloc_0(&save)) /*- serves as record field seperator */
				die_nomem();
		}
	}
	close(pipefd[0]);
	if (substdio_flush(subfdout) == -1) {
		strerr_warn1("fatal: write: ", &strerr_sys);
		die(111);
	}
	if (wait_pid(&wstat, dane_pid) != dane_pid) {
		strerr_warn1("fatal: wait_pid: ", &strerr_sys);
		die (111);
	}
	if (wait_crashed(wstat)) {
		logerrf("fatal: child crashed\n");
		die (111);
	}
	switch (dane_exitcode = wait_exitcode(wstat))
	{
	case 0: /*- either verified or no TLSA records */
		return (RECORD_OK);
	case 1:
		return (RECORD_NOVRFY);
	case 2:
		if (verbose == 2)
			logerrf("child out of memory\n");
		break;
	case 3:
		if (verbose == 2)
			logerrf("child unable to dup pipefd\n");
		break;
	case 4:
		if (verbose == 2)
			logerrf("child exec failed\n");
		break;
	case 5: /*- perm dns - NO TLSA RR Records */
		if (!stralloc_copyb(&save, "_25._tcp.", 9) ||
				!stralloc_cats(&save, host) ||
				!stralloc_catb(&save, ": No TLSA RR", 12) ||
				!stralloc_0(&save))
			die_nomem();
		return (RECORD_OK);
	case 6:
		break;
	case 100: /*- child returning 100 means domain doesn't exist */
		break;
	default:
		strnum[fmt_ulong(strnum, (unsigned long) dane_exitcode)] = '\0';
		logerr("warning");
		logerr(": error with child: exit code [");
		logerr(strnum);
		logerrf("]\n");
		break;
	}
	return (RECORD_FAIL);
}
#else
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
char            smtptobuf[1500];
substdio        smtpto = SUBSTDIO_FDBUF(write, 2, smtptobuf, sizeof smtptobuf);
unsigned long smtpcode() { return(550);}
#endif /*- #if defined(HASTLSA) && defined(TLS) */

void
getversion_starttls_c()
{
	static char    *x = "$Id: starttls.c,v 1.16 2023-01-15 12:42:12+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: starttls.c,v $
 * Revision 1.16  2023-01-15 12:42:12+05:30  Cprogrammer
 * quit() function changed to have varargs
 *
 * Revision 1.15  2023-01-06 17:40:27+05:30  Cprogrammer
 * moved tls/ssl functions to dossl.c
 *
 * Revision 1.14  2023-01-04 09:24:27+05:30  Cprogrammer
 * fixed incorrect setting of smtptext
 *
 * Revision 1.13  2023-01-03 19:50:47+05:30  Cprogrammer
 * replace set_tls_method() from libqmail
 * made global variables static
 *
 * Revision 1.12  2022-05-18 13:30:29+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.11  2021-06-14 01:19:59+05:30  Cprogrammer
 * collapsed stralloc_..
 *
 * Revision 1.10  2021-05-26 10:47:20+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.9  2020-11-24 13:48:34+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2020-11-22 23:12:12+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.7  2020-05-11 10:59:44+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.6  2020-05-10 17:47:13+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.5  2018-06-01 22:53:36+05:30  Cprogrammer
 * display correct host in perm_dns()
 *
 * Revision 1.4  2018-05-31 17:12:18+05:30  Cprogrammer
 * fixed potential use of uninitialized variable in do_pkix()
 * changed DANE validation messages
 *
 * Revision 1.3  2018-05-31 02:21:50+05:30  Cprogrammer
 * print status of DANE Validation on stdout
 *
 * Revision 1.2  2018-05-30 20:11:23+05:30  Cprogrammer
 * using hexstring variable inside tlsa_vrfy_records() clobbered certDataField
 *
 * Revision 1.1  2018-05-30 11:54:47+05:30  Cprogrammer
 * Initial revision
 *
 */
