/*
 * $Log: tls.h,v $
 * Revision 1.2  2008-06-01 15:34:00+05:30  Cprogrammer
 * new function ssl_error_str()
 *
 * Revision 1.1  2004-07-30 17:35:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TLS_H
#define TLS_H

#include <openssl/ssl.h>

extern int      smtps;
extern SSL     *ssl;

void            ssl_free(SSL *myssl);
void            ssl_exit(int status);
#define _exit ssl_exit

const char     *ssl_error(void);
const char     *ssl_error_str(void);
const char     *ssl_strerror(void);

#endif
