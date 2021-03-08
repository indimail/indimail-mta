/*
 * $Log: tls.h,v $
 * Revision 1.5  2021-03-08 20:02:57+05:30  Cprogrammer
 * fixed datatypes for allwrite(), allwritessl() arguments
 *
 * Revision 1.4  2021-03-06 23:14:58+05:30  Cprogrammer
 * added server functions
 *
 * Revision 1.3  2021-03-04 22:59:59+05:30  Cprogrammer
 * generic tls.c for connect, accept
 *
 * Revision 1.2  2021-03-04 11:45:43+05:30  Cprogrammer
 * added host argument to match host with common name
 *
 * Revision 1.1  2021-03-03 22:27:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TLS_H
#define TLS_H
#include <sys/types.h>
#ifdef TLS
#include <openssl/ssl.h>
#endif

enum tlsmode  {none = 0, client = 1, server = 2};
enum starttls {smtp, pop3, unknown};

ssize_t         saferead(int, char *, size_t, long);
ssize_t         safewrite(int, char *, size_t, long);
ssize_t         allwrite(int, char *, size_t);
#ifdef TLS
SSL_CTX        *tls_init(char *, char *, char *, enum tlsmode);
SSL            *tls_session(SSL_CTX *, int, char *);
int             tls_connect(SSL *, char *);
int             tls_accept(SSL *);
void            ssl_free();
int             translate(SSL *, int, int, unsigned int);
ssize_t         allwritessl(SSL *ssl, char *buf, size_t len);
const char     *myssl_error_str();
#endif

#endif
