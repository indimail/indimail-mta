/*
 * $Log: ssl_timeoutio.c,v $
 * Revision 1.4  2017-08-08 23:56:38+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.3  2005-06-03 09:06:41+05:30  Cprogrammer
 * code beautification
 *
 * Revision 1.2  2004-10-22 20:30:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-30 17:35:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef TLS
#include "select.h"
#include "error.h"
#include "ndelay.h"
#include "ssl_timeoutio.h"

int
ssl_timeoutio(int (*fun) (), long t, int rfd, int wfd, SSL *ssl, char *buf, int len)
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
		errno = error_timeout;
	return -1;
}

int
ssl_timeoutaccept(long t, int rfd, int wfd, SSL *ssl)
{
	int             r;

	/*
	 * if connection is established, keep NDELAY 
	 */
	if (ndelay_on(rfd) == -1 || ndelay_on(wfd) == -1)
		return -1;
	if ((r = ssl_timeoutio(SSL_accept, t, rfd, wfd, ssl, NULL, 0)) <= 0)
	{
		ndelay_off(rfd);
		ndelay_off(wfd);
	} else
		SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
	return r;
}

int
ssl_timeoutconn(long t, int rfd, int wfd, SSL *ssl)
{
	int             r;

	/*
	 * if connection is established, keep NDELAY 
	 */
	if (ndelay_on(rfd) == -1 || ndelay_on(wfd) == -1)
		return -1;
	if ((r = ssl_timeoutio(SSL_connect, t, rfd, wfd, ssl, NULL, 0)) <= 0)
	{
		ndelay_off(rfd);
		ndelay_off(wfd);
	} else
		SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
	return r;
}

int
ssl_timeoutrehandshake(long t, int rfd, int wfd, SSL *ssl)
{
	int             r;

	SSL_renegotiate(ssl);
	r = ssl_timeoutio(SSL_do_handshake, t, rfd, wfd, ssl, NULL, 0);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (r <= 0 || SSL_get_state(ssl) == SSL_ST_CONNECT)
#else
	if (r <= 0 || ssl->type == SSL_ST_CONNECT)
#endif
		return r;
	/*
	 * this is for the server only 
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	SSL_set_accept_state(ssl);
#else
	ssl->state = SSL_ST_ACCEPT;
#endif
	return ssl_timeoutio(SSL_do_handshake, t, rfd, wfd, ssl, NULL, 0);
}

int
ssl_timeoutread(long t, int rfd, int wfd, SSL *ssl, char *buf, int len)
{
	if (!buf)
		return 0;
	if (SSL_pending(ssl))
		return SSL_read(ssl, buf, len);
	return ssl_timeoutio(SSL_read, t, rfd, wfd, ssl, buf, len);
}

int
ssl_timeoutwrite(long t, int rfd, int wfd, SSL *ssl, char *buf, int len)
{
	if (!buf)
		return 0;
	return ssl_timeoutio(SSL_write, t, rfd, wfd, ssl, buf, len);
}
#endif

void
getversion_ssl_timeoutio_c()
{
	static char    *x = "$Id: ssl_timeoutio.c,v 1.4 2017-08-08 23:56:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
