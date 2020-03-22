/*
 * $Log: envdir.c,v $
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
#include "envdir.h"
#include "strerr.h"
#include "error.h"
#include "alloc.h"
#include "pathexec.h"

#define FATAL "envdir: fatal: "

void
die_usage(void)
{
	strerr_die1x(111, "envdir: usage: envdir dir child");
}

int
main(int argc, char **argv)
{
	char           *fn;
	char          **e;
	int             tmperrno;

	if (!*argv)
		die_usage();
	if (!*++argv)
		die_usage();
	fn = *argv;
	if (!*++argv)
		die_usage();
	envdir_set(fn);
	if ((e = pathexec(argv)))
	{
		tmperrno = errno;
		alloc_free((char *) e);
		errno = tmperrno;
	}
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_envdir_c()
{
	static char    *x = "$Id: envdir.c,v 1.3 2010-06-08 21:57:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
