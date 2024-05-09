/*-
 * $Id: condtomaildir.c,v 1.9 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <sys/stat.h>
#include <strerr.h>
#include <error.h>
#include <env.h>
#include <wait.h>
#include <env.h>
#include <seek.h>
#include <stralloc.h>
#include <sig.h>
#include "maildir_deliver.h"

#define FATAL "condtomaildir: fatal: "

static stralloc rpline = { 0 };
static stralloc dtline = { 0 };

int
main(int argc, char **argv)
{
	int             pid, wstat, i, reverse = 0;
	char           *ptr;

	if (!argv[1] || !argv[2])
		strerr_die1x(100, "condtomaildir: usage: condtomaildir dir program [ arg ... ]");
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] != 'r')
				strerr_die3x(100, "condtomaildir: Invalid option ", argv[i], "\nusage: condtomaildir [ -r ] dir program [ arg ... ]");
			if (argv[i][2] == 0)
				reverse = 1;
		}
	}
	if (!argv[1] || !argv[2] || (reverse && (!argv[2] || !argv[3])))
		strerr_die1x(100, "condtomaildir: usage: condtomaildir [ -r ] dir program [ arg ... ]");

	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		execvp(reverse ? argv[3] : argv[2], reverse ? argv + 3: argv + 2);
		if (error_temp(errno))
			_exit(111);
		else
		if (errno == error_noent)
			_exit(2);
		_exit(100);
	}
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		if (reverse)
			_exit(0);
		break;
	case 100:
		strerr_die2x(100,FATAL,"permanent child error");
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		if (!reverse)
			_exit(0);
	}
	if (seek_begin(0) == -1)
		strerr_die2sys(111, FATAL, "unable to rewind: ");
	sig_pipeignore();
	umask(077);
	if ((ptr = env_get("RPLINE"))) {
		if (!stralloc_copys(&rpline, ptr) || !stralloc_0(&rpline))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
	}
	if ((ptr = env_get("DTLINE"))) {
		if (!stralloc_copys(&dtline, ptr) || !stralloc_0(&dtline))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
	}
	i = maildir_deliver(reverse ? argv[2] : argv[1], &rpline, &dtline, env_get("QQEH"));
	switch (i)
	{
	case 0:
		break;
	case 2:
		strerr_die1x(111, "Unable to chdir to maildir. (#4.2.1)");
	case 3:
		strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
	case 4:
		strerr_die1x(111, "Unable to read message. (#4.3.0)");
	case 5:
		strerr_die1x(100, "Recipient's mailbox is full");
	default:
		strerr_die1x(111, "Temporary error on maildir delivery. (#4.3.0)");
	}
	strerr_die1x(99, "condtomaildir");
	/*- Not reached */
	return (0);
}

void
getversion_condtomaildir_c()
{
	const char     *x = "$Id: condtomaildir.c,v 1.9 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: condtomaildir.c,v $
 * Revision 1.9  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.8  2023-07-11 12:57:16+05:30  Cprogrammer
 * added -r reverse option to write to maildir when program fails instead of when program succeeds.
 *
 * Revision 1.7  2022-04-04 14:20:03+05:30  Cprogrammer
 * use USE_FSYNC, USE_FDATASYNC, USE_SYNCDIR to set sync to disk feature
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2020-11-24 13:44:40+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.4  2014-05-22 14:44:34+05:30  Cprogrammer
 * removed duplicate sig.h
 *
 * Revision 1.3  2004-10-22 20:24:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:34:44+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-07-17 20:51:06+05:30  Cprogrammer
 * Initial revision
 *
 */
