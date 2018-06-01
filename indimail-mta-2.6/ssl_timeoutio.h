/*
 * $Log: ssl_timeoutio.h,v $
 * Revision 1.2  2017-08-08 17:40:22+05:30  Cprogrammer
 * removed extra space
 *
 * Revision 1.1  2004-07-30 17:35:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SSL_TIMEOUTIO_H
#define SSL_TIMEOUTIO_H

#include <openssl/ssl.h>

/*
 * the version is like this: 0xMNNFFPPS: major minor fix patch status 
 */
#if OPENSSL_VERSION_NUMBER < 0x00906000L
# error "Need OpenSSL version at least 0.9.6"
#endif

int             ssl_timeoutconn(long t, int rfd, int wfd, SSL *ssl);
int             ssl_timeoutaccept(long t, int rfd, int wfd, SSL *ssl);
int             ssl_timeoutrehandshake(long t, int rfd, int wfd, SSL *ssl);
int             ssl_timeoutread(long t, int rfd, int wfd, SSL *ssl, char *buf, int len);
int             ssl_timeoutwrite(long t, int rfd, int wfd, SSL *ssl, char *buf, int len);
int             ssl_timeoutio(int (*fun) (), long t, int rfd, int wfd, SSL *ssl, char *buf, int len);

#endif
