/*
 * $Log: dnscname.c,v $
 * Revision 1.9  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.8  2023-09-23 21:21:03+05:30  Cprogrammer
 * use ansic proto for functions
 *
 * Revision 1.7  2020-11-24 13:44:44+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 20:24:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:34:55+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.4  2004-07-17 21:18:18+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "dns.h"
#include "dnsdoe.h"

stralloc        sa = { 0 };

int
main(int argc, char **argv)
{
	if (!argv[1])
		_exit(100);

	if (!stralloc_copys(&sa, argv[1])) {
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}
	dns_init(0);
	dnsdoe(dns_cname(&sa));
	substdio_putflush(subfdout, sa.s, sa.len);
	substdio_putsflush(subfdout, "\n");
	return(0);
}

void
getversion_dnscname_c()
{
	const char     *x = "$Id: dnscname.c,v 1.9 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
