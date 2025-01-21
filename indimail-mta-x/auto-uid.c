/*
 * $Id: auto-uid.c,v 1.13 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <subfd.h>
#include <substdio.h>
#include <qprintf.h>

char            buf1[256];
substdio        ss1 = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, buf1, sizeof(buf1));

int
main(int argc, char **argv)
{
	char           *name;
	char           *value;
	struct passwd  *pw;

	if (!(name = argv[1]))
		_exit(100);
	if (!(value = argv[2]))
		_exit(100);
	if (!(pw = getpwnam(value))) {
		if (!(pw = getpwuid(getuid())))
#ifndef DARWIN
			pw = getpwnam("mail");
#else
			pw = getpwnam("daemon");
#endif
	}
	if (!pw) {
		subprintf(subfderr, "fatal: unable to find user %s\n", value);
		substdio_flush(subfderr);
		_exit(111);
	}
	if (subprintf(&ss1, "int %s = %u;\n", name, pw->pw_uid) == -1)
		_exit(111);
	if (substdio_flush(&ss1) == -1)
		_exit(111);
	return(0);
}

void
getversion_auto_uid_c()
{
	const char     *x = "$Id: auto-uid.c,v 1.13 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
/*
 * $Log: auto-uid.c,v $
 * Revision 1.13  2025-01-22 00:30:37+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2023-07-13 02:41:56+05:30  Cprogrammer
 * replaced outs() with subprintf
 *
 * Revision 1.10  2020-11-24 13:44:05+05:30  Cprogrammer
 * removed exit.h
 *
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
