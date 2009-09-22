/*
 * $Log: control.c,v $
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

	if((fd = open_read(fn)) == -1)
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

int
control_readfile(sa, fn)
	stralloc       *sa;
	char           *fn;
{
	int             fd, match;
	buffer          ssin;

	if (!stralloc_copys(sa, ""))
		return -1;
	if((fd = open_read(fn)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	buffer_init(&ssin, read, fd, inbuf, sizeof inbuf);
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len)
		{
			close(fd);
			return 1;
		}
		striptrailingwhitespace(&line);
		if (!stralloc_0(&line))
			break;
		if (line.s[0] && line.s[0] != '#' && !stralloc_cat(sa, &line))
			break;
		if (!match)
		{
			close(fd);
			return 1;
		}
	}
	close(fd);
	return -1;
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
