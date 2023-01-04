/*
 * $Id: starttls.c,v 1.14 2023-01-04 09:24:27+05:30 Cprogrammer Exp mbhangui $
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

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define SSL_ST_BEFORE 0x4000
#endif

#define HUGESMTPTEXT  5000
GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)

static int      smtpfd;
static const char *ssl_err_str = 0;
static struct ip_mx partner;
static char    *partner_fqdn = 0;
static stralloc rhost = { 0 }; /*- host ip to which qmail-remote ultimately connects */
static SSL_CTX *ctx;
static stralloc saciphers = { 0 }, tlsFilename = { 0 }, clientcert = { 0 };
static stralloc smtptext = { 0 };
static saa      ehlokw = { 0 };	/*- list of EHLO keywords and parameters */
static int      maxehlokwlen = 0;
static stralloc hexstring = { 0 };
static stralloc hextmp = { 0 };
static stralloc certdir_s;
stralloc        sa = { 0 };
ipalloc         ia = { 0 };
tlsarralloc     ta = { 0 };
static stralloc tline = { 0 };
stralloc        save = { 0 };
static union v46addr outip;
static stralloc sauninit = { 0 };

extern int      verbose;
extern int      timeoutconn;
extern int      timeoutdata;
static SSL     *ssl;
static int      smtps;
extern stralloc helohost;

no_return void
ssl_exit(int status)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0)
			usleep(100);
		SSL_free(ssl);
		ssl = 0;
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

void
die(int e)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0);
		SSL_free(ssl);
		ssl = 0;
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
void
quit(char *prepend, char *append, int e)
{
	substdio_putsflush(&smtpto, "QUIT\r\n");
	if (verbose == 2)
		substdio_putsflush(subfdout, "Client: QUIT\n");
	/*- waiting for remote side is just too ridiculous */
	logerr(prepend);
	outhost();
	logerr(append);
	logerr(".\n");
	outsmtptext();
	substdio_flush(subfderr);
	die (e);
}

void
tls_quit(const char *s1, char *s2, char *s3, char *s4, stralloc *peer_sa)
{
	char            ch;
	int             i, state;

	logerr((char *) s1);
	if (s2)
		logerr((char *) s2);
	if (s3)
		logerr((char *) s3);
	if (s4)
		logerr((char *) s4);
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

/*-
 * don't want to fail handshake if certificate can't be verified
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX *ctx_dummy)
{
	return 1;
}

int
match_partner(char *s, int len)
{
	if (!case_diffb(partner_fqdn, len, (char *) s) && !partner_fqdn[len])
		return 1;
	/*- we also match if the name is *.domainname */
	if (*s == '*') {
		const char     *domain = partner_fqdn + str_chr(partner_fqdn, '.');
		if (!case_diffb((char *) domain, --len, (char *) ++s) && !domain[len])
			return 1;
	}
	return 0;
}

void
do_pkix(char *servercert)
{
	X509           *peercert;
	STACK_OF(GENERAL_NAME) *gens;
	int             r, i = 0;
	char           *tmp;

	/*- PKIX */
	if ((r = SSL_get_verify_result(ssl)) != X509_V_OK) {
		tmp = (char *) X509_verify_cert_error_string(r);
		tls_quit("TLS unable to verify server with ", servercert, ": ", tmp, 0);
	}
	if (!(peercert = SSL_get_peer_certificate(ssl)))
		tls_quit("TLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);

	/*-
	 * RFC 2595 section 2.4: find a matching name
	 * first find a match among alternative names
	 */
	if ((gens = X509_get_ext_d2i(peercert, NID_subject_alt_name, 0, 0))) {
		for (i = 0, r = sk_GENERAL_NAME_num(gens); i < r; ++i) {
			const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, i);
			if (gn->type == GEN_DNS)
				if (match_partner((char *) gn->d.ia5->data, gn->d.ia5->length))
					break;
		}
		sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
	}

	/*- no alternative name matched, look up commonName */
	if (!gens || i >= r) {
		stralloc        peer = { 0 };
		X509_NAME      *subj = X509_get_subject_name(peercert);
		if ((i = X509_NAME_get_index_by_NID(subj, NID_commonName, -1)) >= 0) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			X509_NAME_ENTRY *t;
			t = X509_NAME_get_entry(subj, i);
			ASN1_STRING    *s = X509_NAME_ENTRY_get_data(t);
#else
			const ASN1_STRING *s = X509_NAME_get_entry(subj, i)->value;
#endif
			if (s) {
				peer.len = s->length;
				peer.s = (char *) s->data;
			}
		}
		if (peer.len <= 0)
			tls_quit("TLS unable to verify server ", partner_fqdn, ": certificate contains no valid commonName", 0, 0);
		if (!match_partner((char *) peer.s, peer.len))
			tls_quit("TLS unable to verify server ", partner_fqdn, ": received certificate for ", 0, &peer);
	}
	X509_free(peercert);
	return;
}

