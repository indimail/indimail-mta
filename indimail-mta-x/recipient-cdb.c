/*
 * $Log: recipient-cdb.c,v $
 * Revision 1.9  2021-06-13 16:38:30+05:30  Cprogrammer
 * added missing error check for stralloc
 *
 * Revision 1.8  2016-05-18 15:34:23+05:30  Cprogrammer
 * use directory defined by auto_assign for writing recipients.cdb
 *
 * Revision 1.7  2010-03-09 14:41:27+05:30  Cprogrammer
 * renamed qmail-recipients to recipient-cdb
 *
 * Revision 1.6  2009-04-29 11:21:22+05:30  Cprogrammer
 * changed coding style
 *
 * Revision 1.5  2009-04-10 13:32:46+05:30  Cprogrammer
 * convert key to lowercase
 *
 * Revision 1.4  2005-08-23 17:35:19+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 20:29:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:37:51+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-09-22 23:27:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/stat.h>
#include "strerr.h"
#include "stralloc.h"
#include "substdio.h"
#include "getln.h"
#include "open.h"
#include "case.h"
#include "auto_assign.h"
#include "cdbmss.h"

#define FATAL "recipient-cdb: fatal: "

int             rename(const char *, const char *);

void
die_read()
{
	strerr_die2sys(111, FATAL, "unable to read recipients: ");
}

void
die_write()
{
	strerr_die2sys(111, FATAL, "unable to write to recipients.tmp: ");
}

char            inbuf[1024];
substdio        ssin;

int             fd;
int             fdtemp;

struct cdbmss   cdbmss;
stralloc        line = { 0 };
stralloc        key = { 0 };
int             match;

int
main()
{
	umask(033);
	if (chdir(auto_assign) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_assign, ": ");
	if ((fd = open_read("recipients")) == -1)
		die_read();
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);
	if ((fdtemp = open_trunc("recipients.tmp")) == -1)
		die_write();
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_write();
	for (;;) {
		if (!stralloc_copys(&key, ":"))
			strerr_die2x(111, FATAL, "out of memory");
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_read();
		while (line.len) {
			if (line.s[line.len - 1] == ' ' ||
					line.s[line.len - 1] == '\n' ||
					line.s[line.len - 1] == '\t') {
				--line.len;
				continue;
			}
			if (line.s[0] != '#') {
				if (!stralloc_cat(&key, &line))
					strerr_die2x(111, FATAL, "out of memory");
				case_lowerb(key.s, key.len);
				if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) "", 0) == -1)
					die_write();
			}
			break;
		}
		if (!match)
			break;
	}
	if (cdbmss_finish(&cdbmss) == -1 || fsync(fdtemp) == -1 ||
			close(fdtemp) == -1)
		die_write();			/*- NFS stupidity */
	if (rename("recipients.tmp", "recipients.cdb") == -1)
		strerr_die2sys(111, FATAL, "unable to move recipients.tmp to recipients.cdb");
	_exit(0);
	/*- Not Reached */
	return (0);
}

void
getversion_qmail_recipients_c()
{
	static char    *x = "$Id: recipient-cdb.c,v 1.9 2021-06-13 16:38:30+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
