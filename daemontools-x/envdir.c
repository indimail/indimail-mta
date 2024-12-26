/*
 * $Id: envdir.c,v 1.10 2024-12-27 01:01:38+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sgetopt.h>
#include <envdir.h>
#include <strerr.h>
#include <error.h>
#include <alloc.h>
#include <env.h>
#include <pathexec.h>
#include <noreturn.h>

#define FATAL "envdir: fatal: "
#define WARN  "envdir: warn: "

static char   **orig_env;

no_return void
die_usage(const char *str)
{
	if (str)
		strerr_die3x(100, FATAL, str, "\nusage: envdir [-cwi] dir child");
	else
		strerr_die2x(100, FATAL, "\nusage: envdir [-cwi] dir child");
}

int
main(int argc, char **argv)
{
	const char     *fn, *err = (char *) 0;
	int             i, opt, warn_on_error = 0, ignore_unreadable = 0,
					unreadable_count = 0, set_path = 0, j = 0;

	orig_env = environ;
	while ((opt = getopt(argc, argv, "cwip")) != opteof) {
		switch (opt)
		{
		case 'c':
			env_clear();
			break;
		case 'w':
			warn_on_error = 1;
			break;
		case 'i':
			ignore_unreadable = 1;
			break;
		case 'p':
			set_path = 1;
			break;
		default:
			die_usage(0);
			break;
		}
	}
	if (argc - optind < 2)
		die_usage("directory and program must be specfied");
	if (!argv[optind] || !*argv[optind])
		die_usage("directory not specified");
	if (!argv[optind + 1] || !*argv[optind + 1])
		die_usage("program to run not specified");
	fn = argv[optind];
	if ((i = envdir(fn, &err, ignore_unreadable, &unreadable_count))) {
		if (!warn_on_error)
			strerr_die5sys(111, FATAL, envdir_str(i), ": ", err, ": ");
		strerr_warn5(WARN, envdir_str(i), ": ", err, ": ", &strerr_sys);
	}
	if (set_path)
		j = pathexec_env_plus("PATH=", 5);
	if (i || j || (!ignore_unreadable && unreadable_count)) { /*- original envdir behaviour */
		pathexec_clear(); /*- clear environment variables picked by pathexec */
		environ = orig_env;
	}
	pathexec(argv + optind + 1);
	strerr_warn4(FATAL, "unable to run ", argv[optind + 1], ": ", &strerr_sys);
}

void
getversion_envdir_main_c()
{
	const char     *x = "$Id: envdir.c,v 1.10 2024-12-27 01:01:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: envdir.c,v $
 * Revision 1.10  2024-12-27 01:01:38+05:30  Cprogrammer
 * Ignore return value of pathexec()
 *
 * Revision 1.9  2024-11-12 16:03:28+05:30  Cprogrammer
 * added -p option to set PATH variable before the call to execve
 *
 * Revision 1.8  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.6  2021-07-14 19:28:02+05:30  Cprogrammer
 * added options i, w to ignore read errors and warn instead of exit on error
 *
 * Revision 1.5  2021-06-30 19:27:53+05:30  Cprogrammer
 * added -c option to clear existing env variables
 *
 * Revision 1.4  2021-05-13 14:51:45+05:30  Cprogrammer
 * use envdir() function from libqmail
 *
 * Revision 1.3  2010-06-08 21:57:51+05:30  Cprogrammer
 * moved code to set environment variables to envdir_set.c
 *
 * Revision 1.2  2004-10-22 20:24:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:40:22+05:30  Cprogrammer
 * Initial revision
 *
 */
