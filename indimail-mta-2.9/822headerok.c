/*
 * $Log: 822headerok.c,v $
 * Revision 1.2  2004-10-22 20:14:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:46:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "error.h"
#include "getln.h"
#include "mess822.h"
#include "exit.h"

stralloc        line = { 0 };
int             match;
static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			_exit(111);
		if (!mess822_ok(&line))
			break;
		if (!match)
			_exit(0);
	}
	if (match)
	{
		--line.len;
		if (line.len && (line.s[line.len - 1] == '\r'))
			--line.len;
	}
	if (!line.len)
		_exit(0);
	_exit(100);
}

void
getversion_822headerok_c()
{
	static char    *x = "$Id: 822headerok.c,v 1.2 2004-10-22 20:14:12+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
