/*
 * $Log: qmail-newmrh.c,v $
 * Revision 1.12  2021-06-13 13:03:43+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
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
#include "auto_control.h"
#include "variables.h"
#include "env.h"
#include "cdbmss.h"

#define FATAL "qmail-newmrh: fatal: "

int             rename(const char *, const char *);

int
main()
{
	int             fd, fdtemp, match;
	char            inbuf[1024];
	substdio        ssin;
	struct cdbmss   cdbmss;
	stralloc        line = { 0 };

	umask(033);

	if (!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (chdir(controldir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", controldir, ": ");
	if ((fd = open_read("morercpthosts")) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", controldir, "/morercpthosts: ");
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);

	if ((fdtemp = open_trunc("morercpthosts.tmp")) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", controldir, "/morercpthosts.tmp: ");

	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", controldir, "/morercpthosts.tmp: ");
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') != 0)
			strerr_die4sys(111, FATAL, "unable to read ", controldir, "/morercpthosts: ");
		case_lowerb(line.s, line.len);
		while (line.len) {
			if (line.s[line.len - 1] == ' ' ||
					line.s[line.len - 1] == '\n' ||
					line.s[line.len - 1] == '\t') {
				--line.len;
				continue;
			}
			if (line.s[0] != '#' && 
					cdbmss_add(&cdbmss, (unsigned char *) line.s, line.len, (unsigned char *) "", 0) == -1)
				strerr_die4sys(111, FATAL, "unable to write to ", controldir, "/morercpthosts.tmp: ");
			break;
		}
		if (!match)
			break;
	}
	if (cdbmss_finish(&cdbmss) == -1 ||
			fsync(fdtemp) == -1 ||
			close(fdtemp) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", controldir, "/morercpthosts.tmp: ");

	if (rename("morercpthosts.tmp", "morercpthosts.cdb") == -1)
		strerr_die2sys(111, FATAL, "unable to move morercpthosts.tmp to morercpthosts.cdb: ");
	return(0);
}

void
getversion_qmail_newmrh_c()
{
	static char    *x = "$Id: qmail-newmrh.c,v 1.12 2021-06-13 13:03:43+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