/*
 * 1. returns 0 --> fallback to non-tls
 *    if certs do not exist
 *    host is in notlshosts
 *    smtps == 0 and tls session cannot be initiated
 * 2. returns 1 if tls session was initated
 * 3. exits on error, if smtps == 1 and tls session did not succeed
 */
int
do_tls(int pkix, int *needtlsauth, char **scert)
{
	int             code, i = 0, _needtlsauth = 0;
	static char     ssl_initialized;
	const char     *ciphers = 0;
	char           *t, *servercert = 0, *certfile;
	static SSL     *myssl;
	stralloc        ssl_option = { 0 };
	int             method_fail;

	if (needtlsauth)
		*needtlsauth = 0;
	if (scert)
		*scert = 0;
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}
	if (myssl) {
		SSL_free(myssl);
		ssl = myssl = (SSL *) 0;
	}
	if (!certdir && !(certdir = env_get("CERTDIR"))) {
		if (!stralloc_copys(&certdir_s, auto_sysconfdir) ||
				!stralloc_catb(&certdir_s, "/certs", 6) ||
				!stralloc_0(&certdir_s))
			die_nomem();
		certdir = certdir_s.s;
	}
	
	if (!stralloc_copys(&clientcert, certdir) ||
			!stralloc_append(&clientcert, "/"))
		die_nomem();
	certfile = ((certfile = env_get("CLIENTCERT")) ? certfile : "clientcert.pem");
	if (!stralloc_cats(&clientcert, certfile) ||
			!stralloc_0(&clientcert))
		die_nomem();
	if (access(clientcert.s, F_OK)) {
		if (errno != error_noent)
			die_control("unable to read client certificate ", clientcert.s);
		return (0);
	}
	if (partner_fqdn) {
		struct stat     st;
		if (!stralloc_copys(&tlsFilename, certdir) ||
				!stralloc_catb(&tlsFilename, "/tlshosts/", 10) ||
				!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn)) ||
				!stralloc_catb(&tlsFilename, ".pem", 4) ||
				!stralloc_0(&tlsFilename))
			die_nomem();
		if (stat(tlsFilename.s, &st)) {
			_needtlsauth = 0;
			if (needtlsauth)
				*needtlsauth = 0;
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/notlshosts/", 12) ||
					!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn) + 1) ||
					!stralloc_0(&tlsFilename))
				die_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/tlshosts/exhaustivelist", 24) ||
					!stralloc_0(&tlsFilename))
				die_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
		} else {
			*scert = servercert = tlsFilename.s;
			_needtlsauth = 1;
			if (needtlsauth)
				*needtlsauth = 1;
		}
	}

	if (!smtps) {
		stralloc       *saptr = ehlokw.sa;
		unsigned int    len = ehlokw.len;

		/*- look for STARTTLS among EHLO keywords */
		for (; len && case_diffs(saptr->s, "STARTTLS"); ++saptr, --len);
		if (!len) {
			if (!_needtlsauth)
				return (0);
			tls_quit("No TLS achieved while", tlsFilename.s, " exists", 0, 0);
		}
	}

	if (!ssl_initialized++)
		SSL_library_init();
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&tlsFilename, controldir) ||
			!stralloc_catb(&tlsFilename, "/tlsclientmethod", 16) ||
			!stralloc_0(&tlsFilename))
		die_nomem();
	if (control_rldef(&ssl_option, tlsFilename.s, 0, "TLSv1_2") != 1)
		die_control("unable to read control file ", tlsFilename.s);
	if (!stralloc_0(&ssl_option))
		die_nomem();
	if (!(ctx = set_tls_method(ssl_option.s, client, &method_fail))) {
		if (!smtps && !_needtlsauth)
			return (0);
		t = (char *) myssl_error();
		switch (method_fail)
		{
		case 1:
#if OPENSSL_VERSION_NUMBER < 0x10100000L
			tls_quit("TLS Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2", t, 0, 0, 0);
#else
			tls_quit("TLS Supported methods: TLSv1, TLSv1_1, TLSv1_2 TLSv1_3", t, 0, 0, 0);
#endif
			break;
		case 2:
			tls_quit("TLS error initializing SSLv23 ctx: ", t, 0, 0, 0);
			break;
		case 3:
			tls_quit("TLS error initializing SSLv3 ctx: ", t, 0, 0, 0);
			break;
		case 4:
			tls_quit("TLS error initializing TLSv1 ctx: ", t, 0, 0, 0);
			break;
		case 5:
			tls_quit("TLS error initializing TLSv1_1 ctx: ", t, 0, 0, 0);
			break;
		case 6:
			tls_quit("TLS error initializing TLSv1_2 ctx: ", t, 0, 0, 0);
			break;
		case 7:
			tls_quit("TLS error initializing TLSv1_3 ctx: ", t, 0, 0, 0);
			break;
		}
	}

	if (_needtlsauth) {
		if (!SSL_CTX_load_verify_locations(ctx, servercert, NULL)) {
			t = myssl_error();
			SSL_CTX_free(ctx);
			ctx = (SSL_CTX *) 0;
			tls_quit("TLS unable to load ", servercert, ": ", t, 0);
		}
		/*- set the callback here; SSL_set_verify didn't work before 0.9.6c */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
	}

	/*- let the other side complain if it needs a cert and we don't have one */
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert.s))
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		SSL_CTX_use_PrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
#else
		SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
