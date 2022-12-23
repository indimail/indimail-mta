/*
 * $Id: tls.c,v 1.14 2022-12-23 10:36:25+05:30 Cprogrammer Exp mbhangui $
 *
 * ssl_timeoutio functions froms from Frederik Vermeulen's
 * tls patch for qmail
 * https://inoa.net/qmail-tls/netqmail-1.06-tls-20200107.patch
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#ifdef TLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <case.h>
#endif
#include <str.h>
#include <ndelay.h>
#include <taia.h>
#include <strerr.h>
#include <env.h>
#include <error.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include "iopause.h"
#include "tls.h"

#ifndef	lint
static char     sccsid[] = "$Id: tls.c,v 1.14 2022-12-23 10:36:25+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef TLS
static enum tlsmode usessl = none;
static char    *sslerr_str;
static SSL     *ssl_t;
static int      efd = -1;

void
set_essential_fd(int fd)
{
	efd = fd;
}

void
ssl_free()
{
	if (!ssl_t)
		return;
	SSL_shutdown(ssl_t);
	SSL_free(ssl_t);
	if (usessl != none)
		usessl = none;
	ssl_t = NULL;
}

/*
 * don't want to fail handshake if certificate can't be verified
 */
static int
verify_cb(int preverify_ok, X509_STORE_CTX * ctx)
{
	return 1;
}

const char     *
myssl_error()
{
	int             r = ERR_get_error();
	if (!r)
		return NULL;
	SSL_load_error_strings();
	return ERR_error_string(r, NULL);
}

const char     *
myssl_error_str()
{
	const char     *err = myssl_error();

	if (err)
		return err;
	if (!errno)
		return 0;
	return (errno == ETIMEDOUT) ? "timed out" : error_str(errno);
}

static int
check_cert(SSL *myssl, char *host)
{
	X509           *peer;
	char            peer_CN[256];

	if (SSL_get_verify_result(myssl) != X509_V_OK) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("SSL_get_verify_result: ", sslerr_str, 0);
		return (1);
	}
	/*
	 * Check the cert chain. The chain length
	 * is automatically checked by OpenSSL when
	 * we set the verify depth in the ctx
	 */

	/*- Check the common name */
	if (!(peer = SSL_get_peer_certificate(myssl))) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("SSL_get_peer_certificate: ", sslerr_str, 0);
		return (1);
	}
	X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, sizeof(peer_CN) - 1);
	if (host && case_diffs(peer_CN, host)) {
		strerr_warn2("hostname doesn't match Common Name ", peer_CN, 0);
		return (1);
	}
	return (0);
}

