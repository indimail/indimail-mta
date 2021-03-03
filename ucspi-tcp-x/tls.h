/*
 * $Log: tls.h,v $
 * Revision 1.1  2021-03-03 22:27:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TLS_H
#define TLS_H
#include <sys/types.h>

ssize_t         saferead(int, char *, size_t, long);
ssize_t         safewrite(int, char *, size_t, long);
#ifdef TLS
int             tls_init(int, char *, char *);
void            ssl_free();
#endif

#endif
