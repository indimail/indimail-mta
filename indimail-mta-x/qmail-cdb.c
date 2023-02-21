/*
 * $Log: qmail-cdb.c,v $
 * Revision 1.14  2022-10-31 23:52:38+05:30  Cprogrammer
 * ignore filename with -r option
 *
 * Revision 1.13  2022-10-31 19:14:29+05:30  Cprogrammer
 * add feature to create recipient.cdb for qmail-smtpd
 *
 * Revision 1.12  2021-06-15 11:46:25+05:30  Cprogrammer
 * moved cdbmss.h to libqmail
 *
 * Revision 1.11  2021-06-14 09:19:24+05:30  Cprogrammer
 * do chdir(controldir) instead of chdir(auto_qmail)
 *
 * Revision 1.10  2021-05-26 10:43:46+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.9  2020-11-24 13:46:34+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2017-10-05 08:46:33+05:30  Cprogrammer
 * fixed wrong control filename display in error message
 *
 * Revision 1.7  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.6  2005-08-23 17:35:32+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.5  2004-10-22 20:29:38+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:38:07+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.3  2003-12-24 17:27:51+05:30  Cprogrammer
 * configurable filename on command line
 *
 * Revision 1.2  2003-12-21 15:32:39+05:30  Cprogrammer
 * changed global variables to automatic
 *
 * Revision 1.1  2003-12-20 02:25:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sgetopt.h>
#include <strerr.h>
#include <stralloc.h>
#include <str.h>
#include <substdio.h>
#include <getln.h>
#include <case.h>
#include <open.h>
#include <error.h>
#include <env.h>
#include <cdbmss.h>
#include "auto_control.h"
#include "auto_assign.h"
#include "variables.h"

#define FATAL "qmail-cdb: fatal: "

int             rename(const char *, const char *);

int
main(int argc, char **argv)
{
	char            inbuf[1024];
	char           *arg, *ptr, *workdir;
	int             fd, fdtemp, match, i, recipient_cdb = 0,
					do_move = 0, dlen;
	struct cdbmss   cdbmss;
	substdio        ssin;
	stralloc        line = {0}, fn = {0}, key = {0};

	umask(033);
	while ((i = getopt(argc, argv, "rm")) != opteof) {
		switch (i)
		{
		case 'r':
			recipient_cdb = 1;
			break;
		case 'm':
			do_move = 1;
			break;
		default:
			strerr_die1x(100, "qmail-cdb [-r] [-m] filename");
		}
	}
	if (!recipient_cdb) {
		if (optind + 1 != argc)
			strerr_die1x(100, "qmail-cdb [-r] [-m] filename");
		arg = argv[optind++];
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
		workdir = controldir;
	} else {
		arg = "recipients";
		workdir = auto_assign;
	}
	if (chdir(workdir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", workdir, ": ");
	/* fn = argv.cdb\0argv.bak */
	if (!stralloc_copys(&fn, arg) ||
			!stralloc_catb(&fn, ".cdb", 4) ||
			!stralloc_0(&fn))
		strerr_die2x(111, FATAL, "out of memory");
	i = fn.len;
	if (!stralloc_cats(&fn, arg) ||
			!stralloc_catb(&fn, ".bak", 4) ||
			!stralloc_0(&fn))
		strerr_die2x(111, FATAL, "out of memory");
	if ((fd = open_read(arg)) == -1)
		strerr_die6sys(111, FATAL, "unable to read ", workdir, "/", arg, ": ");
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);

	str_copyb(fn.s - 4, "tmp", 3);
	if ((fdtemp = open_trunc(fn.s + i)) == -1)
		strerr_die6sys(111, FATAL, "unable to write to ", workdir, "/", fn.s + i, ": ");
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		strerr_die6sys(111, FATAL, "unable to write to ", workdir, "/", fn.s + i, ": ");
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') != 0)
			strerr_die6sys(111, FATAL, "unable to read ", workdir, "/", arg, ": ");
		while (line.len) {
			if (line.s[line.len - 1] == ' ' ||
					line.s[line.len - 1] == '\n' ||
					line.s[line.len - 1] == '\t')  {
				--line.len;
				continue;
			}
			if (line.s[0] != '#') {
				if (recipient_cdb) {
					if (!stralloc_copys(&key, ":"))
						strerr_die2x(111, FATAL, "out of memory");
					if (!stralloc_cat(&key, &line))
						strerr_die2x(111, FATAL, "out of memory");
					case_lowerb(key.s, key.len);
					ptr = key.s;
					dlen = key.len;
				} else {
					case_lowerb(line.s, line.len);
					ptr = line.s;
					dlen = line.len;
				}
				if (cdbmss_add(&cdbmss, (unsigned char *) ptr, dlen, (unsigned char *) "", 0) == -1)
					strerr_die6sys(111, FATAL, "unable to write to ", workdir, "/", fn.s + i, ": ");
			}
			break;
		}
		if (!match)
			break;
	}
	if (cdbmss_finish(&cdbmss) == -1 ||
			fsync(fdtemp) == -1 ||
			close(fdtemp) == -1)
		strerr_die6sys(111, FATAL, "unable to write to ", workdir, "/", fn.s + i, ": ");

	/*- rename fn.tmp to fn.cdb */
	if (rename(fn.s + i, fn.s) == -1)
		strerr_die6sys(111, FATAL, "unable to move ", fn.s + i, " to ", fn.s, ": ");
	if (do_move) {
		str_copyb(fn.s - 4, "bak", 3);
		/*- rename fn to fn.bak */
		if (rename(arg, fn.s + i) == -1)
			strerr_die6sys(111, FATAL, "unable to move ", arg, " to ", fn.s + i, ": ");
	}
	return(0);
}

void
getversion_qmail_cdb_c()
{
	static char    *x = "$Id: qmail-cdb.c,v 1.14 2022-10-31 23:52:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
