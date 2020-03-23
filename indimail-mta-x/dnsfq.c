/*
 * $Log: dnsfq.c,v $
 * Revision 1.8  2005-08-23 17:32:23+05:30  Cprogrammer
 * ipv6 correction
 *
 * Revision 1.7  2005-06-11 21:28:32+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.6  2004-10-22 20:24:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-08-14 02:18:00+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.4  2004-07-17 21:18:23+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/socket.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif
#include "exit.h"

stralloc        sa = { 0 };
ipalloc         ia = { 0 };

int
main(argc, argv)
	int             argc;
	char          **argv;
{
#ifdef USE_SPF
	int             j;
	strsalloc       ssa = { 0 };
#endif

	if (!argv[1])
		_exit(100);
	if (!stralloc_copys(&sa, argv[1]))
	{
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}
	dns_init(1);
	dnsdoe(dns_ip(&ia, &sa));
	if (ia.len <= 0)
	{
		substdio_putsflush(subfderr, "no IP addresses\n");
		_exit(100);
	}
#ifdef USE_SPF
	if (ia.ix[0].af == AF_INET)
		dnsdoe(dns_ptr(&ssa, &ia.ix[0].addr.ip));
#ifdef IPV6
	else
		dnsdoe(dns_ptr6(&ssa, &ia.ix[0].addr.ip6));
#endif
	for (j = 0; j < ssa.len; ++j)
	{
		substdio_putflush(subfdout, ssa.sa[j].s, ssa.sa[j].len);
		substdio_putsflush(subfdout, "\n");
	}
#else
	if (ia.ix[0].af == AF_INET)
		dnsdoe(dns_ptr(&sa, &ia.ix[0].addr.ip));
#ifdef IPV6
	else
		dnsdoe(dns_ptr6(&sa, &ia.ix[0].addr.ip6));
#endif
	substdio_putflush(subfdout, sa.s, sa.len);
	substdio_putsflush(subfdout, "\n");
#endif
	return (0);
}

void
getversion_dnsfq_c()
{
	static char    *x = "$Id: dnsfq.c,v 1.8 2005-08-23 17:32:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