#endif

	if (!(myssl = SSL_new(ctx))) {
		t = myssl_error();
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			ctx = (SSL_CTX *) 0;
			return (0);
		}
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
		tls_quit("TLS error initializing ssl: ", t, 0, 0, 0);
	} else {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}

	if (!smtps) {
		substdio_putsflush(&smtpto, "STARTTLS\r\n");
		if (verbose == 2)
			substdio_putsflush(subfdout, "Client: STARTTLS\n");
	}

	/*- while the server is preparing a response, do something else */
	if (!ciphers) {
		if (control_readfile(&saciphers, "tlsclientciphers", 0) == -1) {
			while (SSL_shutdown(myssl) == 0)
				usleep(100);
			SSL_free(myssl);
			ssl = myssl = NULL;
			die_control("unable to read control file ", "tlsclientciphers");
		}
		if (saciphers.len) {
			/*- convert all '\0's except the last one to ':' */
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		}
	}
	SSL_set_cipher_list(myssl, ciphers);

	/*- SSL_set_options(myssl, SSL_OP_NO_TLSv1); */
	SSL_set_fd(myssl, smtpfd);

	/*- read the response to STARTTLS */
	if (!smtps) {
		if (smtpcode() != 220) {
			while (SSL_shutdown(myssl) == 0)
				usleep(100);
			SSL_free(myssl);
			ssl = myssl = (SSL *) 0;
			if (!_needtlsauth)
				return (0);
			tls_quit("STARTTLS rejected while ", tlsFilename.s, " exists", 0, 0);
		}
	}
	ssl = myssl;
	if (ssl_timeoutconn(timeoutconn, smtpfd, smtpfd, ssl) <= 0) {
		t = (char *) myssl_error_str();
		tls_quit("TLS connect failed: ", t, 0, 0, 0);
	}
	if (smtps && (code = smtpcode()) != 220) /*- 220 ready for tls */
		quit("TLS Connected to ", " but greeting failed", code);
	if (pkix && _needtlsauth)
		do_pkix(servercert);
	return (1);
}

/*-
 * USAGE
 * 0, 1, 2, 3, 255
 * 255 PrivCert
 * 
 * SELECTOR
 * --------
 * 0, 1, 255
 * 255 PrivSel
 * 
 * Matching Type
 * -------------
 * 0, 1, 2, 255
 * 255 PrivMatch
 *
 * Return Value
 * 0   - match target certificate & payload data
 * 1,2 - successful match
 */
stralloc        certData = { 0 };
int
tlsa_vrfy_records(char *certDataField, int usage, int selector, int match_type, char **err_str )
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX     *mdctx;
#else
	EVP_MD_CTX     *mdctx;
	EVP_MD_CTX      _mdctx;
