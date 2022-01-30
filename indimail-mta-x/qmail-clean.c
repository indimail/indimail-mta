/*
 * $Log: qmail-clean.c,v $
 * Revision 1.14  2022-01-30 08:37:53+05:30  Cprogrammer
 * made bigtodo configurable
 *
 * Revision 1.13  2021-06-27 10:45:02+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 *
 * Revision 1.12  2021-06-14 01:03:48+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.11  2021-05-16 00:40:21+05:30  Cprogrammer
 * use configurable conf_split instead of auto_split variable
 *
 * Revision 1.10  2020-11-24 13:46:37+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.9  2010-02-10 08:58:19+05:30  Cprogrammer
 * removed dependency on indimail
 *
 * Revision 1.8  2007-12-20 12:46:57+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.7  2004-10-22 20:28:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:36:35+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.5  2004-07-17 21:20:36+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include "sig.h"
#include "now.h"
#include "str.h"
#include "direntry.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "byte.h"
#include "scan.h"
#include "fmt.h"
#include "error.h"
#include "fmtqfn.h"
#include "env.h"
#include "variables.h"
#include "auto_split.h"
#include "getEnvConfig.h"

#define OSSIFIED 129600	/*- see qmail-send.c */

stralloc        line = { 0 };

void
cleanuppid()
{
	DIR            *dir;
	direntry       *d;
	struct stat     st;
	datetime_sec    time;

	time = now();
	dir = opendir("pid");
	if (!dir)
		return;
	while ((d = readdir(dir))) {
		if (str_equal(d->d_name, ".") ||
				str_equal(d->d_name, ".."))
			continue;
		if (!stralloc_copys(&line, "pid/") ||
				!stralloc_cats(&line, d->d_name) ||
				!stralloc_0(&line))
			continue;
		if (stat(line.s, &st) == -1)
			continue;
		if (time < st.st_atime + OSSIFIED)
			continue;
		unlink(line.s);
	}
	closedir(dir);
}

char            fnbuf[FMTQFN];

void
respond(s)
	char           *s;
{
	if (substdio_putflush(subfdoutsmall, s, 1) == -1)
		_exit(100);
}

int
main()
{
	int             i, match, bigtodo;
	int             cleanuploop;
	unsigned long   id;

	if(!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue";
	if (chdir(queuedir) == -1)
		_exit(111);
	sig_pipeignore();

	getEnvConfigInt(&bigtodo, "BIGTODO", 0);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	if (!stralloc_ready(&line, 200))
		_exit(111);
	cleanuploop = 0;
	for (;;) {
		if (cleanuploop)
			--cleanuploop;
		else {
			cleanuppid();
			cleanuploop = 30;
		}
		if (getln(subfdinsmall, &line, &match, '\0') == -1)
			break;
		if (!match)
			break;
		if (line.len < 7) {
			respond("x");
			continue;
		}
		if (line.len > 100) {
			respond("x");
			continue;
		}
		if (line.s[line.len - 1]) {
			respond("x");
			continue;
		}	/*- impossible */
		for (i = line.len - 2; i > 4; --i) {
			if (line.s[i] == '/')
				break;
			if ((unsigned char) (line.s[i] - '0') > 9) {
				respond("x");
				continue;
			}
		}
		if (line.s[i] == '/') {
			if (!scan_ulong(line.s + i + 1, &id)) {
				respond("x");
				continue;
			}
		}
		if (byte_equal(line.s, 5, "foop/")) {
#define U(prefix,flag) fmtqfn(fnbuf,prefix,id,flag); \
if (unlink(fnbuf) == -1) if (errno != error_noent) { respond("!"); continue; }
			U("intd/", bigtodo)
			U("mess/", 1)
			respond("+");
		} else
		if (byte_equal(line.s, 4, "todo/")) {
			U("intd/", bigtodo)
			U("todo/", bigtodo)
			respond("+");
		} else
			respond("x");
	}
	return(0);
}

void
getversion_qmail_clean_c()
{
	static char    *x = "$Id: qmail-clean.c,v 1.14 2022-01-30 08:37:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
