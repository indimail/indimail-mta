/*
 * $Log: ipalloc.c,v $
 * Revision 1.9  2020-11-22 23:11:05+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.8  2020-05-10 17:46:42+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.7  2019-07-19 13:47:45+05:30  Cprogrammer
 * 2nd argument of ipalloc_readyplus() should be unsigned int
 *
 * Revision 1.6  2018-05-26 12:41:01+05:30  Cprogrammer
 * fixed typo with #undef
 *
 * Revision 1.5  2004-10-22 20:25:58+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-11 13:54:24+05:30  Cprogrammer
 * prevent inclusion of ipalloc.h with prototypes
 *
 * Revision 1.3  2004-10-09 23:23:56+05:30  Cprogrammer
 * prevent inclusion of prototype from ipalloc.h
 *
 * Revision 1.2  2004-07-17 21:19:15+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "alloc.h"
#include "gen_allocdefs.h"
#include "ip.h"
#include "ipalloc.h"

GEN_ALLOC_readyplus(ipalloc, struct ip_mx, ix, len, a, 10, ipalloc_readyplus)
GEN_ALLOC_append(ipalloc, struct ip_mx, ix, len, a, 10, ipalloc_readyplus, ipalloc_append)

void
getversion_ipalloc_c()
{
	static char    *x = "$Id: ipalloc.c,v 1.9 2020-11-22 23:11:05+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
