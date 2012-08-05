/*
 * $Log: tls.c,v $
 * Revision 2.4  2011-04-08 17:27:19+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.3  2010-04-15 12:49:55+05:30  Cprogrammer
 * included string.h for Mac OS X
 *
 * Revision 2.2  2009-09-16 09:04:28+05:30  Cprogrammer
 * fixed compilation error on mandriva systems
 *
 * Revision 2.1  2009-08-11 16:21:47+05:30  Cprogrammer
 * tls routines
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

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

int
check_cert(SSL *ssl)
{
	X509           *peer;
    char            peer_CN[256];
    
    if (SSL_get_verify_result(ssl) != X509_V_OK)
	{
		fprintf(stderr, "check_cert: Unable to get peer certificate %s\n", 
			ERR_error_string(ERR_get_error(), 0));
		return (1);
	}
    /*
	 * Check the cert chain. The chain length
	 * is automatically checked by OpenSSL when
	 * we set the verify depth in the ctx
	 */

    /*- Check the common name */
    if (!(peer = SSL_get_peer_certificate(ssl)))
	{
		fprintf(stderr, "check_cert: Unable to get peer certificate %s\n", 
			ERR_error_string(ERR_get_error(), 0));
		return (1);
	}
    X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, 256);
	/*-
    if(strcasecmp(peer_CN,host))
    err_exit("Common name doesn't match host name");
	*/
	return (0);
}

int
tls_init(int fd, char *clientcert)
{
	int             ret;
	SSL            *myssl;
	SSL_CTX        *ctx;
	BIO            *sbio;
	char           *ciphers;

	usessl = (access(clientcert, F_OK) ? 0 : 1);
	if (!usessl)
		return (0);
	ciphers = getenv("TLSCLIENTCIPHERS");
	SSL_library_init();
	if (!(ctx = SSL_CTX_new(SSLv23_client_method())))
	{
		fprintf(stderr, "SSL_CTX_new: unable to create SSL context: %s\n",
			ERR_error_string(ERR_get_error(), 0));
		return (1);
	}
	if (!SSL_CTX_load_verify_locations(ctx, clientcert, NULL))
	{
		fprintf(stderr, "TLS unable to load %s: %s\n", clientcert,
			ERR_error_string(ERR_get_error(), 0));
		SSL_CTX_free(ctx);
		return (1);
	}
	/*
	 * set the callback here; SSL_set_verify didn't work before 0.9.6c 
	 */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
    /*- Set our cipher list */
	if(ciphers && !SSL_CTX_set_cipher_list(ctx, ciphers))
	{
		fprintf(stderr, "unable to set ciphers: %s\n", 
			ERR_error_string(ERR_get_error(), 0));
		SSL_CTX_free(ctx);
		return (1);
	}
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert))
	{
		if (SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert, SSL_FILETYPE_PEM) != 1)
		{
			fprintf(stderr, "SSL_CTX_use_RSAPrivateKey: unable to load RSA private key: %s: %s\n",
				clientcert, ERR_error_string(ERR_get_error(), 0));
			SSL_CTX_free(ctx);
			return (1);
		}
		if (SSL_CTX_use_certificate_file(ctx, clientcert, SSL_FILETYPE_PEM) != 1)
		{
			fprintf(stderr, "SSL_CTX_use_certificate_file: unable to load certificate: %s: %s\n",
				clientcert, ERR_error_string(ERR_get_error(), 0));
			SSL_CTX_free(ctx);
			return (1);
		}
	}
	if (!(myssl = SSL_new(ctx)))
	{
		fprintf(stderr, "unable to set up SSL session: %s\n", 
			ERR_error_string(ERR_get_error(), 0));
		SSL_CTX_free(ctx);
		return (1);
	}
	ssl = myssl;
	SSL_CTX_free(ctx);
    if (!(sbio = BIO_new_socket(fd, BIO_NOCLOSE)))
	{
		fprintf(stderr, "BIO_new_socket failed\n");
		SSL_free(myssl);
		return (1);
	}
    SSL_set_bio(myssl, sbio, sbio); /*- cannot fail */
    if ((ret = SSL_connect(myssl)) <= 0)
	{
		SSL_free(myssl);
		fprintf(stderr, "SSL_connect: SSL Handshake: %s\n", 
			ERR_error_string(ERR_get_error(), 0));
			/*- SSL_get_error(myssl, ret)); -*/
		usessl = 0;
		errno = EPROTO;
		return (1);
	}
    return (check_cert(myssl));
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
	return (errno == ETIMEDOUT) ? "timed out" : strerror(errno);
}

int
ssl_timeoutio(int (*fun) (), long t, int rfd, int wfd, SSL * ssl, char *buf, int len)
{
	int             n = 0;
	const long      end = t + time(NULL);

	do
	{
		fd_set          fds;
		struct timeval  tv;

		const int       r = buf ? fun(ssl, buf, len) : fun(ssl);
		if (r > 0)
			return r;
		if ((t = end - time(NULL)) < 0)
			break;
		tv.tv_sec = t;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		switch (SSL_get_error(ssl, r))
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

int
ssl_timeoutread(long t, int rfd, int wfd, SSL * ssl, char *buf, int len)
{
	if (!buf)
		return 0;
	if (SSL_pending(ssl))
		return SSL_read(ssl, buf, len);
	return ssl_timeoutio(SSL_read, t, rfd, wfd, ssl, buf, len);
}

int
ssl_timeoutwrite(long t, int rfd, int wfd, SSL * ssl, char *buf, int len)
{
	if (!buf)
		return 0;
	return ssl_timeoutio(SSL_write, t, rfd, wfd, ssl, buf, len);
}
#endif

int             timeoutread(int, int, char *, int);
int             timeoutwrite(int, int, char *, int);

ssize_t
saferead(fd, buf, len, timeout)
	int             fd;
	char           *buf;
	int             len;
	int             timeout;
{
	int             r;

#ifdef HAVE_SSL
	if (usessl)
	{
		if ((r = ssl_timeoutread(timeout, fd, fd, ssl, buf, len)) < 0)
		{
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				fprintf(stderr, "read: %s\n", sslerr_str);
		}
	} else
		r = timeoutread(timeout, fd, buf, len);
#else
	r = timeoutread(timeout, fd, buf, len);
#endif
	return r;
}

ssize_t
safewrite(fd, buf, len, timeout)
	int             fd;
	char           *buf;
	int             len;
	int             timeout;
{
	int             r;

#ifdef HAVE_SSL
	if (usessl)
	{
		if ((r = ssl_timeoutwrite(timeout, fd, fd, ssl, buf, len)) < 0)
		{
			sslerr_str = (char *) myssl_error_str();
			if (sslerr_str)
				fprintf(stderr, "write: %s\n", sslerr_str);
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
	static char    *x = "$Id: tls.c,v 2.4 2011-04-08 17:27:19+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
