/*
 * $Log: envdir.c,v $
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
	char          **e;
	int             i, opt, warn_on_error = 0, ignore_unreadable = 0,
					unreadable_count = 0;

	orig_env = environ;
	while ((opt = getopt(argc, argv, "cwi")) != opteof) {
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
	if (i || (!ignore_unreadable && unreadable_count)) { /*- original envdir behaviour */
		pathexec_clear(); /*- clear environment variables picked by pathexec */
		environ = orig_env;
	}
	if ((e = pathexec(argv + optind + 1))) {
		strerr_warn4(FATAL, "unable to run ", argv[optind + 1], ": ", &strerr_sys);
		alloc_free((char *) e);
		_exit(111);
	}
	/*- Not reached */
	return(1);
}

void
getversion_envdir_main_c()
{
	const char     *x = "$Id: envdir.c,v 1.7 2021-08-30 12:04:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
