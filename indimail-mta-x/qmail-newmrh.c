/*
 * $Log: qmail-newmrh.c,v $
 * Revision 1.11  2020-11-24 13:47:01+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.10  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.9  2005-08-23 17:34:58+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.8  2004-10-22 20:28:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-22 15:37:00+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.6  2003-12-20 01:47:52+05:30  Cprogrammer
 * use stralloc for preparing control file
 *
 * Revision 1.5  2003-10-23 01:24:09+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-10-01 19:05:09+05:30  Cprogrammer
 * changed return type to int
 *
 * Revision 1.3  2003-09-26 20:44:09+05:30  Cprogrammer
 * bug fix. morercpthosts was not correctly formated with '/'
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

#define FATAL "qmail-newmrh: fatal: "

int             rename(const char *, const char *);

void
die_read()
{
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	strerr_die4sys(111, FATAL, "unable to read ", controldir, "/morercpthosts: ");
}

void
die_write()
{
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	strerr_die4sys(111, FATAL, "unable to write to ", controldir, "/morercpthosts.tmp: ");
}

char            inbuf[1024];
substdio        ssin;

int             fd;
int             fdtemp;

struct cdbmss   cdbmss;
stralloc        line = { 0 };
int             match;

int
main()
{
	stralloc        controlfile1 = {0}, controlfile2 = {0};

	umask(033);
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");

	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&controlfile1, controldir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile1, "/morercpthosts"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile1))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if((fd = open_read(controlfile1.s)) == -1)
		die_read();
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);

	controlfile1.len--;
	if (!stralloc_cats(&controlfile1, ".tmp"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile1))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if((fdtemp = open_trunc(controlfile1.s)) == -1)
		die_write();

	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_write();
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_read();
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
					die_write();
			break;
		}
		if (!match)
			break;
	}
	if (cdbmss_finish(&cdbmss) == -1)
		die_write();
	if (fsync(fdtemp) == -1)
		die_write();
	if (close(fdtemp) == -1)
		die_write();/*- NFS stupidity */

	if (!stralloc_copys(&controlfile2, controldir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&controlfile2, "/morercpthosts.cdb"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&controlfile2))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (rename(controlfile1.s, controlfile2.s) == -1)
		strerr_die6sys(111, FATAL, "unable to move ", controlfile1.s, " to ", controlfile2.s, ": ");
	return(0);
}

void
getversion_qmail_newmrh_c()
{
	static char    *x = "$Id: qmail-newmrh.c,v 1.11 2020-11-24 13:47:01+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
