/*
 * $Log: stralloc_eady.c,v $
 * Revision 1.2  2019-05-26 12:04:34+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "gen_allocdefs.h"

GEN_ALLOC_ready(stralloc, char, s, len, a, i, n, x, 30, stralloc_ready)
GEN_ALLOC_readyplus(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus)
