/*
 * $Log: dnstxt.c,v $
 * Revision 1.6  2009-04-05 12:51:59+05:30  Cprogrammer
 * added preprocessor warning
 *
 * Revision 1.5  2009-03-14 08:52:07+05:30  Cprogrammer
 * replaced dnsText() with dns_text()
 *
 * Revision 1.4  2008-08-03 18:25:11+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.3  2004-10-22 20:24:34+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-21 21:54:57+05:30  Cprogrammer
 * make dnstxt work with both USE_SPF and DOMAIN_KEYS
 *
 * Revision 1.1  2004-09-05 00:49:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "str.h"
#include "scan.h"
#include "dns.h"
#include "dnsdoe.h"
#include "exit.h"
#include "fmt.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif

#ifdef USE_SPF
strsalloc       ssa = { 0 };
stralloc        sa = { 0 };
#endif

#if !defined(USE_SPF) && !defined(DOMAIN_KEYS)
#warning "not compiled with -DUSE_SPF or -DDOMAIN_KEYS"
#endif

#ifdef DOMAIN_KEYS
char           *dns_text(char *);
#endif

int
main(argc, argv)
	int             argc;
	char          **argv;
{
#ifdef USE_SPF
	int             j;
#elif defined(DOMAIN_KEYS)
	char           *txtrec;
#endif
	int             len;
	char            strnum[FMT_ULONG];

#ifdef USE_SPF
	if (!argv[1])
		_exit(100);
	dns_init(0);
	if (!stralloc_copys(&sa, argv[1]))
	{
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}
	dnsdoe(dns_txt(&ssa, &sa));
	for (len = j = 0; j < ssa.len; ++j)
	{
		substdio_put(subfdout, ssa.sa[j].s, ssa.sa[j].len);
		substdio_puts(subfdout, "\n");
		len += ssa.sa[j].len;
	}
#elif defined(DOMAIN_KEYS)
	if (!argv[1])
		_exit(100);
	txtrec = dns_text(argv[1]);
	if (!str_diff(txtrec, "e=perm;"))
	{
		substdio_putsflush(subfderr, "hard error\n");
		_exit(100);
	} else
	if (!str_diff(txtrec, "e=temp;"))
	{
		substdio_putsflush(subfderr, "soft error\n");
		_exit(100);
	}
	len = str_len(txtrec);
	substdio_puts(subfdout, txtrec);
	substdio_puts(subfdout, "\n");
#else
	substdio_putsflush(subfderr, "not compiled with -DUSE_SPF or -DDOMAIN_KEYS\n");
	_exit(111);
#endif
	strnum[fmt_ulong(strnum, len)] = 0;
	substdio_puts(subfdout, "record length ");
	substdio_puts(subfdout, strnum);
	substdio_putsflush(subfdout, "\n");
	_exit(0);
	/*- Not Reached */
	return(0);
}

void
getversion_dnstxt_c()
{
	static char    *x = "$Id: dnstxt.c,v 1.6 2009-04-05 12:51:59+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
