/*
 * $Log: qmail-clean.c,v $
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
#include "auto_qmail.h"
#include "auto_split.h"
#include "getEnvConfig.h"

#define OSSIFIED 129600	/*- see qmail-send.c */

stralloc        line = { 0 };
int             conf_split;

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
		if (str_equal(d->d_name, "."))
			continue;
		if (str_equal(d->d_name, ".."))
			continue;
		if (!stralloc_copys(&line, "pid/"))
			continue;
		if (!stralloc_cats(&line, d->d_name))
			continue;
		if (!stralloc_0(&line))
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
	int             i;
	int             match;
	int             cleanuploop;
	unsigned long   id;

	if (chdir(auto_qmail) == -1)
		_exit(111);
	if(!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue";
	if (chdir(queuedir) == -1)
		_exit(111);
	sig_pipeignore();

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
			U("intd/", 1)
			U("mess/", 1)
			respond("+");
		} else
		if (byte_equal(line.s, 4, "todo/")) {
			U("intd/", 1)
			U("todo/", 1)
			respond("+");
		} else
			respond("x");
	}
	return(0);
}

void
getversion_qmail_clean_c()
{
	static char    *x = "$Id: qmail-clean.c,v 1.11 2021-05-16 00:40:21+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