SSL_CTX        *
set_tls_method(char *ssl_option, enum tlsmode tmode)
{
	SSL_CTX        *ctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	int             method = 4;	/*- (1 unused) 2 = SSLv23, 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2 */
#else
	int             method = 0;	/*- (1,2 unused) 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2, 7=TLSv1.3 */
#endif

	if (ssl_option) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if (str_equal(ssl_option, "SSLv23"))
			method = 2;
		else
		if (str_equal(ssl_option, "SSLv3"))
			method = 3;
		else
#endif
		if (str_equal(ssl_option, "TLSv1"))
			method = 4;
		else
		if (str_equal(ssl_option, "TLSv1_1"))
			method = 5;
		else
		if (str_equal(ssl_option, "TLSv1_2"))
			method = 6;
		else
		if (str_equal(ssl_option, "TLSv1_3"))
			method = 7;
		else {
			strerr_warn1("tls_init: Invalid TLS method configured", 0);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
			strerr_warn1("tls_init: Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2", 0);
#else
			strerr_warn1("tls_init: Supported methods: TLSv1, TLSv1_1, TLSv1_2 TLSv1_3", 0);
#endif
			return ((SSL_CTX *) NULL);
		}
	}
	SSL_library_init();
	/*- a new SSL context with the bare minimum of options */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (method == 2 && !(ctx = SSL_CTX_new(tmode == client ? SSLv23_client_method() : SSLv23_server_method()))) {
		strerr_warn1("tls_init: TLS not available: unable to initialize SSLv23 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
	if (method == 3 && !(ctx = SSL_CTX_new(tmode == client ? SSLv3_client_method() : SSLv3_server_method()))) {
		strerr_warn1("tls_init: TLS not available: unable to initialize SSLv3 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#if defined(TLSV1_SERVER_METHOD) || defined(TLS1_VERSION)
	if (method == 4 && !(ctx = SSL_CTX_new(tmode == client ? TLSv1_client_method() : TLSv1_server_method()))) {
		strerr_warn1("tls_init: TLS not available: unable to initialize TLSv1 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 4) {
		strerr_warn1("tls_init: TLS not available: TLSv1 method not available", 0);
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_1_SERVER_METHOD) || defined(TLS1_1_VERSION)
	if (method == 5 && !(ctx = SSL_CTX_new(tmode == client ? TLSv1_1_client_method() : TLSv1_1_server_method()))) {
		strerr_warn1("tls_init: TLS not available: unable to initialize TLSv1_1 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 5) {
		strerr_warn1("tls_init: TLS not available: TLSv1_1_server_method not available", 0);
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_2_SERVER_METHOD) || defined(TLS1_2_VERSION)
	if (method == 6 && !(ctx = SSL_CTX_new(tmode == client ? TLSv1_2_client_method() : TLSv1_2_server_method()))) {
		strerr_warn1("tls_init: TLS not available: unable to initialize TLSv1_2 ctx", 0);
		return ((SSL_CTX *) NULL) ;
	} else
#else
	if (method == 6) {
		strerr_warn1("tls_init: TLS not available: TLSv1_2_server_method not available", 0);
		return ((SSL_CTX *) NULL) ;
	}
#endif
#else /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	switch (tmode)
	{
	case client:
		ctx = SSL_CTX_new(TLS_client_method());
		break;
	case server:
		ctx = SSL_CTX_new(TLS_server_method());
		break;
	default:
		ctx = SSL_CTX_new(TLS_method());
		break;
	}
	/*-
	 * Currently supported versions are SSL3_VERSION, TLS1_VERSION, TLS1_1_VERSION,
	 * TLS1_2_VERSION, TLS1_3_VERSION for TLS
	 */
	switch (method)
	{
	case 2:
		strerr_warn1("tls_init: TLS not available: SSLv23_server_method not available", 0);
		return ((SSL_CTX *) NULL) ;
	case 3:
		SSL_CTX_set_min_proto_version(ctx, SSL3_VERSION);
		SSL_CTX_set_max_proto_version(ctx, SSL3_VERSION);
		break;
	case 4:
		SSL_CTX_set_min_proto_version(ctx, TLS1_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_VERSION);
		break;
	case 5:
		SSL_CTX_set_min_proto_version(ctx, TLS1_1_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_1_VERSION);
		break;
	case 6:
		SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
		break;
	}
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
#endif /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	return ctx;
}

SSL_CTX        *
tls_init(char *tls_method, char *cert, char *cafile, char *ciphers, enum tlsmode tmode)
{
	static SSL_CTX *ctx = (SSL_CTX *) NULL;

	if (ctx)
		return (ctx);
	ctx = set_tls_method(tls_method, tmode);
	if (!ctx) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("SSL_CTX_new: error initializing methhod: ", sslerr_str, 0);
		return ((SSL_CTX *) NULL);
	}
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
	if (!SSL_CTX_load_verify_locations(ctx, cert, NULL)) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn4("unable to load certificate: ", cert, ": ", sslerr_str, 0);
		SSL_CTX_free(ctx);
		return ((SSL_CTX *) NULL);
	}
	/*
	 * set the callback here; SSL_set_verify didn't work before 0.9.6c
	 */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	/*- Set our cipher list */
	if (ciphers && !SSL_CTX_set_cipher_list(ctx, ciphers)) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn4("tls_init: unable to set ciphers: ", ciphers, ": ", sslerr_str, 0);
		SSL_CTX_free(ctx);
		return ((SSL_CTX *) NULL);
	}
#endif
	if (SSL_CTX_use_certificate_chain_file(ctx, cert)) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		if (SSL_CTX_use_PrivateKey_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
			sslerr_str = (char *) myssl_error_str();
			strerr_warn2("SSL_CTX_use_PrivateKey_file: Unable to load private keys: ", sslerr_str, 0);
			SSL_CTX_free(ctx);
			return ((SSL_CTX *) NULL);
		}
#else
		if (SSL_CTX_use_RSAPrivateKey_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
			sslerr_str = (char *) myssl_error_str();
			strerr_warn2("SSL_CTX_use_RSAPrivateKey_file: Unable to load RSA private keys: ", sslerr_str, 0);
			SSL_CTX_free(ctx);
			return ((SSL_CTX *) NULL);
		}
#endif
		if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
			sslerr_str = (char *) myssl_error_str();
			strerr_warn4("SSL_CTX_use_certificate_file: Unable to use cerficate: ", cert, ": ", sslerr_str, 0);
			SSL_CTX_free(ctx);
			return ((SSL_CTX *) NULL);
		}
		if (cafile && 1 != SSL_CTX_load_verify_locations(ctx, cafile, 0)) {
			sslerr_str = (char *) myssl_error_str();
			strerr_warn4("SSL_CTX_load_verify_locations: Unable to use ca certificate: ", cafile, ": ", sslerr_str, 0);
			SSL_CTX_free(ctx);
			return ((SSL_CTX *) NULL);
		}
	}
	return (ctx);
}

