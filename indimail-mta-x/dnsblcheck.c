/*
 * $Log: dnsblcheck.c,v $
 * Revision 1.1  2022-06-01 13:02:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <subfd.h>
#include <unistd.h>
#include <sgetopt.h>
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#include <sys/time.h>
#include <env.h>
#include <noreturn.h>
#include "control.h"
#include "dns.h"

stralloc        dnsblhost = { 0 }, dnsbllist = { 0 };

no_return void
die_nomem()
{
	substdio_puts(subfderr, "dnsblcheck: out of memory\n");
	substdio_flush(subfderr);
	_exit(111);
}

/*-
 * dnsbl function
 * adapted from
 * http://qmail-dnsbl.sourceforge.net/
 * Author fabio.busatto@sikurezza.org
 */

static int
dnsblcheck(char **mesg, char *remoteip)
{
	char           *ch;
	static stralloc dnsblbyte = { 0 }, dnsblrev = { 0 };
	static ipalloc  dnsblip = { 0 };
	char            x[IPFMT];
	int             len;

	ch = remoteip;
	if (!stralloc_copys(&dnsblrev, ""))
		die_nomem();
	for (;;) {
		if (!stralloc_copys(&dnsblbyte, ""))
			die_nomem();
		while (ch[0] && (ch[0] != '.')) {
			if (!stralloc_append(&dnsblbyte, ch))
				die_nomem();
			ch++;
		}
		if (!stralloc_append(&dnsblbyte, ".") ||
				!stralloc_cat(&dnsblbyte, &dnsblrev) ||
				!stralloc_copy(&dnsblrev, &dnsblbyte))
			die_nomem();
		if (!ch[0])
			break;
		ch++;
	}
	ch = dnsbllist.s;
	while (ch < (dnsbllist.s + dnsbllist.len)) {
		if (!stralloc_copy(&dnsblhost, &dnsblrev) ||
				!stralloc_cats(&dnsblhost, ch) ||
				!stralloc_0(&dnsblhost))
			die_nomem();
		substdio_put(subfdout, " [", 2);
		substdio_put(subfdout, dnsblhost.s, dnsblhost.len);
		substdio_put(subfdout, " ", 1);
		substdio_flush(subfdout);
		switch (dns_ip(&dnsblip, &dnsblhost))
		{
		case DNS_MEM:
			substdio_put(subfdout, "out of mem] ", 12);
			substdio_flush(subfdout);
			break;
		case DNS_SOFT:
			substdio_put(subfdout, "soft error] ", 12);
			substdio_flush(subfdout);
			break;
		case DNS_HARD:
			substdio_put(subfdout, "hard error] ", 12);
			substdio_flush(subfdout);
			break;
		default:
			len = ip4_fmt(x, &dnsblip.ix->addr.ip);
			substdio_put(subfdout, x, len);
			substdio_put(subfdout, "] ", 2);
			substdio_flush(subfdout);
			return 1;
		}
		while (*ch++);
	}
	return 0;
}

int
main(int argc, char **argv)
{
	char           *mesg, *ip;
	int             opt, r, i;
	struct timeval  tmval1, tmval2;
	char            strnum[FMT_DOUBLE];

	while ((opt = getopt(argc, argv, "d:")) != opteof) {
		switch (opt)
		{
		case 'd':
			if (!stralloc_copys(&dnsbllist, optarg) ||
					!stralloc_0(&dnsbllist))
				die_nomem();
			dnsbllist.len--;
			break;
		default:
			strerr_die1x(100, "USAGE: dnsblcheck -d dnsbl_server_ip ip");
		}
	}
	if (optind + 1 != argc || !dnsbllist.len)
		strerr_die1x(100, "USAGE: dnsblcheck -d dnsbl_server_ip ip");
	ip = argv[optind++];
	substdio_put(subfdout, "IP ", 3);
	substdio_puts(subfdout, ip);
	substdio_put(subfdout, " ... ", 5);
	gettimeofday(&tmval1, 0);
	r = dnsblcheck(&mesg, ip);
	gettimeofday(&tmval2, 0);
	strnum[i = fmt_double(strnum,
			(tmval2.tv_sec + ((double) tmval2.tv_usec / 1000000)) - (tmval1.tv_sec + ((double) tmval1.tv_usec / 1000000)), 4)] = 0;

	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, " secs\n", 6);
	substdio_put(subfdout, dnsblhost.s, dnsblhost.len);
	if (r)
		substdio_put(subfdout, " blacklisted\n", 13);
	else
		substdio_put(subfdout, " OK\n", 4);
	substdio_flush(subfdout);
	_exit(r ? 1 : 0);
}

void
getversion_dnsblcheck_c()
{
	const char     *x = "$Id: dnsblcheck.c,v 1.1 2022-06-01 13:02:09+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
