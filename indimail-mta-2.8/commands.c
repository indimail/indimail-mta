/*
 * $Log: commands.c,v $
 * Revision 1.6  2005-05-13 13:13:43+05:30  Cprogrammer
 * 2GB limit bug
 *
 * Revision 1.5  2004-10-22 20:24:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:17:42+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "commands.h"
#include "substdio.h"
#include "stralloc.h"
#include "str.h"
#include "case.h"

static stralloc cmd = { 0 };
extern int      ctl_maxcmdlen;

int
commands(ss, c)
	substdio       *ss;
	struct commands *c;
{
	unsigned int    i;
	char           *arg;

	for (;;)
	{
		if (!stralloc_copys(&cmd, ""))
			return -1;
		for (;;)
		{
			if (!stralloc_readyplus(&cmd, 1))
				return -1;
			i = substdio_get(ss, cmd.s + cmd.len, 1);
			if (i != 1)
				return i;
			if (cmd.s[cmd.len] == '\n')
				break;
			++cmd.len;
			if (ctl_maxcmdlen && cmd.len > ctl_maxcmdlen)
				return -1;
		}
		if (cmd.len > 0 && cmd.s[cmd.len - 1] == '\r')
			--cmd.len;
		cmd.s[cmd.len] = 0;
		i = str_chr(cmd.s, ' ');
		arg = cmd.s + i;
		while (*arg == ' ')
			++arg;
		cmd.s[i] = 0;
		for (i = 0; c[i].text; ++i)
		{
			if (case_equals(c[i].text, cmd.s))
				break;
		}
		if(!(c[i].text) && (*(cmd.s) == 'x' || *(cmd.s) == 'X'))
			c[i].fun("unimplemented");
		else
			c[i].fun(arg);
		if (c[i].flush)
			c[i].flush();
	}
}

void
getversion_commands_c()
{
	static char    *x = "$Id: commands.c,v 1.6 2005-05-13 13:13:43+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
