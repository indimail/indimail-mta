/*
 * $Log: qmaildirmake.c,v $
 * Revision 1.9  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2020-11-24 13:46:47+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2009-06-25 12:39:47+05:30  Cprogrammer
 * renamed maildirmake to qmaildirmake
 *
 * Revision 1.5  2009-02-21 15:06:30+05:30  Cprogrammer
 * ignore errno EEXIST
 *
 * Revision 1.4  2004-10-22 20:26:25+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:19:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "strerr.h"

#define FATAL "maildirmake: fatal: "

int
main(int argc, char **argv)
{
	umask(077);
	if (!argv[1])
		strerr_die1x(100, "maildirmake: usage: maildirmake name");
	if (mkdir(argv[1], 0700) == -1 && errno != EEXIST)
		strerr_die4sys(111, FATAL, "unable to mkdir ", argv[1], ": ");
	if (chdir(argv[1]) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", argv[1], ": ");

	if (mkdir("tmp", 0700) == -1 && errno != EEXIST)
		strerr_die4sys(111, FATAL, "unable to mkdir ", argv[1], "/tmp: ");
	if (chdir("tmp") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "tmp", ": ");
	if (chdir("..") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "..", ": ");

	if (mkdir("new", 0700) == -1 && errno != EEXIST)
		strerr_die4sys(111, FATAL, "unable to mkdir ", argv[1], "/new: ");
	if (chdir("new") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "new", ": ");
	if (chdir("..") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "..", ": ");

	if (mkdir("cur", 0700) == -1 && errno != EEXIST)
		strerr_die4sys(111, FATAL, "unable to mkdir ", argv[1], "/cur: ");
	if (chdir("cur") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "cur", ": ");
	if (chdir("..") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", "..", ": ");
	_exit(0);
}

void
getversion_maildirmake_c()
{
	const char     *x = "$Id: qmaildirmake.c,v 1.9 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
