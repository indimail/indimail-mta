/*
 * $Log: auto-int.c,v $
 * Revision 1.8  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.7  2020-11-24 13:43:54+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 15:34:12+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.5  2004-07-17 21:15:56+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "scan.h"
#include "fmt.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

void
my_puts(const char *s)
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

int
main(int argc, char **argv)
{
	char           *name;
	char           *value;
	unsigned long   num;
	char            strnum[FMT_ULONG];

	name = argv[1];
	if (!name)
		_exit(100);
	value = argv[2];
	if (!value)
		_exit(100);
	scan_ulong(value, &num);
	strnum[fmt_ulong(strnum, num)] = 0;
	my_puts("int ");
	my_puts(name);
	my_puts(" = ");
	my_puts(strnum);
	my_puts(";\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return(0);
}
