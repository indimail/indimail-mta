/*
 * $Log: 822headerok.c,v $
 * Revision 1.5  2024-01-23 01:20:14+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2022-10-30 17:55:00+05:30  Cprogrammer
 * converted to ansic prototype
 *
 * Revision 1.3  2020-11-24 13:43:38+05:30  Cprogrammer
 * removed exit.h
 *
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
#include "buffer_defs.h"

stralloc        line = { 0 };
int             match;
static char     ssinbuf[BUFSIZE_IN];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);

int
main(int argc, char **argv)
{
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			_exit(111);
		if (!mess822_ok(&line))
			break;
		if (!match)
			_exit(0);
	}
	if (match) {
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
	const char     *x = "$Id: 822headerok.c,v 1.5 2024-01-23 01:20:14+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
