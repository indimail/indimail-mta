/*
 * $Log: tls.c,v $
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
 *
 */
#include <sys/types.h>
#ifdef TLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <case.h>
#endif
#include <sys/select.h>
#include <unistd.h>
#include <strerr.h>
#include <env.h>
#include <error.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include "tls.h"

#ifndef	lint
static char     sccsid[] = "$Id: tls.c,v 1.4 2021-03-04 22:58:55+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef TLS
static int      usessl = 0;
static char    *sslerr_str = 0;
static SSL     *ssl = 0;

void
ssl_free()
{
	if (!ssl)
		return;
	SSL_shutdown(ssl);
	SSL_free(ssl);
}

/*
 * don't want to fail handshake if certificate can't be verified
 */
static int
verify_cb(int preverify_ok, X509_STORE_CTX * ctx)
{
	return 1;
}

static int
check_cert(SSL *myssl, char *host)
{
	X509           *peer;
    char            peer_CN[256];

    if (SSL_get_verify_result(myssl) != X509_V_OK) {
		strerr_warn2("check_cert: Unable to get verify result: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return (1);
	}
    /*
	 * Check the cert chain. The chain length
	 * is automatically checked by OpenSSL when
	 * we set the verify depth in the ctx
	 */

    /*- Check the common name */
    if (!(peer = SSL_get_peer_certificate(myssl))) {
		strerr_warn2("check_cert: Unable to get peer certificate: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return (1);
	}
    X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, sizeof(peer_CN) - 1);
	if (host && case_diffs(peer_CN, host)) {
    	strerr_warn2("hostname doesn't match Common Name ", peer_CN, 0);
		return (1);
	}
	return (0);
}

SSL            *
tls_init(int fd, char *clientcert, char *cafile)
{
	SSL            *myssl;
	SSL_CTX        *ctx;
	BIO            *sbio;
	char           *ciphers;

	if (usessl)
		return ssl;
	SSL_library_init();
	if (!(ctx = SSL_CTX_new(SSLv23_client_method()))) {
		strerr_warn2("SSL_CTX_new: unable to create SSL context: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return ((SSL *) NULL);
	}
	if (!SSL_CTX_load_verify_locations(ctx, clientcert, NULL)) {
		strerr_warn4("TLS unable to load ", clientcert, ": ",
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(ctx);
		return ((SSL *) NULL);
	}
	/*
	 * set the callback here; SSL_set_verify didn't work before 0.9.6c
	 */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
    /*- Set our cipher list */
	if (!(ciphers = env_get("TLS_CIPHER_LIST")))
		ciphers = "PROFILE=SYSTEM";
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	if (!SSL_CTX_set_cipher_list(ctx, ciphers)) {
		strerr_warn4("unable to set ciphers: ", ciphers, ": ",
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(ctx);
		return ((SSL *) NULL);
	}
#endif
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert)) {
		if (SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert, SSL_FILETYPE_PEM) != 1) {
			strerr_warn4("SSL_CTX_use_RSAPrivateKey: unable to load RSA private key: ",
				clientcert, ": ", ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(ctx);
			return ((SSL *) NULL);
		}
		if (SSL_CTX_use_certificate_file(ctx, clientcert, SSL_FILETYPE_PEM) != 1) {
			strerr_warn4("SSL_CTX_use_certificate_file: unable to load certificate: ",
				clientcert, ": ", ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(ctx);
			return ((SSL *) NULL);
		}
		if (cafile && 1 != SSL_CTX_load_verify_locations(ctx, cafile, 0)) {
			strerr_warn4("SSL_CTX_load_verify_locations: unable to load certificate: ",
				clientcert, ": ", ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(ctx);
			return ((SSL *) NULL);
		}
	}
	if (!(myssl = SSL_new(ctx))) {
		strerr_warn2("unable to set up SSL session: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(ctx);
		return ((SSL *) NULL);
	}
	SSL_CTX_free(ctx);
#ifndef CRYPTO_POLICY_NON_COMPLIANCE
	if (!SSL_set_cipher_list(myssl, ciphers)) {
		strerr_warn4("unable to set ciphers: ", ciphers, ": ",
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return ((SSL *) NULL);
	}
#endif
    if (!(sbio = BIO_new_socket(fd, BIO_NOCLOSE))) {
		strerr_warn1("BIO_new_socket failed: ", &strerr_sys);
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return ((SSL *) NULL);
	}
    SSL_set_bio(myssl, sbio, sbio); /*- cannot fail */
	usessl = 1;
	return (ssl = myssl);
}

int
tls_connect(SSL *myssl, char *host)
{
   	if (SSL_connect(myssl) <= 0) {
		strerr_warn2("SSL_connect: SSL Handshake: ", ERR_error_string(ERR_get_error(), 0), 0);
		/*- SSL_get_error(myssl, ret)); -*/
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return -1;
	}
   	if (check_cert(myssl, host)) {
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return -1;
	}
	return 0;
}

int
tls_accept(SSL *myssl)
{
	if (SSL_accept(ssl) <= 0) {
		strerr_warn2("SSL_accept: unable to accept SSL connection", ERR_error_string(ERR_get_error(), 0), 0);
		SSL_shutdown(myssl);
		SSL_free(myssl);
		return -1;
	}
	return 0;
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

ssize_t
ssl_timeoutio(int (*fun) (), long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	int             n = 0;
	const long      end = t + time(NULL);

	do
	{
		fd_set          fds;
		struct timeval  tv;

		const ssize_t   r = buf ? fun(myssl, buf, len) : fun(myssl);
		if (r > 0)
			return r;
		if ((t = end - time(NULL)) < 0)
			break;
		tv.tv_sec = t;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		switch (SSL_get_error(myssl, r))
		{
		default:
			return r;			/*- some other error */
		case SSL_ERROR_WANT_READ:
			FD_SET(rfd, &fds);
			n = select(rfd + 1, &fds, NULL, NULL, &tv);
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
	if (n != -1)
		errno = ETIMEDOUT;
	return -1;
}

ssize_t
ssl_timeoutread(long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	if (!buf)
		return 0;
	if (SSL_pending(myssl))
		return SSL_read(myssl, buf, len);
	return ssl_timeoutio(SSL_read, t, rfd, wfd, myssl, buf, len);
}

ssize_t
ssl_timeoutwrite(long t, int rfd, int wfd, SSL *myssl, char *buf, size_t len)
{
	if (!buf)
		return 0;
	return ssl_timeoutio(SSL_write, t, rfd, wfd, myssl, buf, len);
}
#endif

ssize_t
saferead(int fd, char *buf, size_t len, long timeout)
{
	ssize_t         r;

#ifdef TLS
	if (usessl) {
		if ((r = ssl_timeoutread(timeout, fd, fd, ssl, buf, len)) < 0) {
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				strerr_warn3("saferead: ", sslerr_str, ": ", &strerr_sys);
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
	if (usessl) {
		if ((r = ssl_timeoutwrite(timeout, fd, fd, ssl, buf, len)) < 0) {
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				strerr_warn3("safewrite: ", sslerr_str, ": ", &strerr_sys);
		}
	} else
		r = timeoutwrite(timeout, fd, buf, len);
#else
	r = timeoutwrite(timeout, fd, buf, len);
#endif
	return r;
}

void
getversion_tls_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
