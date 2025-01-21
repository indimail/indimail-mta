/*
 * $Id: tai64n.c,v 1.6 2025-01-21 23:35:54+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include "timestamp.h"
#include "substdio.h"

ssize_t
mywrite(int fd, char *buf, size_t len)
{
	int             w;
	w = write(fd, buf, len);
	if (w == -1)
		_exit(111);
	return w;
}

char            outbuf[2048];
substdio        out = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) mywrite, 1, outbuf, sizeof outbuf);

ssize_t
myread(int fd, char *buf, size_t len)
{
	int             r;
	substdio_flush(&out);
	r = read(fd, buf, len);
	if (r == -1)
		_exit(111);
	return r;
}

char            inbuf[1024];
substdio        in = SUBSTDIO_FDBUF(myread, 0, inbuf, sizeof inbuf);
char            stamp[TIMESTAMP + 1];

int
main()
{
	char            ch;

	for (;;)
	{
		if (substdio_get(&in, &ch, 1) != 1)
			_exit(0);

		timestamp(stamp);
		stamp[TIMESTAMP] = ' ';
		substdio_put(&out, stamp, TIMESTAMP + 1);

		for (;;)
		{
			substdio_put(&out, &ch, 1);
			if (ch == '\n')
				break;
			if (substdio_get(&in, &ch, 1) != 1)
				_exit(0);
		}
	}
}

void
getversion_tai64n_c()
{
	const char     *x = "$Id: tai64n.c,v 1.6 2025-01-21 23:35:54+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: tai64n.c,v $
 * Revision 1.6  2025-01-21 23:35:54+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.5  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2008-07-15 19:54:22+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.3  2004-10-22 20:31:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-09 23:35:13+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.1  2003-12-31 19:32:07+05:30  Cprogrammer
 * Initial revision
 *
 */
