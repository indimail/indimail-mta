/*
 * $Log: addrlist.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.2  2004-10-22 20:16:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <strerr.h>
#include <subfd.h>
#include <getln.h>
#include <mess822.h>
#include <noreturn.h>

#define FATAL "addrlist: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
myput(buf)
	char           *buf;
{
	char            ch;

	while ((ch = *buf))
	{
		if (ch == '\n')
			ch = 0;
		substdio_put(subfdout, &ch, 1);
		++buf;
	}
}

stralloc        line = { 0 };
int             match;

stralloc        addrlist = { 0 };
stralloc        quoted = { 0 };

int
main()
{
	int             i;
	int             j;

	for (;;)
	{
		if (getln(subfdin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!line.len)
			break;
		if (match)
			--line.len;

		substdio_puts(subfdout, "input {");
		substdio_put(subfdout, line.s, line.len);
		substdio_puts(subfdout, "}\n");

		for (i = 0; i < line.len; ++i)
			if (line.s[i] == 0)
				line.s[i] = '\n';
		if (!stralloc_0(&line))
			nomem();

		if (!mess822_addrlist(&addrlist, line.s))
			nomem();

		for (j = i = 0; j < addrlist.len; ++j)
			if (!addrlist.s[j])
			{
				if (addrlist.s[i] == '(')
				{
					substdio_puts(subfdout, "comment {");
					myput(addrlist.s + i + 1);
					substdio_puts(subfdout, "}\n");
				} else
				if (addrlist.s[i] == '+')
				{
					substdio_puts(subfdout, "address {");
					myput(addrlist.s + i + 1);
					substdio_puts(subfdout, "}\n");
				}
				i = j + 1;
			}

		if (!mess822_quotelist(&quoted, &addrlist))
			nomem();
		substdio_puts(subfdout, "rewrite {");
		substdio_put(subfdout, quoted.s, quoted.len);
		substdio_puts(subfdout, "}\n");

		substdio_puts(subfdout, "\n");
		if (!match)
			break;
	}

	substdio_flush(subfdout);
	_exit(0);
}

void
getversion_addrlist_c()
{
	const char     *x = "$Id: addrlist.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
