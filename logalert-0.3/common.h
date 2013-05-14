/*
 * $Log: common.h,v $
 * Revision 1.1  2013-05-15 00:13:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef COMMON_H
#define COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int64_t mdir_t;
typedef uint64_t umdir_t;
#else
typedef long long mdir_t;
typedef unsigned long long umdir_t;
#define PRId64 "lld"
#define PRIu64 "llu"
#define SCNd64 "lld"
#define SCNu64 "llu"
#endif

int             tcpopen(char *, char *, int);
int             tcpbind(char *, char *, int);
int             scopy(char *, const char *, const int);
int             slen(const char *);
int             r_mkdir(char *, mode_t, uid_t, gid_t);
void            getEnvConfigInt(long *, char *, long);
int             sockread(int, char *, int);
#ifdef HAVE_STDARG_H
int             filewrt     __P((int, char *, ...));
void            debug       __P((char *, ...));
#else
int             filewrt     ();
int             debug       ();
#endif

#endif /*- #ifdef HAVE_CONFIG_H */

#endif /*- #ifdef COMMON_H */
