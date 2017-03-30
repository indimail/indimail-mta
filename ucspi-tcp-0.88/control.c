/*
 * $Log: control.c,v $
 * Revision 1.2  2017-03-30 22:42:20+05:30  Cprogrammer
 * made control_readline() exactly duplicate of qmail-1.03 control_readline()
 *
 * Revision 1.1  2003-12-27 17:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "buffer.h"
#include "stralloc.h"
#include "scan.h"
#include "error.h"
#include "open.h"
#include "getln.h"
#include "env.h"
#include "control.h"
#include <unistd.h>

static char     inbuf[64];
static stralloc line = { 0 };

int
control_readint(i, fn)
	int            *i;
	char           *fn;
{
	unsigned long   u;

	switch (control_readline(&line, fn))
	{
	case 0:
		return 0;
	case -1:
		return -1;
	}
	if (!stralloc_0(&line))
		return -1;
	if (!scan_ulong(line.s, &u))
		return 0;
	*i = u;
	return 1;
}

int
control_readline(sa, fn)
	stralloc       *sa;
	char           *fn;
{
	buffer          ss;
	int             fd, match;
	static char    *controldir;
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.')
	{
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "/etc/indimail/control";
		}
		if (!stralloc_copys(&controlfile, controldir))
			return(-1);
		if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
			return(-1);
		if (!stralloc_cats(&controlfile, fn))
			return(-1);
	} else
	if (!stralloc_copys(&controlfile, fn))
		return(-1);
	if (!stralloc_0(&controlfile))
		return(-1);
	if((fd = open_read(controlfile.s)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	buffer_init(&ss, read, fd, inbuf, sizeof inbuf);
	if (getln(&ss, sa, &match, '\n') == -1)
	{
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	return 1;
}

void
striptrailingwhitespace(sa)
	stralloc       *sa;
{
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
			return;
		}
	}
}
