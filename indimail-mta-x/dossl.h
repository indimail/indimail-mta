/*
 * $Id: dossl.h,v 1.1 2023-01-06 17:44:26+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef _DOSSL_H_
#define _DOSSL_H_

#include <openssl/ssl.h>
#include <stralloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
#ifdef TLS
void            do_pkix(SSL *, char *, char *,
                    void (*)(const char *, char *, char *, char *, char *, stralloc *), void(*)(),
                    void(*)(char *, char *, int, int), stralloc *);
int             do_tls(SSL **, int, int, int, int *, char **, char *, char *, int,
                    void (*tlsquit)(const char *, char *, char *, char *, char *, stralloc *),
                    void(*)(), void(*)(), void(*)(),
                    void(*)(char *, char *, int, int), stralloc *, saa * _ehlokw, int);
#endif
#ifdef HASTLSA
int
                tlsa_vrfy_records(SSL *, char *, int, int, int, char *,
                    void (*)(const char *, char *, char *, char *, char *, stralloc *),
                    void(*)(), stralloc *, void(*out)(), void(*flush)(), char **, int);
#endif
#endif /*- _DOSSL_H_ */
