/*
 * $Log: stralloc.h,v $
 * Revision 1.4  2004-10-09 23:34:05+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-06-18 23:01:55+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef STRALLOC_H
#define STRALLOC_H

#include "gen_alloc.h"

GEN_ALLOC_typedef(stralloc, char, s, len, a)
#ifndef _STRALLOC_EADY
int             stralloc_ready(stralloc *, int);
int             stralloc_readyplus(stralloc *, int);
int             stralloc_copy(stralloc *, stralloc *);
int             stralloc_copys(stralloc *, char *);
int             stralloc_copyb(stralloc *, char *, int);
int             stralloc_cat(stralloc *, stralloc *);
int             stralloc_cats(stralloc *, char *);
int             stralloc_catb(stralloc *, char *, int);
int             stralloc_append(stralloc *, char *); /*- beware: this takes a pointer to 1 char */
int             stralloc_starts(stralloc *, char *);
int             stralloc_catulong0(stralloc *, unsigned long, unsigned int);
int             stralloc_catlong0(stralloc *, long, unsigned int);
#else
int             stralloc_ready();
int             stralloc_readyplus();
int             stralloc_copy();
int             stralloc_copys();
int             stralloc_copyb();
int             stralloc_cat();
int             stralloc_cats();
int             stralloc_catb();
int             stralloc_append();
int             stralloc_starts();
int             stralloc_catulong0();
int             stralloc_catlong0();
#endif

#define stralloc_0(sa) stralloc_append(sa,"")
#define stralloc_catlong(sa,l) (stralloc_catlong0((sa),(l),0))
#define stralloc_catuint0(sa,i,n) (stralloc_catulong0((sa),(unsigned long) (i),(n)))
#define stralloc_catint0(sa,i,n) (stralloc_catlong0((sa),(long) (i),(n)))
#define stralloc_catint(sa,i) (stralloc_catlong0((sa),(long) (i),0))

#endif
