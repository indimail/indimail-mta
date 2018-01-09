/*
 * $Log: dnsdoe.c,v $
 * Revision 1.3  2004-10-22 20:24:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:18:20+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "substdio.h"
#include "subfd.h"
#include "exit.h"
#include "dns.h"
#include "dnsdoe.h"

void
dnsdoe(r)
	int             r;
{
	switch (r)
	{
	case DNS_HARD:
		substdio_putsflush(subfderr, "hard error\n");
		_exit(100);
	case DNS_SOFT:
		substdio_putsflush(subfderr, "soft error\n");
		_exit(111);
	case DNS_MEM:
		substdio_putsflush(subfderr, "out of memory\n");
		_exit(111);
	}
}

void
getversion_dnsdoe_c()
{
	static char    *x = "$Id: dnsdoe.c,v 1.3 2004-10-22 20:24:26+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
