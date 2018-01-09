/*
 * $Log: sorted.h,v $
 * Revision 1.1  2008-06-03 23:21:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SORTED_H
#define SORTED_H

#include "gen_alloc.h"
#include "stralloc.h"

GEN_ALLOC_typedef(sorted, stralloc, p, len, a)

extern int sorted_insert(sorted *, stralloc *);

#endif
