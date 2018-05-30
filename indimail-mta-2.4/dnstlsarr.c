/*
 * $Log: dnstlsarr.c,v $
 * Revision 1.8  2018-05-31 02:20:55+05:30  Cprogrammer
 * added option to query mx records before fetching TLSA Resource Records
 *
 * Revision 1.7  2018-05-30 20:17:38+05:30  Cprogrammer
 * added prototype for scan_int()
 *
 * Revision 1.6  2018-05-30 20:10:54+05:30  Cprogrammer
 * added options to do complete DANE verification
 *
 * Revision 1.5  2018-05-28 19:56:56+05:30  Cprogrammer
 * exit with 111 for wrong usage
 *
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
#if defined(HASTLSA) && defined(TLS)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "stralloc.h"
#include "fmt.h"
#include "dns.h"
#include "dnsdoe.h"
#include "tlsarralloc.h"
#include "sgetopt.h"
#include "tls.h"
#include "control.h"
#include "scan.h"
#include "now.h"

char            temp[IPFMT + FMT_ULONG];
int             timeoutssl = 300;
int             timeoutconnect = 60;
int             verbose;
stralloc        helohost = { 0 };
stralloc        sahost = { 0 };

extern stralloc sa;
extern tlsarralloc ta;
extern ipalloc  ia;

void            die_nomem();
void            out(char *);
void            flush();
void            die_control(char *);
int             do_dane_validation(char *, int);

void
pusage()
{
	substdio_puts(subfderr,      "usage: dnstlsarr [-p port] [-c timeoutc] [-t timeoutr] [host\n");
	substdio_putsflush(subfderr, "         -p port  - port to connect to\n");
	substdio_putsflush(subfderr, "         -c timoutc - Timeout for connection to remote\n");
	substdio_putsflush(subfderr, "         -t timoutr - Timeout for data from remote\n");
	substdio_putsflush(subfderr, "         -v level - verbosity level for STARTTLS\n");
	substdio_putsflush(subfderr, "         -s Initiate STARTTLS to initiate DANE verification\n");
	_exit (111);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             opt, k, j, i, query_mx = 0, verify = 0;
	char           *port = "25", *host = (char *) 0;
	char            hex[2];
	unsigned long   r;

	while ((opt = getopt(argc,argv,"msv:p:c:t:")) != opteof) {
    	switch(opt)
		{
		case 'p':
			port = optarg;
			break;
		case 'm':
			query_mx = 1;
			break;
		case 's':
			verify = 1;
			break;
		case 'c': /*- timeoutconnect */
			scan_int(optarg, &timeoutconnect);
			break;
		case 't': /*- timeoutconnect */
			scan_int(optarg, &timeoutssl);
			break;
		case 'v':
			verbose = *optarg - '0';
			break;
		}
	}
	if (optind < argc)
		host = argv[optind++];
	else
		pusage();
	if (verify) { /*- DANE Verification */
		if (control_init() == -1)
			die_control("me");
		if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
			die_control("helohost");
		if (query_mx) {
			if (!stralloc_copys(&sahost, host)) {
				substdio_putsflush(subfderr, "out of memory\n");
				_exit(111);
			}
			dns_init(0);
			r = now() + getpid();
			dnsdoe(dns_mxip(&ia, &sahost, r));
			for (j = 0; j < ia.len; ++j) {
				out("checking ");
				out(ia.ix[j].fqdn);
				out("\n");
				flush();
				if (!(i = do_dane_validation(ia.ix[j].fqdn, 25)))
					return (i);
			}
			return (1);
		} else {
			out("checking ");
			out(host);
			out("\n");
			flush();
			return (do_dane_validation(host, 25));
		}
	}
	/*- DANE TLSA RR display mode */
	dns_init(0);
	if (query_mx) {
		if (!stralloc_copys(&sahost, host)) {
			substdio_putsflush(subfderr, "out of memory\n");
			_exit(111);
		}
		r = now() + getpid();
		dnsdoe(dns_mxip(&ia, &sahost, r));
		for (k = 0; k < ia.len; ++k) {
			if (!stralloc_copyb(&sa, "_", 1))
				die_nomem();
			if (!stralloc_cats(&sa, port))
				die_nomem();
			if (!stralloc_catb(&sa, "._tcp.", 6))
				die_nomem();
			if (!stralloc_cats(&sa, ia.ix[k].fqdn))
				die_nomem();
			out("MX ");
			out(ia.ix[k].fqdn);
			switch(ia.ix[k].af)
			{
			case AF_INET:
				out(" IPv4 ");
				substdio_put(subfdout, temp, ip4_fmt(temp, &ia.ix[k].addr.ip));
			break;
#ifdef IPV6
			case AF_INET6:
				out(" IPv6 ");
				substdio_put(subfdout, temp, ip6_fmt(temp, &ia.ix[k].addr.ip6));
			break;
#endif
			default:
				substdio_puts(subfdout, "Unknown address family = ");
				substdio_put(subfdout, temp, fmt_ulong(temp, ia.ix[k].af));
			}
			substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ia.ix[k].pref));
			out("\n");
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
		}
	} else {
		if (!stralloc_copyb(&sa, "_", 1))
			die_nomem();
		if (!stralloc_cats(&sa, port))
			die_nomem();
		if (!stralloc_catb(&sa, "._tcp.", 6))
			die_nomem();
		if (!stralloc_cats(&sa, host))
			die_nomem();
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
	}
	_exit(0);
}
#else
#warning "not compiled with -DHASTLSA -DTLS"
int
main()
{
	substdio_puts(subfderr, "not compiled with -DHASTLSA & -DTLS. Check conf-tlsa, conf-tls\n");
	substdio_flush(subfderr);
	_exit (100);
}
#endif

void
getversion_dnstlsarr_c()
{
	static char    *x = "$Id: dnstlsarr.c,v 1.8 2018-05-31 02:20:55+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
