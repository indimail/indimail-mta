/*
 * $Id: dossl.h,v 1.4 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef _DOSSL_H_
#define _DOSSL_H_

#include <openssl/ssl.h>
#include <stralloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include "varargs.h"

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
#ifdef TLS
void            do_pkix(SSL *, const char *, const char *,
                    void (*tlsquit)(const char *, const char *, const char *, const char *, const char *, stralloc *),
			void (*mem_err)(void), stralloc *);
#ifdef HAVE_STDARG_H
int             do_tls(SSL **, int, int, int, int *, char **, const char *, const char *, int,
                    void (*tlsquit) (const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void (*mem_err) (void), void (*ctrl_err)(const char *, const char *), void (*write_err)(void),
                    void (*quit)(int, int, const char *, ...), stralloc *, saa * _ehlokw, int);
#else
int             do_tls(SSL **, int, int, int, int *, char **, char *, char *, int,
                    void (*tlsquit) (const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void (*mem_err) (void), void (*ctrl_err) (const char *, const char *), void (*write_err) (void),
                    void (*quit)(), stralloc *, saa * _ehlokw, int);
#endif
#endif
#ifdef HASTLSA
int
                tlsa_vrfy_records(SSL *, char *, int, int, int, const char *,
                    void (*tlsquit) (const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void (*mem_err) (void), stralloc *, void (*out) (const char *), void (*flush) (void), char **, int);
#endif
#endif /*- _DOSSL_H_ */
