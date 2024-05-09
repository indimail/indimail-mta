/*
 * $Log: tai64nunix.c,v $
 * Revision 1.5  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2014-03-18 17:37:56+05:30  Cprogrammer
 * fixed issue with tai timestamp with non-whitespace after 25 characters
 *
 * Revision 1.3  2004-10-22 20:31:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-09 23:35:28+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.1  2003-12-31 19:32:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include "substdio.h"
#include "subfd.h"
#include "fmt.h"

char            num[FMT_ULONG];

void
get(char *ch)
{
	int             r;

	r = substdio_get(subfdin, ch, 1);
	if (r == 1)
		return;
	if (r == 0)
		_exit(0);
	_exit(111);
}

void
out(const char *buf, int len)
{
	if (substdio_put(subfdout, buf, len) == -1)
		_exit(111);
}

time_t          secs;
unsigned long   nanosecs;
unsigned long   u;

int
main()
{
	char            ch, count;

	for (;;)
	{
		get(&ch);
		if (ch == '@')
		{
			secs = 0;
			nanosecs = 0;
			for (count = 0;;count++)
			{
				get(&ch);
				u = ch - '0';
				if (u >= 10)
				{
					u = ch - 'a';
					if (u >= 6 || count == 24)
						break;
					u += 10;
				}
				secs <<= 4;
				secs += nanosecs >> 28;
				nanosecs &= 0xfffffff;
				nanosecs <<= 4;
				nanosecs += u;
			}
			secs -= 4611686018427387914ULL;
			out(num, fmt_uint0(num, secs, 9));
			out(".", 1);
			out(num, fmt_uint0(num, nanosecs, 9));
		}
		for (;;)
		{
			out(&ch, 1);
			if (ch == '\n')
				break;
			get(&ch);
		}
	}
	/*- Not reached */
	return(0);
}

void
getversion_tai64nunix_c()
{
	const char     *x = "$Id: tai64nunix.c,v 1.5 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $";

	x++;
}
