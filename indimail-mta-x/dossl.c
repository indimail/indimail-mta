/*
 * $Id: dossl.c,v 1.7 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $
 */
#include "hastlsa.h"
#if defined(TLS) || defined(TLSA)
#include <unistd.h>
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/stat.h>
#include <str.h>
#include <error.h>
#include <subfd.h>
#include <case.h>
#include <env.h>
#include <stralloc.h>
#include <tls.h>
#include <alloc.h>
#include <fmt.h>
#include "variables.h"
#include "varargs.h"
#include "control.h"
#include "dossl.h"
#include "auto_sysconfdir.h"
#include "auto_control.h"

#ifdef TLS
extern unsigned long smtpcode();

static SSL_CTX *ctx;
static stralloc saciphers, tlsFilename, clientcert, certdir_s;

extern int      timeoutconn;
extern int      timeoutdata;
extern substdio smtpto;

/*-
 * don't want to fail handshake if certificate can't be verified
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX *ctx_dummy)
{
	return 1;
}

int
match_partner(char *s, int len, const char *fqdn)
{
	if (!case_diffb(fqdn, len, (char *) s) && !fqdn[len])
		return 1;
	/*- we also match if the name is *.domainname */
	if (*s == '*') {
		const char     *domain = fqdn + str_chr(fqdn, '.');
		if (!case_diffb((char *) domain, --len, (char *) ++s) && !domain[len])
			return 1;
	}
	return 0;
}

void
do_pkix(SSL *ssl, const char *servercert, const char *fqdn,
		void(*tlsquit)(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, stralloc *s),
		void(*mem_err)(),
		stralloc *stext)
{
	X509           *peercert;
	STACK_OF(GENERAL_NAME) *gens;
	int             r, i = 0;
	char           *t;

	/*- PKIX */
	if ((r = SSL_get_verify_result(ssl)) != X509_V_OK) {
		t = (char *) X509_verify_cert_error_string(r);
		if (stext) {
			if (!stralloc_copyb(stext, "TLS unable to verify server with ", 33) ||
					!stralloc_cats(stext, servercert) ||
					!stralloc_catb(stext, ": ", 2) ||
					!stralloc_cats(stext, t))
				mem_err();
		}
		tlsquit("Z", "TLS unable to verify server with ", servercert, ": ", t, 0);
	}
	if (!(peercert = SSL_get_peer_certificate(ssl))) {
		if (stext) {
			if (!stralloc_copyb(stext, "TLS unable to verify server ", 28) ||
					!stralloc_cats(stext, fqdn) ||
					!stralloc_catb(stext, ": no certificate provided", 25))
				mem_err();
		}
		tlsquit("Z", "TLS unable to verify server ", fqdn, ": no certificate provided", 0, 0);
	}

	/*-
	 * RFC 2595 section 2.4: find a matching name
	 * first find a match among alternative names
	 */
	if ((gens = X509_get_ext_d2i(peercert, NID_subject_alt_name, 0, 0))) {
		for (i = 0, r = sk_GENERAL_NAME_num(gens); i < r; ++i) {
			const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, i);
			if (gn->type == GEN_DNS)
				if (match_partner((char *) gn->d.ia5->data, gn->d.ia5->length, fqdn))
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
			X509_NAME_ENTRY *xnet;
			xnet = X509_NAME_get_entry(subj, i);
			ASN1_STRING    *s = X509_NAME_ENTRY_get_data(xnet);
#else
			const ASN1_STRING *s = X509_NAME_get_entry(subj, i)->value;
#endif
			if (s) {
				peer.len = s->length;
				peer.s = (char *) s->data;
			}
		}
		if (peer.len <= 0) {
			if (stext) {
				if (!stralloc_copyb(stext, "TLS unable to verify server ", 28) ||
						!stralloc_cats(stext, fqdn) ||
						!stralloc_catb(stext, ": certificate contains no valid commonName", 42))
					mem_err();
			}
			tlsquit("Z", "TLS unable to verify server ", fqdn, ": certificate contains no valid commonName", 0, 0);
		}
		if (!match_partner((char *) peer.s, peer.len, fqdn)) {
			if (stext) {
				if (!stralloc_copyb(stext, "TLS unable to verify server ", 28) ||
						!stralloc_cats(stext, fqdn) ||
						!stralloc_catb(stext, ": received certificate for ", 27) ||
						!stralloc_cat(stext, &peer) ||
						!stralloc_0(stext))
					mem_err();
			}
			tlsquit("Z", "TLS unable to verify server ", fqdn, ": received certificate for ", 0, &peer);
		}
	}
	X509_free(peercert);
	return;
}

