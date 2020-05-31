/*
 * $Log: stralloc_pend.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "stralloc.h"
#include "gen_allocdefs.h"

GEN_ALLOC_append(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus, stralloc_append)
