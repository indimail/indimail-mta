/*
 * $Id: dossl.h,v 1.2 2023-01-15 12:27:37+05:30 Cprogrammer Exp mbhangui $
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
					void (*mem_err)(),
					stralloc *);
#ifdef HAVE_STDARG_H
int             do_tls(SSL **, int, int, int, int *, char **, const char *, const char *, int,
                    void (*tlsquit)(const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void (*mem_err)(), void (*ctrl_err)(), void (*write_err)(),
                    void (*quit)(int, int, const char *, ...),
					stralloc *, saa * _ehlokw, int);
#else
int             do_tls(SSL **, int, int, int, int *, char **, char *, char *, int,
                    void (*tlsquit)(const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void (*mem_err)(), void (*ctrl_err)(), void (*write_err)(),
                    void (*quit)(),
					stralloc *, saa * _ehlokw, int);
#endif
#endif
#ifdef HASTLSA
int
                tlsa_vrfy_records(SSL *, char *, int, int, int, const char *,
                    void (*tlsquit)(const char *, const char *, const char *, const char *, const char *, stralloc *),
                    void(*mem_err)(), stralloc *, void(*out)(), void(*flush)(), char **, int);
#endif
#endif /*- _DOSSL_H_ */
