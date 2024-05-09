/*
 * $Log: delcr.c,v $
 * Revision 1.2  2020-11-27 17:32:15+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2005-01-22 00:57:09+05:30  Cprogrammer
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
	register int    flagcr = 0;

	for (;;)
	{
		if ((n = substdio_feed(subfdin)) < 0)
			_exit(111);
		if (!n)
		{
			if (flagcr && substdio_put(subfdout, "\r", 1) == -1)
				_exit(111);
			if (substdio_flush(subfdout) == -1)
				_exit(111);
			_exit(0);
		}
		x = substdio_PEEK(subfdin);
		substdio_SEEK(subfdin, n);
		while (n > 0)
		{
			ch = *x++;
			--n;
			if (!flagcr)
			{
				if (ch == '\r')
				{
					flagcr = 1;
					continue;
				}
				if (substdio_put(subfdout, &ch, 1) == -1)
					_exit(111);
				continue;
			}
			if (ch != '\n')
			{
				if (substdio_put(subfdout, "\r", 1) == -1)
					_exit(111);
				if (ch == '\r')
					continue;
			}
			flagcr = 0;
			if (substdio_put(subfdout, &ch, 1) == -1)
				_exit(111);
		}
	}
	return(0);
}

void
getversion_delcr_c()
{
	const char     *x = "$Id: delcr.c,v 1.2 2020-11-27 17:32:15+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
