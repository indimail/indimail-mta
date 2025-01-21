/*
 * $Id: cdb-database.c,v 1.6 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $
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
#include <strerr.h>
#include <open.h>
#include <error.h>
#include <case.h>
#include <fmt.h>
#include "auto_control.h"
#include "variables.h"

#define FATAL "cdb-database: fatal: "

int             rename(const char *, const char *);

int
main(int argc, char **argv)
{
	int             i, fd, fdtemp, match, l_no;
	struct cdbmss   cdbmss;
	stralloc        key = { 0 }, data = { 0 }, line = { 0 }, wildchars = { 0 },
					fntmp = {0}, fncdb = {0};
	char            inbuf[1024], strnum[FMT_ULONG];
	substdio        ssin;

	if (argc != 2)
		strerr_die1x(100, "usage: cdb-database filename");
	controldir = (controldir = env_get("CONTROLDIR")) ? controldir : auto_control;
	if (chdir(controldir) == -1)
		strerr_die4sys(111, FATAL, "chdir: ", controldir, ": ");
	if ((fd = open_read(argv[1])) == -1)
		strerr_die3sys(111, FATAL, argv[1], ": ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (!stralloc_copys(&fntmp, argv[1]) || !stralloc_catb(&fntmp, ".tmp", 4) || !stralloc_0(&fntmp))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_copys(&fncdb, argv[1]) || !stralloc_catb(&fncdb, ".cdb", 4) || !stralloc_0(&fncdb))
		strerr_die2sys(111, FATAL, "out of memory");
	if ((fdtemp = open_trunc(fntmp.s)) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": ");
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	if (!stralloc_copys(&wildchars, ""))
		strerr_die2sys(111, FATAL, "out of memory");
	for (l_no = 1;;l_no++) {
		if (getln(&ssin, &line, &match, '\n') != 0)
			strerr_die4sys(111, FATAL, "read: ", argv[1], ": ");
		if (line.len) {
			if (line.s[0] == '#')
				continue;
			if (line.s[0] == '.')
				break;
		}
		if (!match || byte_chr(line.s, line.len, '\0') < line.len) { /*- NULL char not allowed */
			strnum[fmt_ulong(strnum, l_no)] = 0;
			strerr_die5x(100, FATAL, "bad format in ", argv[1], " line ", strnum);
		}
		i = byte_chr(line.s, line.len, ':');
		if (i == line.len || !i || i < 2) { /*- =.*:, +.*: */
			strnum[fmt_ulong(strnum, l_no)] = 0;
			strerr_die5x(100, FATAL, "bad format in ", argv[1], " line ", strnum);
		}
		if (!stralloc_copys(&key, "!") ||
				!stralloc_catb(&key, line.s + 1, i - 1))
			strerr_die2sys(111, FATAL, "out of memory");
		case_lowerb(key.s, key.len);
		if (line.s[0] == '+') {
			if (i >= 2) {
				if (byte_chr(wildchars.s, wildchars.len, line.s[i - 1]) == wildchars.len) {
					if (!stralloc_append(&wildchars, line.s + i - 1))
						strerr_die2sys(111, FATAL, "out of memory");
				}
			}
		} else
		if (!stralloc_0(&key))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_copyb(&data, line.s + i + 1, line.len - i - 1))
			strerr_die2sys(111, FATAL, "out of memory");
		if (data.s[data.len - 2] != ':') {
			strnum[fmt_ulong(strnum, l_no)] = 0;
			strerr_die5x(100, FATAL, "bad format in ", argv[1], " line ", strnum);
		}
		data.s[data.len - 2] = 0;
		data.len -= 2;
		if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) data.s, data.len) == -1)
			strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	}
	close(fd);
	if (cdbmss_add(&cdbmss, (unsigned char *) "", 0, (unsigned char *) wildchars.s, wildchars.len) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	if (cdbmss_finish(&cdbmss) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	if (fsync(fdtemp) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	if (close(fdtemp) == -1)
		strerr_die3sys(111, FATAL, fntmp.s, ": write: ");
	if (rename(fntmp.s, fncdb.s) == -1)
		strerr_die6sys(111, FATAL, "rename: ", fntmp.s, " ==> ", fncdb.s, ": ");
	return(0);
}

void
getversion_cdb_database_c()
{
	const char     *x = "$Id: cdb-database.c,v 1.6 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: cdb-database.c,v $
 * Revision 1.6  2025-01-22 00:30:37+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-09-11 09:04:29+05:30  Cprogrammer
 * allow comments in input file
 *
 * Revision 1.3  2023-02-12 13:29:01+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.2  2022-10-31 09:07:47+05:30  Cprogrammer
 * look at last colon when parsing data
 *
 * Revision 1.1  2021-06-15 11:30:39+05:30  Cprogrammer
 * Initial revision
 *
 */
