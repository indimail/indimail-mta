/*
 * $Log: auto-gid.c,v $
 * Revision 1.8  2020-11-24 13:43:49+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2018-05-29 09:36:03+05:30  Cprogrammer
 * use effective gid when group or default group "mail" does not exist
 *
 * Revision 1.6  2009-02-10 09:28:55+05:30  Cprogrammer
 * allow auto-gid to run as non-root
 *
 * Revision 1.5  2004-10-22 15:33:55+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.4  2004-07-17 21:15:23+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include "subfd.h"
#include "substdio.h"
#include "scan.h"
#include "fmt.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF(write, 1, buf1, sizeof(buf1));

void
outs(s)
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
	struct group   *gr;
	char            strnum[FMT_ULONG];

	name = argv[1];
	if (!name)
		_exit(100);
	value = argv[2];
	if (!value)
		_exit(100);
	if (!(gr = getgrnam(value))) {
		if (!(gr = getgrgid(getegid())))
			gr = getgrnam("mail");
	}
	if (!gr) {
		substdio_puts(subfderr, "fatal: unable to find group ");
		substdio_puts(subfderr, value);
		substdio_puts(subfderr, "\n");
		substdio_flush(subfderr);
		_exit(111);
	}
	strnum[fmt_ulong(strnum, (unsigned long) gr->gr_gid)] = 0;
	outs("int ");
	outs(name);
	outs(" = ");
	outs(strnum);
	outs(";\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	_exit(0);
	/*- Not reached */
	return(0);
}