/*
 * 1. returns 0 or 2 --> fallback to non-tls
 *    if certs do not exist
 *    host is in notlshosts
 *    smtps == 0 and tls session cannot be initiated
 * 2. returns 1 if tls session was initiated
 * 3. exits on error, if smtps == 1 and tls session did not succeed
 */
int
do_tls(SSL **ssl, int pkix, int smtps, int smtpfd, int *needtlsauth,
		char **scert, const char *fqdn, const char *_host, int hostlen,
		void (*tlsquit) (const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, stralloc *s),
		void (*mem_err) (void),
		void (*ctrl_err) (const char *, const char *),
		void (*write_err) (void),
#ifdef HAVE_STDARG_H
		void(*quit)(int code, int e, const char *p, ...),
#else
		void(*quit)(),
#endif
		stralloc *stext,
		saa *ehlokw,
		int verbose)
{
	int             code, i = 0, _needtlsauth = 0, method;
	static char     ssl_initialized;
	const char     *ciphers = NULL;
	const char     *t, *servercert = NULL, *certfile;
	static SSL     *myssl;
	stralloc        ssl_option = { 0 };
	int             method_fail;

	if (needtlsauth)
		*needtlsauth = 0;
	if (scert)
		*scert = NULL;
	/*-
	 * do_tls() gets called in do_smtp()
	 * if do_smtp() returns for trying next mx
	 * we need to re-initialize
	 */
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}
	if (myssl) {
		SSL_free(myssl);
		myssl = NULL;
	}
	if (!certdir && !(certdir = env_get("CERTDIR"))) {
		if (!stralloc_copys(&certdir_s, auto_sysconfdir) ||
				!stralloc_catb(&certdir_s, "/certs", 6) ||
				!stralloc_0(&certdir_s))
			mem_err();
		certdir = certdir_s.s;
	}
	if (!stralloc_copys(&clientcert, certdir) ||
			!stralloc_append(&clientcert, "/"))
		mem_err();
	certfile = ((certfile = env_get("CLIENTCERT")) ? certfile : "clientcert.pem");
	if (!stralloc_cats(&clientcert, certfile) ||
			!stralloc_0(&clientcert))
		mem_err();
	if (access(clientcert.s, F_OK)) {
		if (errno != error_noent)
			ctrl_err("Unable to read client certificate", clientcert.s);
		return (0);
	}
	if (fqdn) {
		struct stat     st;
		if (!stralloc_copys(&tlsFilename, certdir) ||
				!stralloc_catb(&tlsFilename, "/tlshosts/", 10) ||
				!stralloc_catb(&tlsFilename, fqdn, str_len(fqdn)) ||
				!stralloc_catb(&tlsFilename, ".pem", 4) ||
				!stralloc_0(&tlsFilename))
			mem_err();
		if (stat(tlsFilename.s, &st)) {
			_needtlsauth = 0;
			if (needtlsauth)
				*needtlsauth = 0;
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/notlshosts/", 12) ||
					!stralloc_catb(&tlsFilename, fqdn, str_len(fqdn) + 1) ||
					!stralloc_0(&tlsFilename)) /*- fqdn */
				mem_err();
			if (!stat(tlsFilename.s, &st))
				return (0);
			if (hostlen) {
				if (!stralloc_copys(&tlsFilename, certdir) ||
						!stralloc_catb(&tlsFilename, "/notlshosts/", 12) ||
						!stralloc_catb(&tlsFilename, _host, hostlen) ||
						!stralloc_0(&tlsFilename)) /*- domain */
					mem_err();
				if (!stat(tlsFilename.s, &st))
					return (0);
			}
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/tlshosts/exhaustivelist", 24) ||
					!stralloc_0(&tlsFilename))
				mem_err();
			if (!stat(tlsFilename.s, &st))
				return (0);
		} else { /*- servercert for fqdn, domain, host exists */
			if (scert)
				*scert = tlsFilename.s;
			servercert = tlsFilename.s;
			_needtlsauth = 1;
			if (needtlsauth)
				*needtlsauth = 1;
		}
	}

	if (!smtps) {
		stralloc       *saptr = ehlokw->sa;
		unsigned int    len = ehlokw->len;

		/*- look for STARTTLS among EHLO keywords */
		for (; len && case_diffs(saptr->s, "STARTTLS"); ++saptr, --len);
		if (!len) {
			if (!_needtlsauth) /*- file certdir/notlshosts/fqdn.pem doesn't exist */
				return (0);
			if (stext) {
				if (!stralloc_copyb(stext, "No TLS achieved while ", 22) ||
						!stralloc_cats(stext, servercert) ||
						!stralloc_catb(stext, " exists", 7))
					mem_err();
			}
			tlsquit("Z", "No TLS achieved while", tlsFilename.s, " exists", 0, 0);
		}
	}

	if (!ssl_initialized++)
		SSL_library_init();
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
#if OPENSSL_VERSION_NUMBER >= 0x1010107f /* openssl 1.1.1 */
	if (control_readline(&ssl_option, "tlsclientmethod") == -1)
