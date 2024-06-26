/*
 * $Log: addcr.c,v $
 * Revision 1.3  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2020-11-27 17:32:07+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2005-01-22 00:56:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

int
main()
{
	register int    n;
	register char  *x;
	char            ch;

	for (;;)
	{
		if ((n = substdio_feed(subfdin)) < 0)
			_exit(111);
		if (!n)
			_exit(0);
		x = substdio_PEEK(subfdin);
		substdio_SEEK(subfdin, n);
		while (n > 0)
		{
			ch = *x++;
			--n;
			if (ch == '\n' && substdio_put(subfdout, "\r", 1) == -1)
				_exit(111);
			if (substdio_put(subfdout, &ch, 1) == -1)
				_exit(111);
		}
	}
	return(0);
}

void
getversion_addcr_c()
{
	const char     *x = "$Id: addcr.c,v 1.3 2024-05-09 22:55:54+05:30 mbhangui Exp mbhangui $";

	x++;
}
