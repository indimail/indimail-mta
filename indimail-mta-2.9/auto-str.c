/*
 * $Log: auto-str.c,v $
 * Revision 1.6  2004-10-22 15:34:20+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.5  2004-07-17 21:15:59+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "exit.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF(write, 1, buf1, sizeof(buf1));

void
my_puts(s)
	char           *s;
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *name;
	char           *value;
	unsigned char   ch;
	char            octal[4];

	name = argv[1];
	if (!name)
		_exit(100);
	value = argv[2];
	if (!value)
		_exit(100);
	my_puts("char ");
	my_puts(name);
	my_puts("[] = \"\\\n");
	while ((ch = *value++))
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
	my_puts("\\\n\";\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	_exit(0);
	/*- Not reached */
	return(0);
}
