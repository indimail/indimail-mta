/*
 * $Id: dnstlsarr.c,v 1.18 2023-07-13 02:39:46+05:30 Cprogrammer Exp mbhangui $
 */
#include "substdio.h"
#include "subfd.h"
#include "hastlsa.h"
#if defined(HASTLSA) && defined(TLS)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stralloc.h>
#include <fmt.h>
#include <sgetopt.h>
#include <scan.h>
#include <now.h>
#include "dns.h"
#include "dnsdoe.h"
#include "tlsarralloc.h"
#include "control.h"
#include "starttls.h"

static char     temp[IPFMT + FMT_ULONG];
static stralloc sahost = { 0 };
int             timeoutdata = 300;
int             timeoutconn = 60;
int             verbose;
stralloc        helohost = { 0 };
static const char *usage_str =
	"usage: dnstlsarr [-p port] [-c timeoutc] [-t timeoutd] [host\n"
	"         -p port       - port to connect to\n"
	"         -c timoutconn - Timeout for connection to remote\n"
	"         -t timoutdata - Timeout for data from remote\n"
	"         -v level      - verbosity level for STARTTLS\n"
	"         -s            - Initiate STARTTLS to initiate DANE verification\n";

int
main(int argc, char **argv)
{
	int             opt, k, j, i, query_mx = 0, verify = 0;
	const char     *port = "25", *host = (char *) 0;
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
		case 'c': /*- timeoutconn */
			scan_int(optarg, &timeoutconn);
			break;
		case 't': /*- timeoutdata */
			scan_int(optarg, &timeoutdata);
			break;
		case 'v':
			verbose = *optarg - '0';
			break;
		default:
			substdio_putsflush(subfderr, usage_str);
			_exit(100);
		}
	}
	if (optind < argc)
		host = argv[optind++];
	else {
		substdio_putsflush(subfderr, usage_str);
		_exit(100);
	}
	if (verify) { /*- DANE Verification */
		if (control_init() == -1)
			die_control("unable to read control file ", "me");
		if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
			die_control("unable to read control file ", "helohost");
		if (query_mx) {
			if (!stralloc_copys(&sahost, host)) {
				substdio_putsflush(subfderr, "out of memory\n");
				_exit(111);
			}
			dns_init(0);
			r = now() + getpid();
			if (verbose) {
				out("MX query for ");
				out(host);
				out("\n");
				flush();
			}
			dnsdoe(dns_mxip(&ia, &sahost, r));
			for (j = 0; j < ia.len; ++j) {
				out("checking MX host ");
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
		if (verbose) {
			out("MX query for ");
			out(host);
			out("\n");
			flush();
		}
		dnsdoe(dns_mxip(&ia, &sahost, r));
		for (k = 0; k < ia.len; ++k) {
			if (!stralloc_copyb(&sa, "_", 1) ||
					!stralloc_cats(&sa, port) ||
					!stralloc_catb(&sa, "._tcp.", 6) ||
					!stralloc_cats(&sa, ia.ix[k].fqdn))
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
				out(" Unknown address family = ");
				substdio_put(subfdout, temp, fmt_ulong(temp, ia.ix[k].af));
			}
			out(" ");
			substdio_put(subfdout, temp, fmt_ulong(temp, (unsigned long) ia.ix[k].pref));
			out("\n");
			if (verbose) {
				out("TLSA RR query for ");
				substdio_put(subfdout, sa.s, sa.len);
				out("\n");
			}
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
		if (!stralloc_copyb(&sa, "_", 1) ||
				!stralloc_cats(&sa, port) ||
				!stralloc_catb(&sa, "._tcp.", 6) ||
				!stralloc_cats(&sa, host))
			die_nomem();
		if (verbose) {
			out("TLSA RR query for ");
			substdio_put(subfdout, sa.s, sa.len);
			out("\n");
		}
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
	flush();
	_exit(0);
	/*- Not reached */
	return (0);
}
#else
#warning "not compiled with -DHASTLSA -DTLS"
#include <unistd.h>
int             timeoutdata = 300;
int             timeoutconn = 60;

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
	const char     *x = "$Id: dnstlsarr.c,v 1.18 2023-07-13 02:39:46+05:30 Cprogrammer Exp mbhangui $";

#if defined(HASTLSA) && defined(TLS)
	x = sccsidstarttlsh;
#endif
	x++;
}

/*
 * $Log: dnstlsarr.c,v $
 * Revision 1.18  2023-07-13 02:39:46+05:30  Cprogrammer
 * refactored code to reduce loc
 *
 * Revision 1.17  2023-01-06 17:31:49+05:30  Cprogrammer
 * changed scope of variables te,, sahost to static
 * added timeoutdata, timeconn variables for dossl.c
 *
 * Revision 1.16  2023-01-03 19:42:24+05:30  Cprogrammer
 * include tls.h from libqmail
 *
 * Revision 1.15  2022-08-21 17:59:08+05:30  Cprogrammer
 * fix compilation error when TLS is not defined in conf-tls
 *
 * Revision 1.14  2021-05-26 11:05:43+05:30  Cprogrammer
 * use starttls.h for prototypes in starttls.c
 *
 * Revision 1.13  2020-11-24 13:45:02+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.12  2018-06-01 22:52:11+05:30  Cprogrammer
 * exit on incorrect usage
 *
 * Revision 1.11  2018-06-01 16:29:52+05:30  Cprogrammer
 * added verbose messages to indicate the type of dns query
 *
 * Revision 1.10  2018-05-31 19:40:46+05:30  Cprogrammer
 * fixed compiler 'not reached' warning
 *
 * Revision 1.9  2018-05-31 14:41:33+05:30  Cprogrammer
 * included hastlsa.h
 *
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
