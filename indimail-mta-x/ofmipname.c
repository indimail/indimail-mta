/*
 * $Log: ofmipname.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2021-06-15 11:42:52+05:30  Cprogrammer
 * moved cdbmss.h to libqmail
 *
 * Revision 1.4  2020-11-24 13:46:25+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.3  2005-08-23 17:33:28+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.2  2004-10-22 20:27:46+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cdbmss.h>
#include <byte.h>
#include <strerr.h>
#include <open.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <getln.h>
#include <noreturn.h>

#define FATAL "ofmipname: fatal: "

int             rename(char *, char *);

static char    *fncdb;
static char    *fntmp;
static stralloc line = { 0 };
static stralloc key = { 0 };
static stralloc data = { 0 };
static struct cdbmss   cdbmss;

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
die_read()
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

no_return void
die_write()
{
	strerr_die4sys(111, FATAL, "unable to create ", fntmp, ": ");
}

no_return void
usage()
{
	strerr_die1x(100, "ofmipname: usage: ofmipname cdb tmp");
}

void
doit(char *x, unsigned int len)
{
	unsigned int    colon;

	if (!len)
		return;
	if (x[0] == '#')
		return;

	colon = byte_chr(x, len, ':');
	if (colon == len)
		return;
	if (!stralloc_copyb(&key, x, colon))
		nomem();
	++colon;
	x += colon;
	len -= colon;

	colon = byte_chr(x, len, ':');
	if (colon == len)
		return;
	if (!stralloc_copyb(&data, x, colon))
		nomem();
	if (!stralloc_0(&data))
		nomem();
	++colon;
	x += colon;
	len -= colon;

	colon = byte_chr(x, len, ':');
	if (colon == len)
		return;
	if (!stralloc_catb(&data, x, colon))
		nomem();
	++colon;
	x += colon;
	len -= colon;

	if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) data.s, data.len) == -1)
		die_write();
}

int
main(int argc, char **argv)
{
	int             fd, match;

	umask(033);

	if (!(fncdb = argv[1]))
		usage();
	if (!(fntmp = argv[2]))
		usage();
	if ((fd = open_trunc(fntmp)) == -1)
		die_write();
	if (cdbmss_start(&cdbmss, fd) == -1)
		die_write();
	for (;;) {
		if (getln(subfdin, &line, &match, '\n') == -1)
			die_read();
		doit(line.s, line.len);
		if (!match)
			break;
	}

	if (cdbmss_finish(&cdbmss) == -1)
		die_write();
	if (fsync(fd) == -1)
		die_write();
	if (close(fd) == -1)
		die_write();		
	if (rename(fntmp, fncdb) == -1) /*- NFS stupidity */
		strerr_die5sys(111, FATAL, "unable to move ", fntmp, " to ", fncdb);
	_exit(0);
}

void
getversion_ofmipname_c()
{
	const char     *x = "$Id: ofmipname.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