SSL            *
tls_session(SSL_CTX *ctx, int fd, char *ciphers)
{
	SSL            *myssl;
	BIO            *sbio;

	if (usessl != none)
		return ssl_t;
	if (!(myssl = SSL_new(ctx))) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("SSL_new: Unable to setup SSL session: ", sslerr_str, 0);
		SSL_CTX_free(ctx);
		return ((SSL *) NULL);
	}
#ifndef CRYPTO_POLICY_NON_COMPLIANCE
	if (ciphers && !SSL_set_cipher_list(myssl, ciphers)) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn4("tls_session: unable to set ciphers: ", ciphers, ": ", sslerr_str, 0);
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return ((SSL *) NULL);
	}
#endif
	if (!(sbio = BIO_new_socket(fd, BIO_NOCLOSE))) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("BIO_new_socket: ", sslerr_str, 0);
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return ((SSL *) NULL);
	}
	SSL_set_bio(myssl, sbio, sbio); /*- cannot fail */
	return (ssl_t = myssl);
}

int
tls_connect(SSL *myssl, char *host)
{
	if (SSL_connect(myssl) <= 0) {
		sslerr_str = (char *) myssl_error_str();
		strerr_warn2("SSL_connect: ", sslerr_str, 0);
		ssl_free();
		return -1;
	}
	if (host && check_cert(myssl, host)) {
		ssl_free();
		return -1;
	}
	usessl = client;
	return 0;
}

int
tls_accept(SSL *myssl)
{
	int             i;
	
	i = SSL_accept(myssl);
	SSL_get_error(myssl, i);
	if (i == 1) {
		usessl = server;
		return 0;
	}
	sslerr_str = (char *) myssl_error();
	strerr_warn2("SSL_accept: unable to accept SSL connection: ", sslerr_str, 0);
	SSL_shutdown(myssl);
	SSL_free(myssl);
	return i == 0 ? 1 : i;
}

