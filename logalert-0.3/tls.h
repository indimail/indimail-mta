/*
 * $Log: tls.h,v $
 * Revision 1.1  2013-05-15 00:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _TLS_H
#define _TLS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

int             tls_init(int, char *);
ssize_t         saferead(int, char *, int, int);
ssize_t         safewrite(int, char *, int, int);
void            ssl_free();
#ifdef HAVE_STDARG_H
int             sslwrt      __P((int, int, char *, ...));
#else
int             sslwrt      ();
#endif

#endif
