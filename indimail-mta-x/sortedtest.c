/*
 * $Log: sortedtest.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2008-06-03 23:24:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "sorted.h"
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"

static stralloc sa;

int
main(int argc, char **argv)
{
	sorted          sl = { 0 };
	int             i;

	while (argv[1])
	{
		if (!stralloc_copys(&sa, argv[1]))
			_exit(2);
		if (!sorted_insert(&sl, &sa))
			_exit(3);
		argv++;
	}
	i = 0;
	while (i < sl.len)
	{
		substdio_put(subfdout, sl.p[i].s, sl.p[i].len);
		substdio_puts(subfdout, " ");
		i++;
	}
	substdio_puts(subfdout, "\n");
	substdio_flush(subfdout);
	_exit(0);
}

void
getversion_sortedtest_c()
{
	const char     *x = "$Id: sortedtest.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