ssize_t
ssl_timeoutio(int (*fun) (), long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	int             n = 0;
	const long      end = t + time(NULL);

	do
	{
		fd_set          fds;
		struct timeval  tv;

		const int       r = buf ? fun(myssl, buf, len) : fun(myssl);
		if (r > 0)
			return r;
		if ((t = end - time(NULL)) < 0) {
			errno = ETIMEDOUT;
			return -1;
		}
		tv.tv_sec = t;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		switch (SSL_get_error(myssl, r))
		{
		default:
			return r;	/*- some other error */
		case SSL_ERROR_WANT_READ:
			if (errno == EAGAIN && usessl == client && fun == SSL_read && efd != -1)
				FD_SET(efd, &fds);
			FD_SET(rfd, &fds);
			n = select(efd != -1 && efd > rfd ? efd + 1 : rfd + 1, &fds, NULL, NULL, &tv);
			/*- this avoids deadlock in tcpclient
			 * where data is initiated by tcpclient by reading fd 0
			 * but if tcpclient blocks on rfd, this will never happen
			 * and tcpcliet will continue to block on rfd.
			 * checking efd for input allows ssl_timeoutio() to
			 * return when data is available to be written to SSL.
			 */
			if (usessl == client && fun == SSL_read && efd != -1) {
				if (FD_ISSET(efd, &fds)) {
					errno = EAGAIN;
					return -1;
				}
			}
			break;
		case SSL_ERROR_WANT_WRITE:
			FD_SET(wfd, &fds);
			n = select(wfd + 1, NULL, &fds, NULL, &tv);
			break;
		}
		/*
		 * n is the number of descriptors that changed status
		 */
	} while (n > 0);
	if (!n)
		errno = ETIMEDOUT;
	return -1;
}

ssize_t
allwrite(int fd, char *buf, size_t len)
{
	ssize_t         w;
	size_t          total = 0;

	while (len) {
		if ((w = write(fd, buf, len)) == -1) {
			if (errno == error_intr)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0)
			;	/*- luser's fault */
		buf += w;
		total += w;
		len -= w;
	}
	return total;
}

ssize_t
allwritessl(SSL *myssl, char *buf, size_t len)
{
	int             w;
	size_t          total = 0;

	while (len) {
		if ((w = SSL_write(myssl, buf, len)) == -1) {
			if (errno == error_intr)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0)
			;	/*- luser's fault */
		buf += w;
		total += w;
		len -= w;
	}
	return total;
}

ssize_t
ssl_timeoutread(long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	if (!buf)
		return 0;
	if (SSL_pending(myssl))
		return (SSL_read(myssl, buf, len));
	return ssl_timeoutio(SSL_read, t, rfd, wfd, myssl, buf, len);
}

ssize_t
ssl_timeoutwrite(long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	if (!buf)
		return 0;
	return ssl_timeoutio((int (*)())allwritessl, t, rfd, wfd, myssl, buf, len);
}
#endif

