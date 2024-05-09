/*
 * $Id: maildir.c,v 1.8 2023-12-23 09:13:28+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <env.h>
#include <stralloc.h>
#include <direntry.h>
#include <datetime.h>
#include <now.h>
#include <str.h>
#include <strerr.h>
#include "prioq.h"
#include "maildir.h"

struct strerr   maildir_chdir_err;
struct strerr   maildir_scan_err;

int
maildir_chdir()
{
	char           *maildir;

	if (!(maildir = env_get("MAILDIR")))
		STRERR(-1, maildir_chdir_err, "MAILDIR not set")
	if (chdir(maildir) == -1)
		STRERR_SYS3(-1, maildir_chdir_err, "unable to chdir to ", maildir, ": ")
	return 0;
}

void
maildir_clean(stralloc *tmpname)
{
	DIR            *dir;
	direntry       *d;
	datetime_sec    my_time;
	struct stat     st;

	my_time = now();
	if (!(dir = opendir("tmp")))
		return;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		if (!stralloc_copys(tmpname, "tmp/") ||
				!stralloc_cats(tmpname, d->d_name) ||
				!stralloc_0(tmpname))
			break;
		if (stat(tmpname->s, &st) == 0)
			if (my_time > st.st_atime + 129600)
				unlink(tmpname->s);
	}
	closedir(dir);
}

static int
append(prioq *pq, stralloc *filenames, const char *subdir, datetime_sec my_time)
{
	DIR            *dir;
	direntry       *d;
	struct prioq_elt pe;
	unsigned int    pos;
	struct stat     st;

	if (!(dir = opendir(subdir)))
		STRERR_SYS3(-1, maildir_scan_err, "unable to scan $MAILDIR/", subdir, ": ")
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		pos = filenames->len;
		if (!stralloc_cats(filenames, subdir) ||
				!stralloc_cats(filenames, "/") ||
				!stralloc_cats(filenames, d->d_name) ||
				!stralloc_0(filenames))
			break;
		if (stat(filenames->s + pos, &st) == 0) {
			if (!S_ISREG(st.st_mode)) { /* a directory in new, cur is enough to break maildirserial */
				strerr_warn3("warning: ", filenames->s + pos, ": not a regular file", 0);
				filenames->len = pos; /*- remove current entry */
				continue;
			}
			if (st.st_mtime < my_time) { /*- don't want to mix up the order */
				/*
				 * this also means that when maildirserial is called
				 * it will skip all files having mtime >= start time of
				 * maildirserial
				 */
				pe.dt = st.st_mtime;
				pe.id = pos;
				if (!prioq_insert(min, pq, &pe))
					break;
			}
		}
	}
	closedir(dir);
	if (d)
		STRERR_SYS3(-1, maildir_scan_err, "unable to read $MAILDIR/", subdir, ": ")
	return 0;
}

int
maildir_scan(prioq *pq, stralloc *filenames, int flagnew, int flagcur)
{
	struct prioq_elt pe;
	datetime_sec    my_time;

	if (!stralloc_copys(filenames, ""))
		return 0;
	while (prioq_get(pq, &pe))
		prioq_del(pq);
	my_time = now();
	if (flagnew && append(pq, filenames, "new", my_time) == -1)
		return -1;
	if (flagcur && append(pq, filenames, "cur", my_time) == -1)
		return -1;
	return 0;
}

void
getversion_maildir_c()
{
	const char     *x = "$Id: maildir.c,v 1.8 2023-12-23 09:13:28+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: maildir.c,v $
 * Revision 1.8  2023-12-23 09:13:28+05:30  Cprogrammer
 * skip non-regular to prevent breaking maildirserial
 *
 * Revision 1.7  2021-06-03 18:09:59+05:30  Cprogrammer
 * use new prioq functions
 *
 * Revision 1.6  2021-05-16 00:14:43+05:30  Cprogrammer
 * include strerr.h explicitly
 *
 * Revision 1.5  2004-10-22 20:26:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:19:26+05:30  Cprogrammer
 * added RCS log
 *
 */
