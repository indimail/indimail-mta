/*
 * $Log: tai64nlocal.c,v $
 * Revision 1.4  2014-03-18 17:37:46+05:30  Cprogrammer
 * fixed SIGSEGV
 *
 * Revision 1.3  2004-10-22 20:31:25+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-09 23:35:21+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.1  2003-12-31 19:32:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
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
struct tm      *t;

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
			t = localtime(&secs);
			out(num, fmt_ulong(num, 1900 + t->tm_year));
			out("-", 1);
			out(num, fmt_uint0(num, 1 + t->tm_mon, 2));
			out("-", 1);
			out(num, fmt_uint0(num, t->tm_mday, 2));
			out(" ", 1);
			out(num, fmt_uint0(num, t->tm_hour, 2));
			out(":", 1);
			out(num, fmt_uint0(num, t->tm_min, 2));
			out(":", 1);
			out(num, fmt_uint0(num, t->tm_sec, 2));
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
	return 0;
}

void
getversion_tai64nlocal_c()
{
	const char     *x = "$Id: tai64nlocal.c,v 1.4 2014-03-18 17:37:46+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
