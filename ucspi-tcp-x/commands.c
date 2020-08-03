/*
 * $Log: commands.c,v $
 * Revision 1.2  2020-08-03 17:21:09+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <substdio.h>
#include <stralloc.h>
#include <str.h>
#include <case.h>
#include "commands.h"

static stralloc cmd = { 0 };

int
commands(substdio *ss, struct commands *c)
{
	int             i;
	char           *arg;
	char            ch;

	for (;;) {
		if (!stralloc_copys(&cmd, ""))
			return -1;

		for (;;) {
			if ((i = substdio_get(ss, &ch, 1)) != 1)
				return i;
			if (ch == '\n')
				break;
			if (!ch)
				ch = '\n';
			if (!stralloc_append(&cmd, &ch))
				return -1;
		}

		if (cmd.len > 0 && cmd.s[cmd.len - 1] == '\r')
			--cmd.len;

		if (!stralloc_0(&cmd))
			return -1;

		i = str_chr(cmd.s, ' ');
		arg = cmd.s + i;
		while (*arg == ' ')
			++arg;
		cmd.s[i] = 0;

		for (i = 0; c[i].verb; ++i)
		{
			if (case_equals(c[i].verb, cmd.s))
				break;
		}
		c[i].action(arg);
		if (c[i].flush)
			c[i].flush();
	}
}
