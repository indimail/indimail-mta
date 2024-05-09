/*
 * $Log: dnsip.c,v $
 * Revision 1.10  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2023-09-23 21:21:46+05:30  Cprogrammer
 * use ansic proto for functions.
 *
 * Revision 1.8  2020-11-24 13:44:53+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2015-08-24 19:05:24+05:30  Cprogrammer
 * replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.6  2005-06-11 21:29:25+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.5  2004-10-22 20:24:31+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:18:25+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/socket.h>
#include "substdio.h"
#include "subfd.h"
#include "fmt.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"

char            temp[IPFMT];

stralloc        sa = { 0 };
ipalloc         ia = { 0 };

int
main(int argc, char **argv)
{
	int             j;

	if (!argv[1])
		_exit(100);

	if (!stralloc_copys(&sa, argv[1])) {
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}

	dns_init(0);
	dnsdoe(dns_ip(&ia, &sa));
	for (j = 0; j < ia.len; ++j) {
		switch(ia.ix[j].af)
		{
		case AF_INET:
			substdio_put(subfdout, temp, ip4_fmt(temp, &ia.ix[j].addr.ip));
		break;
#ifdef IPV6
		case AF_INET6:
			substdio_put(subfdout, temp, ip6_fmt(temp, &ia.ix[j].addr.ip6));
		break;
#endif
		default:
			substdio_puts(subfdout, "Unknown address family = ");
			substdio_put(subfdout, temp, fmt_ulong(temp, ia.ix[j].af));
		}
		substdio_putsflush(subfdout, "\n");
	}
	return(0);
}

void
getversion_dnsip_c()
{
	const char     *x = "$Id: dnsip.c,v 1.10 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
