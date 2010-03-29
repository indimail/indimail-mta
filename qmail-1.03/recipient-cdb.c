/*
 * $Log: recipient-cdb.c,v $
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
#include "exit.h"
#include "open.h"
#include "case.h"
#include "auto_qmail.h"
#include "cdbmss.h"

#define FATAL "recipient-cdb: fatal: "

int             rename(const char *, const char *);

void
die_read()
{
	strerr_die2sys(111, FATAL, "unable to read users/recipients: ");
}

void
die_write()
{
	strerr_die2sys(111, FATAL, "unable to write to users/recipients.tmp: ");
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
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");
	if ((fd = open_read("users/recipients")) == -1)
		die_read();
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);
	if ((fdtemp = open_trunc("users/recipients.tmp")) == -1)
		die_write();
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_write();
	for (;;)
	{
		stralloc_copys(&key, ":");
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_read();
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
			if (line.s[0] != '#' && stralloc_cat(&key, &line))
			{
				case_lowerb(key.s, key.len);
				if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) "", 0) == -1)
					die_write();
			}
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
		die_write();			/*- NFS stupidity */
	if (rename("users/recipients.tmp", "users/recipients.cdb") == -1)
		strerr_die2sys(111, FATAL, "unable to move users/recipients.tmp to users/recipients.cdb");
	_exit(0);
	/*- Not Reached */
	return (0);
}

void
getversion_qmail_recipients_c()
{
	static char    *x = "$Id: recipient-cdb.c,v 1.7 2010-03-09 14:41:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
