/*
 * $Log: auto-fnlen.c,v $
 * Revision 1.3  2020-11-24 13:43:46+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 15:33:46+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-09-19 18:54:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include "substdio.h"
#include "scan.h"
#include "fmt.h"
#include "timestamp.h"
#include "auto_pidt.h"

static unsigned long fnbytes = TIMESTAMP + PID_BYTES * 2 + sizeof(unsigned short) * 2 + 3;


char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

void
my_puts(char *s) /*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

int
main()
{
	char            buf[FMT_ULONG];
	buf[fmt_ulong(buf, fnbytes)] = '\0';

	my_puts("#define FN_BYTES ");
	my_puts(buf);
	my_puts("\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return (0);
}
