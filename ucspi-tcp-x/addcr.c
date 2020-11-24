/*
 * $Log: addcr.c,v $
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
	static char    *x = "$Id: addcr.c,v 1.1 2005-01-22 00:56:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
