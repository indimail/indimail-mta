/*
 * $Log: auto-str.c,v $
 * Revision 1.3  2008-07-17 23:02:13+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.2  2004-05-12 22:44:34+05:30  Cprogrammer
 * changed puts() to my_puts for fedora release
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "buffer.h"
#include "exit.h"

char            bspace[256];
buffer          b = BUFFER_INIT(write, 1, bspace, sizeof bspace);

void
my_puts(char *s)
{
	if (buffer_puts(&b, s) == -1)
		_exit(111);
}

int
main(int argc, char **argv)
{
	char           *name;
	char           *value;
	unsigned char   ch;
	char            octal[4];

	if(!(name = argv[1]))
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
	if (buffer_flush(&b) == -1)
		_exit(111);
	_exit(0);
	/* Not reached */
	return(0);
}
