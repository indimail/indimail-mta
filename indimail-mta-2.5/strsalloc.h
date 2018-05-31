/*
 * $Log: strsalloc.h,v $
 * Revision 1.3  2009-06-04 12:09:28+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-10-11 14:09:19+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-08-15 19:57:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef STRSALLOC_H
#define STRSALLOC_H

#include "stralloc.h"
#include "gen_alloc.h"

GEN_ALLOC_typedef(strsalloc, stralloc, sa, len, a)
int      strsalloc_readyplus(strsalloc *, register unsigned int);
int      strsalloc_append(strsalloc *, stralloc *);

#endif
