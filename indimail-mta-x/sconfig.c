/*
 * $Log: sconfig.c,v $
 * Revision 1.4  2010-07-21 09:17:58+05:30  Cprogrammer
 * minor coding style change
 *
 * Revision 1.3  2004-10-22 20:30:08+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:38:52+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-05-14 00:45:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "open.h"
#include "substdio.h"
#include "error.h"
#include "getln.h"
#include "stralloc.h"
#include "sconfig.h"
#include "env.h"
#include <unistd.h>

int
config_default(c, s)
	config_str     *c;
	char           *s;
{
	if (c->flagconf || !s)
		return 0;
	if (!stralloc_copys(&c->sa, s))
		return -1;
	c->flagconf = 1;
	return 0;
}

int
config_copy(c, d)
	config_str     *c;
	config_str     *d;
{
	if (c->flagconf || !d->flagconf)
		return 0;
	if (!stralloc_copy(&c->sa, &d->sa))
		return -1;
	c->flagconf = 1;
	return 0;
}

int
config_env(c, s)
	config_str     *c;
	char           *s;
{
	if (c->flagconf || !(s = env_get(s)))
		return 0;
	if (!stralloc_copys(&c->sa, s))
		return -1;
	c->flagconf = 1;
	return 0;
}

static void
process(sa)
	stralloc       *sa;
{
	int             i;

	while (sa->len > 0)
	{
		switch (sa->s[sa->len - 1])
		{
		case '\n':
		case ' ':
		case '\t':
			--sa->len;
			break;
		default:
			for (i = 0; i < sa->len; ++i)
				if (sa->s[i] == 0)
					sa->s[i] = '\n';
			return;
		}
	}
}

static char     inbuf[128];
static stralloc line = { 0 };

int
config_readline(c, fn)
	config_str     *c;
	char           *fn;
{
	substdio        ss;
	int             fd;
	int             match;

	if (c->flagconf)
		return 0;
	if ((fd = open_read(fn)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof inbuf);
	if (getln(&ss, &line, &match, '\n') == -1)
	{
		close(fd);
		return -1;
	}
	close(fd);
	process(&line);
	if (!stralloc_copy(&c->sa, &line))
		return -1;
	c->flagconf = 1;
	return 0;
}

int
config_readfile(c, fn)
	config_str     *c;
	char           *fn;
{
	substdio        ss;
	int             fd;
	int             match;

	if (c->flagconf)
		return 0;
	if (!stralloc_copys(&c->sa, ""))
		return -1;
	if ((fd = open_read(fn)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof inbuf);
	for (;;)
	{
		if (getln(&ss, &line, &match, '\n') == -1)
		{
			close(fd);
			return -1;
		}
		process(&line);
		if (!stralloc_0(&line))
		{
			close(fd);
			return -1;
		}
		if (line.s[0] && line.s[0] != '#' && !stralloc_cat(&c->sa, &line))
		{
			close(fd);
			return -1;
		}
		if (!match)
			break;
	}
	close(fd);
	c->flagconf = 1;
	return 0;
}

void
getversion_sconfig_c()
{
	static char    *x = "$Id: sconfig.c,v 1.4 2010-07-21 09:17:58+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
