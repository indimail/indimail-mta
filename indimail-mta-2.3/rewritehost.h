/*
 * $Log: rewritehost.h,v $
 * Revision 1.2  2004-10-11 14:01:28+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-16 01:20:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef REWRITEHOST_H
#define REWRITEHOST_H
#include "stralloc.h"

int             rewritehost(stralloc *, char *, unsigned int, stralloc *);
int             rewritehost_addr(stralloc *, char *, unsigned int, stralloc *);
int             rewritehost_list(stralloc *, char *, unsigned int, stralloc *);

#endif
