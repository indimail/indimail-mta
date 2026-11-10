/*
 * $Log: readproctitle.c,v $
 * Revision 1.6  2026-11-10 21:59:57+05:30  Cprogrammer
 * fix compiler warning
 *
 * Revision 1.5  2026-11-10 16:12:27+05:30  mbhangui
 * fix set but unused variable warnings
 *
 * Revision 1.4  2026-06-05 09:21:35+05:30  Cprogrammer
 * fix compiler warning for unused rcs variable
 *
 * Revision 1.3  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:29:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:31:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"

int
main(int argc, char **argv)
{
	char           *buf;
	unsigned int    i, len;
	char            ch;

	if (argc < 2)
		_exit(100);
	buf = argv[argc - 1];
	len = 0;
	while (buf[len])
		buf[len++] = '.';
	if (len < 5)
		_exit(100);

	for (;;)
		switch (read(0, &ch, 1))
		{
		case 1:
			if (ch)
			{
				for (i = 4; i < len; ++i)
					buf[i - 1] = buf[i];
				buf[len - 1] = ch;
			}
			break;
		case 0:
			_exit(0);
		case -1:
			if (errno != error_intr)
				_exit(111);
		}
}

void
getversion_readproctitle_c()
{
	const char     *x = "$Id: readproctitle.c,v 1.6 2026-11-10 21:59:57+05:30 Cprogrammer Exp mbhangui $";

	(void) x;
}
