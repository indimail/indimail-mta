/*
 * $Id: qmail-newu.c,v 1.15 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stralloc.h>
#include <subfd.h>
#include <getln.h>
#include <substdio.h>
#include <cdbmss.h>
#include <byte.h>
#include <env.h>
#include <open.h>
#include <error.h>
#include <case.h>
#include <noreturn.h>
#include "auto_assign.h"

int             rename(const char *, const char *);

no_return void
die_temp()
{
	_exit(111);
}

no_return void
die_chdir(char *home)
{
	substdio_puts(subfderr, "qmail-newu: fatal: unable to chdir to ");
	substdio_puts(subfderr, home);
	substdio_putsflush(subfderr, "\n");
	die_temp();
}

no_return void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: out of memory\n");
	die_temp();
}

no_return void
die_opena()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to open users/assign\n");
	die_temp();
}

no_return void
die_reada()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to read users/assign\n");
	die_temp();
}

no_return void
die_format()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: bad format in users/assign\n");
	die_temp();
}

no_return void
die_opent()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to open users/cdb.tmp\n");
	die_temp();
}

no_return void
die_writet()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to write users/cdb.tmp\n");
	die_temp();
}

no_return void
die_rename()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to move users/cdb.tmp to users/cdb\n");
	die_temp();
}

int
main(int argc, char **argv)
{
	int             i, numcolons, fd, fdtemp, match;
	char           *assigndir;
	struct cdbmss   cdbmss;
	stralloc        key = { 0 }, data = { 0 }, line = { 0 }, wildchars = { 0 };
	char            inbuf[1024];
	substdio        ssin;

	assigndir = (assigndir = env_get("ASSIGNDIR")) ? assigndir : auto_assign;
	if (chdir(argc == 1 ? assigndir : argv[1]) == -1)
		die_chdir(argc == 1 ? assigndir : argv[1]);
	if ((fd = open_read("assign")) == -1)
		die_opena();
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if ((fdtemp = open_trunc("cdb.tmp")) == -1)
		die_opent();
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_writet();
	if (!stralloc_copys(&wildchars, ""))
		die_nomem();
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_reada();
		if (line.len && (line.s[0] == '.'))
			break;
		if (!match)
			die_format();
		if (byte_chr(line.s, line.len, '\0') < line.len)
			die_format();
		i = byte_chr(line.s, line.len, ':');
		if (i == line.len)
			die_format();
		if (i == 0)
			die_format();
		if (!stralloc_copys(&key, "!") ||
				!stralloc_catb(&key, line.s + 1, i - 1))
			die_nomem();
		case_lowerb(key.s, key.len);
		if (line.s[0] == '+') {
			if (i >= 2) {
				if (byte_chr(wildchars.s, wildchars.len, line.s[i - 1]) == wildchars.len) {
					if (!stralloc_append(&wildchars, line.s + i - 1))
						die_nomem();
				}
			}
		} else
		if (!stralloc_0(&key))
			die_nomem();
		if (!stralloc_copyb(&data, line.s + i + 1, line.len - i - 1))
			die_nomem();
		numcolons = 0;
		for (i = 0; i < data.len; ++i) {
			if (data.s[i] == ':') {
				data.s[i] = 0;
				if (++numcolons == 6)
					break;
			}
		}
		if (numcolons < 6)
			die_format();
		data.len = i;

		if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) data.s, data.len) == -1)
			die_writet();
	}
	close(fd);
	if (cdbmss_add(&cdbmss, (unsigned char *) "", 0, (unsigned char *) wildchars.s, wildchars.len) == -1)
		die_writet();
	if (cdbmss_finish(&cdbmss) == -1)
		die_writet();
	if (fsync(fdtemp) == -1)
		die_writet();
	if (close(fdtemp) == -1)
		die_writet();/*- NFS stupidity */
	if (rename("cdb.tmp", "cdb") == -1)
		die_rename();
	return(0);
}

void
getversion_qmail_newu_c()
{
	const char     *x = "$Id: qmail-newu.c,v 1.15 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: qmail-newu.c,v $
 * Revision 1.15  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2023-02-12 13:28:28+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.12  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.11  2021-06-15 11:57:21+05:30  Cprogrammer
 * moved cdbmss.h to libqmail
 *
 * Revision 1.10  2020-11-24 13:47:04+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.9  2016-05-18 15:20:59+05:30  Cprogrammer
 * use env variable ASSIGNDIR or auto_assign for users/cdb and user/assign file
 *
 * Revision 1.8  2008-06-20 16:00:34+05:30  Cprogrammer
 * added argument to process different directory for assign file
 *
 * Revision 1.7  2005-08-23 17:35:01+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.6  2004-10-22 20:28:34+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:37:04+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-07-17 21:20:57+05:30  Cprogrammer
 * added RCS log
 *
 */
