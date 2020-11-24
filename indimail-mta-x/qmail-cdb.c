/*
 * $Log: qmail-cdb.c,v $
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
#include "strerr.h"
#include "stralloc.h"
#include "substdio.h"
#include "getln.h"
#include "case.h"
#include "open.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "variables.h"
#include "env.h"
#include "cdbmss.h"

#define FATAL "qmail-cdb: fatal: "

int             rename(const char *, const char *);

void
die_read(char *s)
{
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	strerr_die6sys(111, FATAL, "unable to read ", controldir, "/", s, ": ");
}

void
die_write(char *s)
{
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	strerr_die6sys(111, FATAL, "unable to write to ", controldir, "/", s, ": ");
}

int
main(int argc, char **argv)
{
	char            inbuf[1024], sserrbuf[512];
	int             fd, fdtemp, match;
	struct cdbmss   cdbmss;
	substdio        ssin;
	substdio        sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
	stralloc        line = {0}, controlfile1 = {0}, controlfile2 = {0};

	if (argc != 2)
	{
		if (substdio_puts(&sserr, "USAGE: qmail-cdb filename\n") != -1)
			substdio_flush(&sserr);
		return(111);
	}
	umask(033);
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&controlfile1, controldir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile1, "/"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile1, argv[1]))
		strerr_die2sys(111, FATAL, "out of memory: ");

	if (!stralloc_copy(&controlfile2, &controlfile1))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile2, ".bak"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile1))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile2))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!access(controlfile1.s, F_OK))
	{
		if (rename(controlfile1.s, controlfile2.s))
			strerr_die6sys(111, FATAL, "unable to move ", controlfile1.s, " to ", controlfile2.s, ": ");
	} else
		return(0);
	if((fd = open_read(controlfile2.s)) == -1)
		die_read(controlfile2.s);
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);
	controlfile1.len--;
	if (!stralloc_cats(&controlfile1, ".tmp"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile1))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if((fdtemp = open_trunc(controlfile1.s)) == -1)
		die_write(controlfile1.s);
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_write(controlfile1.s);
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_read(controlfile2.s);
		case_lowerb(line.s, line.len);
		while (line.len)
		{
			if (line.s[line.len - 1] == ' ')
			{
				--line.len;
				continue;
			}
			if (line.s[line.len - 1] == '\n')
			{
				--line.len;
				continue;
			}
			if (line.s[line.len - 1] == '\t')
			{
				--line.len;
				continue;
			}
			if (line.s[0] != '#')
				if (cdbmss_add(&cdbmss, (unsigned char *) line.s, line.len, (unsigned char *) "", 0) == -1)
					die_write(controlfile1.s);
			break;
		}
		if (!match)
			break;
	}
	if (cdbmss_finish(&cdbmss) == -1)
		die_write(controlfile1.s);
	if (fsync(fdtemp) == -1)
		die_write(controlfile1.s);
	if (close(fdtemp) == -1)
		die_write(controlfile1.s); /*- NFS stupidity */

	if (!stralloc_copys(&controlfile2, controldir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile2, "/"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile2, argv[1]))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile2, ".cdb"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile2))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (rename(controlfile1.s, controlfile2.s) == -1)
		strerr_die6sys(111, FATAL, "unable to move ", controlfile1.s, " to ", controlfile2.s, ": ");
	return(0);
}

void
getversion_qmail_cdb_c()
{
	static char    *x = "$Id: qmail-cdb.c,v 1.9 2020-11-24 13:46:34+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
