/*
 * $Log: make-text.c,v $
 * Revision 1.3  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.2  2020-11-24 13:46:08+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2004-07-17 20:56:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "getln.h"
#include "str.h"

stralloc        line = { 0 };
int             match;

void
outs(const char *s)
{
	if (substdio_puts(subfdoutsmall, s) == -1) {
		_exit(-1);
	}
}

void
outqs(char *s)
{
	unsigned int    u;

	while (*s) {
		u = str_chr(s, '"');
		if (s[u]) {
			s[u] = '\0';
			outs(s);
			outs("\\\"");
			s += u + 1;
		} else {
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
