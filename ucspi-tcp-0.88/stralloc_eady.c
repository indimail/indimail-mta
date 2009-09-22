/*
 * $Log: stralloc_eady.c,v $
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "alloc.h"
#include "stralloc.h"
#include "gen_allocdefs.h"

GEN_ALLOC_ready(stralloc, char, s, len, a, i, n, x, 30, stralloc_ready)
GEN_ALLOC_readyplus(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus)
