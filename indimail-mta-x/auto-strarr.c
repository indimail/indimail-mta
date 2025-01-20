/*
 * $Log: auto-strarr.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2020-11-24 13:43:59+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2004-09-19 18:53:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

void
my_puts(const char *s) /*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

void
outs_octal(const char *s)
{
	unsigned char   ch;
	char            octal[4];
	while ((ch = *s++))
	{
		my_puts("\\");
		octal[3] = 0;
		octal[2] = '0' + (ch & 7);
		ch >>= 3;
		octal[1] = '0' + (ch & 7);
		ch >>= 3;
		octal[0] = '0' + (ch & 7);
		my_puts(octal);
	}
}

int
main(int argc, char **argv)
{
	char           *name;
	char           *value;
	char          **args = argv;
	int             c = 2;

	name = *(++args);
	if (!name)
		_exit(100);
	value = *(++args);
	if (!value)
		_exit(100);

	my_puts("const char *");
	my_puts(name);
	my_puts("[] = {\n");

	do
	{
		my_puts("\"\\\n");
		my_puts(value);
		my_puts("\\\n\",\n");
		value = *(++args);
	}
	while (++c < argc);

	my_puts("0\n};\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return (0);
}
