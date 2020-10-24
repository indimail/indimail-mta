/*
 * $Log: svok.c,v $
 * Revision 1.3  2020-10-08 18:34:12+05:30  Cprogrammer
 * use /run, /var/run if system supports it
 *
 * Revision 1.2  2004-10-22 20:31:18+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:32:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "strerr.h"
#include "error.h"
#include "open.h"
#ifdef USE_RUNFS
#include "run_init.h"
#endif

#define FATAL "svok: fatal: "

int
main(int argc, char **argv)
{
	int             fd;

	if (!argv[1])
		strerr_die1x(100, "svok: usage: svok dir");
	if (chdir(argv[1]) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", argv[1], ": ");
#ifdef USE_RUNFS
	run_init(argv[1], FATAL);
#endif
	if ((fd = open_write("supervise/ok")) == -1) {
		if (errno == error_noent)
			_exit(100);
		if (errno == error_nodevice)
			_exit(100);
		strerr_die4sys(111, FATAL, "unable to open ", argv[1], "/supervise/ok: ");
	}

	_exit(0);
}

void
getversion_svok_c()
{
	static char    *x = "$Id: svok.c,v 1.3 2020-10-08 18:34:12+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