#endif
	const EVP_MD   *md = 0;
	BIO            *membio = 0;
	EVP_PKEY       *pkey = 0;
	X509           *xs = 0;
	STACK_OF(X509) *sk;
	static char     errbuf[256];
	char            buffer[512], hex[2];
	unsigned char   md_value[EVP_MAX_MD_SIZE];
	unsigned char  *ptr;
	int             i, len, e;
	unsigned int    md_len;

	if (!ssl)
		return (-1);
	switch (usage)
	{
	/*- Trust anchor */
	case 0: /*- PKIX-TA(0) maps to DANE-TA(2) */
		/*- flow through */
	case 2: /*- DANE-TA(2) */
		usage = 2;
		break;
	case 1: /*- PKIX-EE(1) maps to DANE-EE(3) */
		/*- flow through */
	case 3: /*- DANE-EE(3) */
		usage = 3;
		break;
	default:
		return (-2);
	}
	switch (selector) /*- match full certificate or subjectPublicKeyInfo */
	{
	case 0: /*- Cert(0) - match full certificate   data/SHA256fingerprint/SHA512fingerprint */
		break;
	case 1: /*- SPKI(1) - match subject public key data/SHA256fingerprint/SHA512fingerprint  */
		break;
	default:
		return (-2);
	}
	switch (match_type) /*- sha256, sha512 */
	{
	case 0: /*- Full(0) - match full cert data or subjectPublicKeyInfo data */
		break;
	case 1: /*- SHA256(1) fingerprint - servers should publish this mandatorily */
		md = EVP_get_digestbyname("sha256");
		break;
	case 2: /*- SHA512(2)  fingerprint - servers should not exclusively publish this */
		md = EVP_get_digestbyname("sha512");
		break;
	default:
		return (-2);
	}
	/*- SSL_ctrl(ssl, SSL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, servername); -*/
	if (!(sk =  SSL_get_peer_cert_chain(ssl)))
		tls_quit("TLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);
	/*- 
	 * the server certificate is generally presented
	 * as the first certificate in the stack along with
	 * the remaining chain.
	 * last certificate in the list is a trust anchor
	 * 5.2.2.  Trust Anchor Digests and Server Certificate Chain
	 * https://tools.ietf.org/html/rfc7671
	 *
	 * for Usage 2, check the last  certificate - sk_X509_value(sk, sk_509_num(sk) - 1)
	 * for Usage 3, check the first certificate - sk_X509_value(sk, 0)
	 */
	/*- for (i = (usage == 2 ? 1 : 0); i < (usage == 2 ? sk_X509_num(sk) : 1); i++) -*/
	i = (usage == 2 ? sk_X509_num(sk) - 1 : 0);
	xs = sk_X509_value(sk, i);
	/*- 
	 * DANE Validation 
	 * case 1 - match full certificate data -                            - X 0 0
	 * case 2 - match full subjectPublicKeyInfo data                     - X 1 0
	 * case 3 - match  SHA512/SHA256 fingerprint of full certificate     - X 0 1, X 0 2
	 * case 4 - match  SHA512/SHA256 fingerprint of subjectPublicKeyInfo - X 1 1, X 1 2
	 */
	if (match_type == 0 && selector == 0) { /*- match full certificate data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_X509(membio, xs)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create write to membio: ", *err_str, 0, 0, 0);
		}
		for (certData.len = 0;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN CERTIFICATE-----"))
				continue;
			if (str_start(buffer, "-----END CERTIFICATE-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				die_nomem();
		}
		if (!stralloc_0(&certData))
			die_nomem();
		certData.len--;
		BIO_free_all(membio);
		e = str_diffn(certData.s, certDataField, certData.len);
		if (verbose) {
			out(e == 0 ? "matched " : "failed  ");
			out(usage == 2 ? "full anchorCert\n" : "full serverCert\n");
			out(certDataField);
			out("\n");
			flush();
		}
		return (e);
	}
	if (match_type == 0 && selector == 1) { /*- match full subjectPublicKeyInfo data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!(pkey = X509_get_pubkey(xs))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to get pubkey: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_PUBKEY(membio, pkey)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to write pubkey to membio: ", *err_str, 0, 0, 0);
		}
		for (;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN PUBLIC KEY-----"))
				continue;
			if (str_start(buffer, "-----END PUBLIC KEY-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				die_nomem();
		}
		if (!stralloc_0(&certData))
			die_nomem();
		certData.len--;
		BIO_free_all(membio);
		e = str_diffn(certData.s, certDataField, certData.len);
		if (verbose) {
			out(e == 0 ? "matched " : "failed  ");
			out(usage == 2 ? "SubjectPublicKeyInfo anchorCert\n" : "SubjectPublicKeyInfo serverCert\n");
			out(certDataField);
			out("\n");
			flush();
		}
		return (e);
	}
	/*- SHA512/SHA256 fingerprint of full certificate */
	if ((match_type == 2 || match_type == 1) && selector == 0) {
		if (!X509_digest(xs, md, md_value, &md_len))
			tls_quit("TLS Unable to get peer cerficatte digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				die_nomem();
		}
		e = str_diffn(certDataField, hextmp.s, hextmp.len);
		if (verbose) {
			out(e == 0 ? "matched " : "failed  ");
			out(match_type == 1 ? "sha256" : "sha512");
			out(" fingerprint [");
			out(certDataField);
			out(usage == 2 ? "] full anchorCert\n" : "] full serverCert\n");
		}
		return (e);
	}
	/*- SHA512/SHA256 fingerprint of subjectPublicKeyInfo */
	if ((match_type == 2 || match_type == 1) && selector == 1) {
		unsigned char  *tmpbuf = (unsigned char *) 0;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		if (!(mdctx = EVP_MD_CTX_new()))
			die_nomem();
#else
		mdctx = &_mdctx;
#endif
		if (!(pkey = X509_get_pubkey(xs)))
			tls_quit("TLS Unable to get public key", 0, 0, 0, 0);
		if (!EVP_DigestInit_ex(mdctx, md, NULL))
			tls_quit("TLS Unable to initialize EVP Digest", 0, 0, 0, 0);
		if ((len = i2d_PUBKEY(pkey, NULL)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!(tmpbuf = (unsigned char *) OPENSSL_malloc(len * sizeof(char))))
			die_nomem();
		ptr = tmpbuf;
		if ((len = i2d_PUBKEY(pkey, &ptr)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!EVP_DigestUpdate(mdctx, tmpbuf, len))
			tls_quit("TLS Unable to update EVP Digest", 0, 0, 0, 0);
		OPENSSL_free(tmpbuf);
		if (!EVP_DigestFinal_ex(mdctx, md_value, &md_len))
			tls_quit("TLS Unable to compute EVP Digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				die_nomem();
		}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		e = str_diffn(certDataField, hextmp.s, hextmp.len);
		if (verbose) {
			out(e == 0 ? "matched " : "failed  ");
			out(match_type == 1 ? "sha256" : "sha512");
			out(" fingerprint [");
			out(certDataField);
			out(usage == 2 ? "] SubjectPublicKeyInfo anchorCert\n" : "] SubjectPublicKeyInfo serverCert\n");
		}
		return (e);
	}
	return (1);
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
				*p = 0;
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
    char           *err_str = 0, *servercert = 0;
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
		strnum[fmt_ulong(strnum, (unsigned long) j)] = 0;
		out(strnum);
		out("]:");
		out(rp->host);
		out(" IN TLSA ( ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->usage)] = 0;
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->selector)] = 0;
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->mtype)] = 0;
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
		quit("Connected to ", " but greeting failed", code);
	else
	if (code >= 400 && code < 500)
		quit("Connected to ", " but greeting failed", code);
	else
	if (code != 220)
		quit("Connected to ", " but greeting failed", code);
	if (!smtps)
		code = ehlo();
	match0Or512 = authfullMatch = authsha256 = authsha512 = 0;
	if (!do_tls(0, &needtlsauth, &servercert)) {/*- tls is needed for DANE */
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
		if (!(tlsa_status = tlsa_vrfy_records(hexstring.s, rp->usage, rp->selector, rp->mtype, &err_str))) {
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
			do_pkix(servercert);
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
		strnum[fmt_ulong(strnum, (unsigned long) dane_exitcode)] = 0;
		logerr("warning");
		logerr(": error with child: exit code [");
		logerr(strnum);
		logerrf("]\n");
		break;
	}
	return (RECORD_FAIL);
}
#else
#warning "TLSA, TLS code not compiled"
#endif /*- #if defined(HASTLSA) && defined(TLS) */

void
getversion_starttls_c()
{
	static char    *x = "$Id: starttls.c,v 1.14 2023-01-04 09:24:27+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: starttls.c,v $
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
