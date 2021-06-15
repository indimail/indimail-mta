/*
 * $Log: ipmeprint.c,v $
 * Revision 1.13  2021-06-12 17:58:05+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.12  2020-11-24 13:45:39+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.11  2015-08-24 19:06:55+05:30  Cprogrammer
 * replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.10  2008-09-16 16:10:32+05:30  Cprogrammer
 * BUG - Fixed control files not getting opened
 *
 * Revision 1.9  2008-09-16 11:41:31+05:30  Cprogrammer
 * display address family
 *
 * Revision 1.8  2008-06-20 15:44:12+05:30  Cprogrammer
 * useless chdir removed
 *
 * Revision 1.7  2005-06-11 21:31:04+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.6  2004-10-22 20:26:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-07-17 21:19:19+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/socket.h>
#include <unistd.h>
#include "auto_qmail.h"
#include "subfd.h"
#include "substdio.h"
#include "ip.h"
#include "fmt.h"
#include "ipme.h"

char            temp[IPFMT];

int
main()
{
	int             j;

	switch (ipme_init())
	{
	case 0:
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	case -1:
		substdio_putsflush(subfderr, "hard error\n");
		_exit(100);
	}
	for (j = 0; j < ipme.len; ++j)
	{
		switch(ipme.ix[j].af)
		{
		case AF_INET:
			substdio_puts(subfdout, "ipv4 -> ");
			substdio_put(subfdout, temp, ip4_fmt(temp, &ipme.ix[j].addr.ip));
		break;
#ifdef IPV6
		case AF_INET6:
			substdio_puts(subfdout, "ipv6 -> ");
			substdio_put(subfdout, temp, ip6_fmt(temp, &ipme.ix[j].addr.ip6));
		break;
#endif
		default:
			substdio_puts(subfdout, "Unknown address family = ");
			substdio_put(subfdout, temp, fmt_ulong(temp, ipme.ix[j].af));
		}
		substdio_puts(subfdout, "\n");
	}
	substdio_flush(subfdout);
	return(0);
}

void
getversion_ipmeprint_c()
{
	static char    *x = "$Id: ipmeprint.c,v 1.13 2021-06-12 17:58:05+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
