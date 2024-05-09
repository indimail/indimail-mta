/*
 * $Log: dnstxt.c,v $
 * Revision 1.9  2023-09-23 21:22:06+05:30  Cprogrammer
 * use ansic proto for functions
 *
 * Revision 1.8  2020-11-24 13:45:05+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2017-09-12 14:09:27+05:30  Cprogrammer
 * refactored code
 *
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
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "str.h"
#include "fmt.h"

char           *dns_text(char *);

int
main(int argc, char **argv)
{
	char           *txtrec;
	int             len;
	char            strnum[FMT_ULONG];

	if (!argv[1])
		_exit(100);
	txtrec = dns_text(argv[1]);
	if (!str_diff(txtrec, "e=perm;")) {
		substdio_putsflush(subfderr, "hard error\n");
		_exit(100);
	} else
	if (!str_diff(txtrec, "e=temp;")) {
		substdio_putsflush(subfderr, "soft error\n");
		_exit(100);
	}
	len = str_len(txtrec);
	substdio_puts(subfdout, txtrec);
	substdio_puts(subfdout, "\n");
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
	const char     *x = "$Id: dnstxt.c,v 1.9 2023-09-23 21:22:06+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
