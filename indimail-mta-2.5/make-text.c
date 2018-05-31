/*
 * $Log: make-text.c,v $
 * Revision 1.1  2004-07-17 20:56:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "getln.h"
#include "str.h"
#include "exit.h"

stralloc        line = { 0 };
int             match;

void
outs(s)
	char           *s;
{
	if (substdio_puts(subfdoutsmall, s) == -1)
	{
		_exit(-1);
	}
}

void
outqs(s)
	char           *s;
{
	unsigned int    u;
	while (*s)
	{
		u = str_chr(s, '"');
		if (s[u])
		{
			s[u] = '\0';
			outs(s);
			outs("\\\"");
			s += u + 1;
		} else
		{
			outs(s);
			s += u;
		}
	}
}

int
main()
{
	unsigned int    u;
	char           *tail;

	for (;;)
	{
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			_exit(-1);
		if (!match && !line.len)
			_exit(0);
		if (match)
			line.s[line.len - 1] = '\0';
		else
		if (!stralloc_0(&line))
			_exit(-1);
		tail = line.s;
		for (;;)
		{
			u = str_chr(tail, '$');
			if (!tail[u])
				break;
			tail[u] = '\0';
			outs("outs(\"");
			outqs(tail);
			outs("\");");
			tail += u + 1;
			if (!*tail)
			{
				_exit(-1);
			}
			switch (*tail)
			{
			case '$':
				outs("outs(\"$\")");
				break;
			case 'L':
				outs("if (*outlocal) { outs(outlocal); outs(\"-\"); }");
				break;
			case 'H':
				outs("outs(outhost);");
				break;
			default:
				_exit(-1);
			}
			++tail;
		}
    	outs("outs(\""); outqs(tail);
		outs("\\n\");\n");
	}
	if (substdio_flush(subfdoutsmall) == -1)
		_exit(-1);
	return(0);
}
