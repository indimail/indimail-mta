/*
 * $Log: generic.c,v $
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2021-06-15 11:36:51+05:30  Cprogrammer
 * moved makeargs.h to libqmail
 *
 * Revision 1.4  2020-04-01 16:13:40+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.3  2009-04-30 16:14:55+05:30  Cprogrammer
 * removed hasindimail.h
 *
 * Revision 1.2  2007-12-20 12:44:08+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.1  2005-06-15 22:11:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <str.h>
#include <env.h>
#include <wait.h>
#include <makeargs.h>
#include <noreturn.h>

extern char    *auto_scancmd[];

no_return int
virusscan(char *messfn)
{
	int             wstat, child;
	unsigned long   u;
	char          **argv;
	char           *scancmd[3] = { 0, 0, 0 };
	char           *ptr;

	switch (child = fork())
	{
	case -1:
		_exit(121);
	case 0:
		if ((ptr = env_get("SCANCMD"))) {
			if (!(argv = makeargs(ptr)))
				_exit(51);
		} else
			argv = auto_scancmd;
		if (!argv[1]) {
			scancmd[0] = argv[0];
			scancmd[1] = messfn;
			argv = scancmd;
		} else
		for (u = 1; argv[u]; u++) {
			if (!str_diffn(argv[u], "%s", 2))
				argv[u] = messfn;
		}
		if (*argv[0] != '/' && *argv[0] != '.')
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		_exit(75);
	}
	if (wait_pid(&wstat, child) == -1)
		_exit(122);
	if (wait_crashed(wstat))
		_exit(123);
	_exit(wait_exitcode(wstat));
}

#ifndef lint
void
getversion_generic_c()
{
	static char    *x = "$Id: generic.c,v 1.6 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmakeargsh;
	x++;
}
#endif