int
translate(int sfd, int clearout, int clearin, unsigned int iotimeout)
{
	struct taia     now, deadline;
	iopause_fd      iop[2];
	int             flagexitasap, iopl;
	ssize_t         n, r;
	char            tbuf[2048];

	flagexitasap = 0;
	ndelay_on(sfd);
	while (!flagexitasap) {
		taia_now(&now);
		taia_uint(&deadline, iotimeout);
		taia_add(&deadline, &now, &deadline);

		/*- fill iopause struct */
		iopl = 2;
		iop[0].fd = sfd;
		iop[0].events = IOPAUSE_READ;
		iop[1].fd = clearin;
		iop[1].events = IOPAUSE_READ;

		/*- do iopause read */
		iopause(iop, iopl, &deadline, &now);
		if (iop[0].revents) { /*- data on sfd */
			if ((n = saferead(sfd, tbuf, sizeof(tbuf), iotimeout)) < 0) {
				if (errno == EAGAIN)
					continue;
				strerr_warn1("translate: read network", &strerr_sys);
				return -1;
			} else
			if (!n) {
				flagexitasap = 1;
				break;
			}
			if ((r = allwrite(clearout, tbuf, n)) == -1) {
				strerr_warn1("translate: write clear channel: ", &strerr_sys);
				return -1;
			}
		}
		if (iop[1].revents) { /*- data on clearin */
			if ((n = timeoutread(iotimeout, clearin, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn1("translate: read clear channel: ", &strerr_sys);
				return -1;
			} else
			if (!n) {
				flagexitasap = 1;
				break;
			}
			if ((r = safewrite(sfd, tbuf, n, iotimeout)) == -1) {
				if (errno == EAGAIN)
					continue;
				strerr_warn1("translate: write network: ", &strerr_sys);
				return -1;
			}
		}
		if (!iop[0].revents && !iop[1].revents) {
			if (!iop[0].revents)
				strerr_warn1("timeout reached without input from network", 0);
			if (!iop[1].revents)
				strerr_warn1("timeout reached without input from child", 0);
			return -1;
		}
	} /*- while (!flagexitasap) */
	return 0;
}

ssize_t
saferead(int fd, char *buf, size_t len, long timeout)
{
	ssize_t         r;

#ifdef TLS
	if (usessl != none) {
		if ((r = ssl_timeoutread(timeout, fd, fd, ssl_t, buf, len)) < 0) {
			if (errno == EAGAIN || errno == ETIMEDOUT)
				return -1;
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				strerr_warn2("saferead: ", sslerr_str, 0);
			else
				strerr_warn1("saferead: ", &strerr_sys);
		}
	} else
		r = timeoutread(timeout, fd, buf, len);
#else
	r = timeoutread(timeout, fd, buf, len);
#endif
	return r;
}

ssize_t
safewrite(int fd, char *buf, size_t len, long timeout)
{
	ssize_t         r;

#ifdef TLS
	if (usessl != none) {
		if ((r = ssl_timeoutwrite(timeout, fd, fd, ssl_t, buf, len)) < 0) {
			if (errno == EAGAIN || errno == ETIMEDOUT)
				return -1;
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				strerr_warn3("safewrite: ", sslerr_str, ": ", &strerr_sys);
			else
				strerr_warn1("safewrite: ", &strerr_sys);
		}
	} else
		r = timeoutwrite(timeout, fd, buf, len);
#else
	r = timeoutwrite(timeout, fd, buf, len);
#endif
	return r;
}

#ifndef	lint
void
getversion_tls_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
#endif

/*
 * $Log: tls.c,v $
 * Revision 1.14  2022-12-23 10:36:25+05:30  Cprogrammer
 * added set_tls_method to set TLS / SSL client/server method
 *
 * Revision 1.13  2022-12-22 22:19:36+05:30  Cprogrammer
 * log ssl error on SSL_accept() failure
 *
 * Revision 1.12  2022-07-01 18:54:12+05:30  Cprogrammer
 * set socket in no delay mode
 *
 * Revision 1.11  2022-05-18 00:48:43+05:30  Cprogrammer
 * replaced deprecated function SSL_CTX_use_RSAPrivateKey_file with SSL_CTX_use_PrivateKey_file
 *
 * Revision 1.10  2021-03-10 18:24:06+05:30  Cprogrammer
 * use set_essential_fd() to avoid deadlock
 *
 * Revision 1.9  2021-03-09 21:35:57+05:30  Cprogrammer
 * return ETIMEDOUT on timeout
 *
 * Revision 1.8  2021-03-09 09:38:08+05:30  Cprogrammer
 * make translate() function generic by using fd instead of SSL structure
 * fix ssl_timeoutio() for non-blocking io to fix SSL_reads() getting blocked
 *
 * Revision 1.7  2021-03-08 20:02:32+05:30  Cprogrammer
 * include taia.h explicitly
 *
 * Revision 1.6  2021-03-08 15:47:17+05:30  Cprogrammer
 * use TLS_client_method(), TLS_server_method() functions
 *
 * Revision 1.5  2021-03-06 23:14:33+05:30  Cprogrammer
 * added server functions
 * added translate() function for ssl io
 *
 * Revision 1.4  2021-03-04 22:58:55+05:30  Cprogrammer
 * generic tls.c for connect, accept
 *
 * Revision 1.3  2021-03-04 11:45:18+05:30  Cprogrammer
 * match host with common name
 *
 * Revision 1.2  2021-03-04 00:28:07+05:30  Cprogrammer
 * fixed compilation for non tls
 *
 * Revision 1.1  2021-03-03 22:26:54+05:30  Cprogrammer
 * Initial revision
 */
