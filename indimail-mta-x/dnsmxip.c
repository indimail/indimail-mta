/*
 * $Log: dnsmxip.c,v $
 * Revision 1.10  2022-08-21 17:58:04+05:30  Cprogrammer
 * fix compilation error when TLS is not defined in conf-tls
 *
 * Revision 1.9  2020-11-24 13:44:56+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2018-05-17 16:10:24+05:30  Cprogrammer
 * print the mx domain along with the ip
 *
 * Revision 1.7  2015-08-24 19:05:34+05:30  Cprogrammer
 * replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.6  2005-06-11 21:29:31+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.5  2004-10-22 20:24:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:18:27+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "fmt.h"
#include "dns.h"
#include "dnsdoe.h"
#include "ip.h"
#include "ipalloc.h"
#include "now.h"

char            temp[IPFMT + FMT_ULONG];

stralloc        sa = { 0 };
ipalloc         ia = { 0 };

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             j;
	unsigned long   r;

	if (!argv[1])
		_exit(100);

	if (!stralloc_copys(&sa, argv[1]))
	{
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}
	r = now() + getpid();
	dns_init(0);
	dnsdoe(dns_mxip(&ia, &sa, r));
	for (j = 0; j < ia.len; ++j)
	{
#ifdef TLS
		substdio_puts(subfdout, ia.ix[j].fqdn);
		substdio_put(subfdout, " ", 1);
#endif
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
		substdio_puts(subfdout, " ");
		substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ia.ix[j].pref));
		substdio_putsflush(subfdout, "\n");
	}
	return(0);
}

void
getversion_dnsmxip_c()
{
	static char    *x = "$Id: dnsmxip.c,v 1.10 2022-08-21 17:58:04+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
