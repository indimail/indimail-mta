/*
 * $Log: tokenize.c,v $
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.2  2004-10-22 20:31:49+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:20:01+05:30  Cprogrammer
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

#define FATAL "tokenize: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
myput(char *buf, int len)
{
	char            ch;

	while (len)
	{
		ch = *buf;
		if (ch == '\n')
			ch = 0;
		substdio_put(subfdout, &ch, 1);
		++buf;
		--len;
	}
}


int
main()
{
	int             i, j, match;
	stralloc        line = { 0 }, tokens = { 0 };

	for (;;) {
		if (getln(subfdin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!line.len)
			break;
		if (match)
			--line.len;

		for (i = 0; i < line.len; ++i)
			if (line.s[i] == 0)
				line.s[i] = '\n';
		if (!stralloc_0(&line))
			nomem();

		if (!mess822_token(&tokens, line.s))
			nomem();

		for (j = i = 0; j < tokens.len; ++j) {
			if (!tokens.s[j]) {
				if (tokens.s[i] == ' ')
					substdio_puts(subfdout, "space\n");
				else
				if (tokens.s[i] == '\t')
					substdio_puts(subfdout, "tab\n");
				else
				if (tokens.s[i] == '=') {
					substdio_puts(subfdout, "string {");
					myput(tokens.s + i + 1, j - i - 1);
					substdio_puts(subfdout, "}\n");
				} else
				if (tokens.s[i] == '(') {
					substdio_puts(subfdout, "comment {");
					myput(tokens.s + i + 1, j - i - 1);
					substdio_puts(subfdout, "}\n");
				} else {
					substdio_puts(subfdout, "special ");
					myput(tokens.s + i, j - i);
					substdio_puts(subfdout, "\n");
				}
				i = j + 1;
			}
		}
		substdio_puts(subfdout, "\n");
		if (!match)
			break;
	}
	substdio_flush(subfdout);
	_exit(0);
}

void
getversion_tokenize_c()
{
	const char     *x = "$Id: tokenize.c,v 1.3 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