#else
	if (control_readline(&ssl_option, "tlsclientmethod") == -1)
#endif
		ctrl_err("Unable to read control files", "tlsclientmethod");
	if (ssl_option.len && !stralloc_0(&ssl_option))
		mem_err();
	if (!(ctx = set_tls_method(ssl_option.len ? ssl_option.s : 0, &method, qremote, &method_fail))) {
		if (!smtps && !_needtlsauth)
			return (0);
		t = myssl_error();
		if (stext) {
			if (!stralloc_copyb(stext, "TLS error initializing ctx: ", 28) ||
					!stralloc_cats(stext, t))
				mem_err();
		}
		switch (method_fail)
		{
		case 1:
#if OPENSSL_VERSION_NUMBER >= 0x1010107f
			tlsquit("Z", "TLS Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2, TLSv1_3", t, 0, 0, 0);
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
			tlsquit("Z", "TLS Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2", t, 0, 0, 0);
#else
			tlsquit("Z", "TLS Supported methods: SSLv23, SSLv3", t, 0, 0, 0);
#endif
			break;
		case 2:
			tlsquit("Z", "TLS error initializing SSLv23 ctx: ", t, 0, 0, 0);
			break;
		case 3:
			tlsquit("Z", "TLS error initializing SSLv3 ctx: ", t, 0, 0, 0);
			break;
		case 4:
			tlsquit("Z", "TLS error initializing TLSv1 ctx: ", t, 0, 0, 0);
			break;
		case 5:
			tlsquit("Z", "TLS error initializing TLSv1_1 ctx: ", t, 0, 0, 0);
			break;
		case 6:
			tlsquit("Z", "TLS error initializing TLSv1_2 ctx: ", t, 0, 0, 0);
			break;
#if OPENSSL_VERSION_NUMBER >= 0x1010107f
		case 7:
			tlsquit("Z", "TLS error initializing TLSv1_3 ctx: ", t, 0, 0, 0);
			break;
#endif
		}
	}
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

	if (_needtlsauth) {
		if (!SSL_CTX_load_verify_locations(ctx, servercert, NULL)) {
			t = myssl_error();
			if (stext) {
				if (!stralloc_copyb(stext, "TLS unable to load ", 19) ||
						!stralloc_cats(stext, servercert) ||
						!stralloc_catb(stext, ": ", 2) ||
						!stralloc_cats(stext, t))
					mem_err();
			}
			SSL_CTX_free(ctx);
			ctx = (SSL_CTX *) 0;
			tlsquit("Z", "TLS unable to load ", servercert, ": ", t, 0);
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
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			ctx = (SSL_CTX *) 0;
			return (0);
		}
		t = myssl_error();
		if (stext) {
			if (!stralloc_copyb(stext, "TLS error initializing ssl: ", 28) ||
					!stralloc_cats(stext, t))
				mem_err();
		}
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
		tlsquit("Z", "TLS error initializing ssl: ", t, 0, 0, 0);
	} else {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}

	if (!smtps) {
		if (substdio_putflush(&smtpto, "STARTTLS\r\n", 10) == -1) {
			SSL_free(myssl);
			myssl = NULL;
			write_err();
		}
		if (verbose == 2)
			substdio_putsflush(subfdout, "Client: STARTTLS\n");
	}

	/*- while the server is preparing a response, do something else */
	if (!(ciphers = env_get(method < 7 ? "TLS_CIPHER_LIST" : "TLS_CIPHER_SUITE"))) {
		if (control_readfile(&saciphers, method < 7 ? "clientcipherlist" : "clientciphersuite", 0) == -1) {
			while (SSL_shutdown(myssl) == 0)
				usleep(100);
			SSL_free(myssl);
			myssl = NULL;
			ctrl_err("Unable to read control file", method < 7 ? "clientcipherlist" : "clientciphersuite");
		}
		if (saciphers.len) {
			/*- convert all '\0's except the last one to ':' */
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		}
	}
	if (ciphers) {
#if OPENSSL_VERSION_NUMBER >= 0x1010107f
		i = (method < 7 ? SSL_set_cipher_list : SSL_set_ciphersuites) (myssl, ciphers);
#else
		i = SSL_set_cipher_list(myssl, ciphers);
#endif
		if (!i) {
			t = myssl_error();
			if (stext) {
				if (!stralloc_copyb(stext, "TLS error seting cipher", 23) ||
						!stralloc_cats(stext, method < 7 ? "list: " : "suite: ") ||
						!stralloc_cats(stext, t))
					mem_err();
			}
			SSL_free(myssl);
			myssl = NULL;
			tlsquit("Z", "TLS error setting cipher", method < 7 ? "list: " : "suite: ", t, 0, 0);
		}
	}

	/*- SSL_set_options(myssl, SSL_OP_NO_TLSv1); */
	SSL_set_fd(myssl, smtpfd);

	/*- read the response to STARTTLS */
	if (!smtps) {
		if (smtpcode() != 220) {
			while (SSL_shutdown(myssl) == 0)
				usleep(100);
			SSL_free(myssl);
			myssl = NULL;
			if (!_needtlsauth)
				return (0);
			if (stext) {
				if (!stralloc_copyb(stext, "STARTTLS rejected while ", 24) ||
						!stralloc_cats(stext, tlsFilename.s) ||
						!stralloc_catb(stext, " exists", 7))
					mem_err();
			}
			tlsquit("Z", "STARTTLS rejected while ", tlsFilename.s, " exists", 0, 0);
		}
	}
	*ssl = myssl;
	if (ssl_timeoutconn(timeoutconn, smtpfd, smtpfd, myssl) <= 0) {
		if (!smtps && !_needtlsauth) {
			i = errno;
			while (SSL_shutdown(myssl) == 0)
				usleep(100);
			SSL_free(myssl);
			*ssl = myssl = NULL;
			errno = i;
			return (2);
		}
		t = myssl_error_str();
		if (stext) {
			if (!stralloc_copyb(stext, "TLS connect failed: ", 20) ||
					!stralloc_cats(stext, t))
				mem_err();
		}
		tlsquit("Z", "TLS connect failed: ", t, 0, 0, 0);
	}
	if (smtps && (code = smtpcode()) != 220)
		quit(code, 1, "ZTLS Connected to ", " but greeting failed", 0);
	if (pkix && _needtlsauth) /*- 220 ready for tls */
		do_pkix(myssl, servercert, fqdn, tlsquit, mem_err, stext);
	return (1);
}
#endif /*- ifdef TLS */

