/*
 * $Id: auto-pidt.c,v 1.5 2025-01-22 00:30:34+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include "substdio.h"
#include "scan.h"
#include "fmt.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

void
my_puts(const char *s)	/*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
{
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

int
main()
{
	char            strnum[FMT_ULONG];
	strnum[fmt_ulong(strnum, (unsigned long) sizeof(pid_t))] = 0;

	my_puts("#define PID_BYTES ");
	my_puts(strnum);
	my_puts("\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return (0);
}

/*
 * $Log: auto-pidt.c,v $
 * Revision 1.5  2025-01-22 00:30:34+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.4  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.3  2020-11-24 13:43:57+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 15:34:16+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.1  2004-09-19 18:54:16+05:30  Cprogrammer
 * Initial revision
 *
 */
