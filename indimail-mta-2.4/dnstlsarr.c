/*
 * $Log: dnstlsarr.c,v $
 * Revision 1.4  2018-05-26 19:17:35+05:30  Cprogrammer
 * fixed program name in usage
 *
 * Revision 1.3  2018-05-26 19:08:41+05:30  Cprogrammer
 * removed -m option to query mx records
 *
 * Revision 1.2  2018-05-26 16:10:14+05:30  Cprogrammer
 * removed leftover debugging code
 *
 * Revision 1.1  2018-05-26 12:37:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "subfd.h"
#include "exit.h"
#ifdef HASTLSA
#include "stralloc.h"
#include "fmt.h"
#include "dns.h"
#include "dnsdoe.h"
#include "tlsarralloc.h"
#include "sgetopt.h"

char            temp[FMT_ULONG];
stralloc        sa = { 0 };
tlsarralloc     ta = { 0 };

void
die_nomem()
{
	substdio_putsflush(subfderr, "out of memory\n");
	_exit(111);
}

void
usage()
{
	substdio_puts(subfderr,      "usage: dnstlsarr [-p port] host\n");
	substdio_putsflush(subfderr, "         -p port  port to connect to\n");
	_exit (100);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             opt, j, i;
	char           *port = "25", *host = (char *) 0;
	char            hex[2];

	while ((opt = getopt(argc,argv,"p:")) != opteof) {
    	switch(opt)
		{
		case 'p':
			port = optarg;
			break;
		}
	}
	if (optind < argc)
		host = argv[optind++];
	else
		usage();
	if (!stralloc_copyb(&sa, "_", 1))
		die_nomem();
	if (!stralloc_cats(&sa, port))
		die_nomem();
	if (!stralloc_catb(&sa, "._tcp.", 6))
		die_nomem();
	if (!stralloc_cats(&sa, host))
		die_nomem();
	dns_init(0);
	dnsdoe(dns_tlsarr(&ta, &sa));
	for (j = 0; j < ta.len; ++j) {
		substdio_put(subfdout, ta.rr[j].host, ta.rr[j].hostlen);
		substdio_put(subfdout, " ttl=", 5);
		substdio_put(subfdout, temp, fmt_ulong(temp, ta.rr[j].ttl));
		substdio_put(subfdout, " ", 1);
		substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ta.rr[j].usage));
		substdio_put(subfdout, " ", 1);
		substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ta.rr[j].selector));
		substdio_put(subfdout, " ", 1);
		substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ta.rr[j].mtype));
		substdio_put(subfdout, " ", 1);
		for (i = 0; i < ta.rr[j].data_len; i++) {
			fmt_hexbyte(hex, (ta.rr[j].data + i)[0]);
			substdio_put(subfdout, hex, 2);
		}
		substdio_putsflush(subfdout, "\n");
	}
	_exit(0);
}
#else
#warning "not compiled with -DHASTLSA"
int
main()
{
	substdio_puts(subfderr, "not compiled with -DHASTLSA. Check conf-tlsa\n");
	substdio_flush(subfderr);
	_exit (100);
}
#endif

void
getversion_dnstlsarr_c()
{
	static char    *x = "$Id: dnstlsarr.c,v 1.4 2018-05-26 19:17:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