#ifdef HASTLSA
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
stralloc        hextmp = { 0 };
int
tlsa_vrfy_records(SSL *ssl, char *certDataField, int usage, int selector,
		int match_type, const char *fqdn,
		void(*tlsquit)(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, stralloc *s),
		void(*mem_err)(void),
		stralloc *stext,
		void(*out)(const char *),
		void(*flush)(void),
		char **err_str, int verbose)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX     *mdctx;
#else
	EVP_MD_CTX     *mdctx;
	EVP_MD_CTX      _mdctx;
#endif
	const EVP_MD   *md = NULL;
	BIO            *membio = NULL;
	EVP_PKEY       *pkey = NULL;
	X509           *xs = NULL;
	STACK_OF(X509) *sk;
	static char     errbuf[256];
	char            buffer[1024], hex[2];
	unsigned char   md_value[EVP_MAX_MD_SIZE];
	unsigned char  *ptr;
	char           *cp;
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
	if (!(sk =  SSL_get_peer_cert_chain(ssl))) {
		if (stext) {
			if (!stralloc_copyb(stext, "TLS unable to verify server ", 28) ||
					!stralloc_cats(stext, fqdn) ||
					!stralloc_catb(stext, ": no certificate provided", 25))
				mem_err();
		}
		tlsquit("Z", "TLS unable to verify server ", fqdn, ": no certificate provided", 0, 0);
	}
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
	 * server cert - X = 2
	 * anchor cert - X = 3
	 * case 1 - match full certificate data -                            - X 0 0
	 * case 2 - match full subjectPublicKeyInfo data                     - X 1 0
	 * case 3 - match  SHA512/SHA256 fingerprint of full certificate     - X 0 1, X 0 2
	 * case 4 - match  SHA512/SHA256 fingerprint of subjectPublicKeyInfo - X 1 1, X 1 2
	 */
	if (match_type == 0 && selector == 0) { /*- match full certificate data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tlsquit("Z", "DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_X509(membio, xs)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tlsquit("Z", "DANE unable to create write to membio: ", *err_str, 0, 0, 0);
		}
		for (certData.len = 0;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN CERTIFICATE-----"))
				continue;
			if (str_start(buffer, "-----END CERTIFICATE-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = '\0';
			if (!stralloc_cats(&certData, buffer))
				mem_err();
		}
		if (!stralloc_0(&certData))
			mem_err();
		certData.len--;
		BIO_free_all(membio);
		e = str_diffn(certData.s, certDataField, certData.len);
			return (0);
		if (verbose && out && flush) {
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
			tlsquit("Z", "DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!(pkey = X509_get_pubkey(xs))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tlsquit("Z", "DANE unable to get pubkey: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_PUBKEY(membio, pkey)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tlsquit("Z", "DANE unable to write pubkey to membio: ", *err_str, 0, 0, 0);
		}
		for (;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN PUBLIC KEY-----"))
				continue;
			if (str_start(buffer, "-----END PUBLIC KEY-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = '\0';
			if (!stralloc_cats(&certData, buffer))
				mem_err();
		}
		if (!stralloc_0(&certData))
			mem_err();
		certData.len--;
		BIO_free_all(membio);
		if (!str_diffn(certData.s, certDataField, certData.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of full certificate */
	if ((match_type == 2 || match_type == 1) && selector == 0) {
		if (!X509_digest(xs, md, md_value, &md_len))
			tlsquit("Z", "TLS Unable to get peer cerficatte digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				mem_err();
		}
		cp = hextmp.s;
		if (!str_diffn(certDataField, (char *) cp, hextmp.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of subjectPublicKeyInfo */
	if ((match_type == 2 || match_type == 1) && selector == 1) {
		unsigned char  *tmpbuf = (unsigned char *) 0;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		if (!(mdctx = EVP_MD_CTX_new()))
			mem_err();
#else
		mdctx = &_mdctx;
#endif
		if (!(pkey = X509_get_pubkey(xs)))
			tlsquit("Z", "TLS Unable to get public key", 0, 0, 0, 0);
		if (!EVP_DigestInit_ex(mdctx, md, NULL))
			tlsquit("Z", "TLS Unable to initialize EVP Digest", 0, 0, 0, 0);
		if ((len = i2d_PUBKEY(pkey, NULL)) < 0)
			tlsquit("", "TLS failed to encode public key", 0, 0, 0, 0);
		if (!(tmpbuf = (unsigned char *) OPENSSL_malloc(len * sizeof(char))))
			mem_err();
		ptr = tmpbuf;
		if ((len = i2d_PUBKEY(pkey, &ptr)) < 0)
			tlsquit("Z", "TLS failed to encode public key", 0, 0, 0, 0);
		if (!EVP_DigestUpdate(mdctx, tmpbuf, len))
			tlsquit("Z", "TLS Unable to update EVP Digest", 0, 0, 0, 0);
		OPENSSL_free(tmpbuf);
		if (!EVP_DigestFinal_ex(mdctx, md_value, &md_len))
			tlsquit("Z", "TLS Unable to compute EVP Digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				mem_err();
		}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		cp = hextmp.s;
		if (!str_diffn(certDataField, (char *) cp, hextmp.len))
			return (0);
		return (1);
	}
	return (1);
}
#endif /*- #ifdef HASTLSA */
#endif /*- #ifdef TLS */

void
getversion_dossl_c()
{
	const char     *x = "$Id: dossl.c,v 1.7 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: dossl.c,v $
 * Revision 1.7  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-08-28 22:24:25+05:30  Cprogrammer
 * return 2 for tls connection failure
 *
 * Revision 1.4  2023-08-26 21:34:17+05:30  Cprogrammer
 * use SSL_set_cipher_list for tlsv1_2 and below, SSL_set_ciphersuite for tlsv1_3 and above
 * return 0 for connnection/negotiation failure in do_tls() for qmail-remote to retry connection in non-tls mode
 * No defaults for missing tlsservermethod, tlsclientmethod
 *
 * Revision 1.3  2023-07-07 10:36:31+05:30  Cprogrammer
 * fix potential SIGSEGV
 *
 * Revision 1.2  2023-01-15 12:25:54+05:30  Cprogrammer
 * prototype change for quit function with varargs
 *
 * Revision 1.1  2023-01-06 17:43:57+05:30  Cprogrammer
 * Initial revision
 *
 */
