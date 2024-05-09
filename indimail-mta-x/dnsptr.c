/*
 * $Log: dnsptr.c,v $
 * Revision 1.11  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.10  2023-09-23 21:21:55+05:30  Cprogrammer
 * use ansic proto for functions
 *
 * Revision 1.9  2020-11-24 13:44:58+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2015-08-24 19:05:47+05:30  Cprogrammer
 * replace ip_scan() with ip4_scan()
 *
 * Revision 1.7  2005-06-17 21:53:00+05:30  Cprogrammer
 * replaced struct ip_address with a shorter typedef ip_addr
 *
 * Revision 1.6  2004-10-22 20:24:33+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-08-14 02:18:20+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.4  2004-07-17 21:18:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "str.h"
#include "scan.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif

int
main(int argc, char **argv)
{
	ip_addr         ip;
#ifdef USE_SPF
	int             j;
	strsalloc       ssa = { 0 };
#else
	stralloc        sa = { 0 };
#endif

	if (!argv[1])
		_exit(100);
	ip4_scan(argv[1], &ip);
	dns_init(0);
#ifdef USE_SPF
	dnsdoe(dns_ptr(&ssa, &ip));
	for (j = 0; j < ssa.len; ++j) {
		substdio_putflush(subfdout, ssa.sa[j].s, ssa.sa[j].len);
		substdio_putsflush(subfdout, "\n");
	}
#else
	dnsdoe(dns_ptr(&sa, &ip));
	substdio_putflush(subfdout, sa.s, sa.len);
	substdio_putsflush(subfdout, "\n");
#endif
	return (0);
}

void
getversion_dnsptr_c()
{
	const char     *x = "$Id: dnsptr.c,v 1.11 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
