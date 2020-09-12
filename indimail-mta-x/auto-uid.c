/*
 * $Log: auto-uid.c,v $
 * Revision 1.9  2020-09-13 01:21:46+05:30  Cprogrammer
 * removed scan.h as dependency
 *
 * Revision 1.8  2010-05-16 16:11:44+05:30  Cprogrammer
 * added rcs id
 *
 * Revision 1.7  2010-05-16 16:03:57+05:30  Cprogrammer
 * use user "daemon" as a substitute for "mail" on OS X
 *
 * Revision 1.6  2009-02-10 09:27:44+05:30  Cprogrammer
 * allow auto-uid to run as non-root
 *
 * Revision 1.5  2004-10-22 15:34:23+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.4  2004-07-17 21:16:02+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "subfd.h"
#include "substdio.h"
#include "exit.h"
#include "fmt.h"

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF(write, 1, buf1, sizeof(buf1));

void
outs(s)	/*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
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
	struct passwd  *pw;
	char            strnum[FMT_ULONG];

	name = argv[1];
	if (!name)
		_exit(100);
	value = argv[2];
	if (!value)
		_exit(100);
	if (!(pw = getpwnam(value)))
	{
		if (!(pw = getpwuid(getuid())))
#ifndef DARWIN
			pw = getpwnam("mail");
#else
			pw = getpwnam("daemon");
#endif
	}
	if (!pw)
	{
		substdio_puts(subfderr, "fatal: unable to find user ");
		substdio_puts(subfderr, value);
		substdio_puts(subfderr, "\n");
		substdio_flush(subfderr);
		_exit(111);
	}
	strnum[fmt_ulong(strnum, (unsigned long) pw->pw_uid)] = 0;
	outs("int ");
	outs(name);
	outs(" = ");
	outs(strnum);
	outs(";\n");
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return(0);
}

void
getversion_auto_uid_c()
{
	static char    *x = "$Id: auto-uid.c,v 1.9 2020-09-13 01:21:46+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
