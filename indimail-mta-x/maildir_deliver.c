/*
 * $Id: maildir_deliver.c,v 1.7 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stralloc.h>
#include <substdio.h>
#include <fmt.h>
#include <error.h>
#include <sig.h>
#include <str.h>
#include <open.h>
#include <alloc.h>
#include <strerr.h>
#ifdef USE_FSYNC
#include <fcntl.h>
#include "syncdir.h"
#endif

static stralloc hostname = { 0 };
static stralloc fntmptph = { 0 };
static stralloc fnnewtph = { 0 };

static void
tryunlinktmp()
{
	unlink(fntmptph.s);
}

static void
sigalrm(int i)
{
	tryunlinktmp();
	_exit (3);
}

/*-
 * Some operating systems quickly recycle PIDs, which can lead
 * to collisions between Maildir-style filenames, which must
 * be unique and non-repeatable within one second.
 *
 * This uses the format of the revised Maildir protocol,
 * available at:
 *
 * http://cr.yp.to/proto/maildir.html
 *
 * It uses four unique identifiers:
 * - inode number of the file written to Maildir/tmp
 * - device number of the file written to Maildir/tmp
 * - time in microseconds
 * - the PID of the writing process
 * Additionally it puts the size of the file as S=size
 *
 * A Maildir-style filename would look like the following:
 *
 * In Maildir/tmp:
 * time.MmicrosecondsPpid.host
 * In Maildir/new:
 * time.IinodeVdeviceMmicrosecondsPpid.host,S=size
 *
 * Additionally, this patch further comforms to the revised
 * Maildir protocol by looking through the hostname for
 * instances of '/' and ':', replacing them with "057" and
 * "072", respectively, when writing it to disk.
 *
 * Special thanks go to Matthias Andree for design and
 * sanity-checking.
 *
 *   --Toby Betts <tmb2@po.cwru.edu>
 */

int
maildir_deliver(const char *dir, stralloc *rpline, stralloc *dtline, const char *qqeh)
{
	char            strnum[FMT_ULONG], host_a[64];
	char            inbuf[1024], outbuf[1024];
	char           *s;
	struct timeval  tmval;
	struct stat     st;
	unsigned long   pid;
	int             loop, fd, i;
	substdio        ss, ssout;

	sig_alarmcatch(sigalrm);
	if (chdir(dir) == -1) {
		if (error_temp(errno))
			return (1);
		return (2);
	}
	pid = getpid();
	host_a[0] = 0;
	gethostname(host_a, sizeof(host_a));
	s = host_a;
	for (loop = 0; loop < str_len(host_a); ++loop) {
		if (host_a[loop] == '/') {
			if (!stralloc_cats(&hostname, "057"))
				strerr_die1x(111, "Out of memory. (#4.3.0)");
			continue;
		}
		if (host_a[loop] == ':') {
			if (!stralloc_cats(&hostname, "072"))
				strerr_die1x(111, "Out of memory. (#4.3.0)");
			continue;
		}
		if (!stralloc_append(&hostname, s + loop))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
	}
	if (!stralloc_copyb(&fntmptph, "tmp/", 4))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	for (loop = 0;; ++loop) {
		gettimeofday(&tmval, 0);
		strnum[fmt_ulong(strnum, tmval.tv_sec)] = 0; /*- seconds */
		fntmptph.len = 4;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_append(&fntmptph, "M"))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
		strnum[fmt_ulong(strnum, tmval.tv_usec)] = 0; /*- microseconds */
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, "P"))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
		strnum[fmt_ulong(strnum, pid)] = 0; /*- pid */
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_cat(&fntmptph, &hostname) /*- hostname */
				|| !stralloc_0(&fntmptph))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
		if ((fd = open_excl(fntmptph.s)) >= 0)
			break;
		if (errno == error_exist) { /*- really should never get to this point */
			if (loop == 2)
				return (1);
			usleep(100);
		} else {
			if (errno == error_dquot)
				return (5);
			return (1);
		}
	} /*- for (loop = 0;; ++loop) */
	alarm(86400);
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	if (rpline->len && substdio_put(&ssout, rpline->s, rpline->len) == -1)
		goto fail;
	if (dtline->len && substdio_put(&ssout, dtline->s, dtline->len) == -1)
		goto fail;
	if (qqeh && substdio_puts(&ssout, qqeh) == -1)
		goto fail;
	switch (substdio_copy(&ssout, &ss))
	{
	case -2:
		tryunlinktmp();
		return (4);
	case -3:
		goto fail;
	}
	if (substdio_flush(&ssout) == -1)
		goto fail;
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fd) : fsync(fd)) == -1)
		goto fail;
#else
	if (fsync(fd) == -1)
		goto fail;
#endif
	if (fstat(fd, &st) == -1)
		goto fail;
	if (close(fd) == -1)
		goto fail;	/*- NFS dorks */
	if (!stralloc_copyb(&fnnewtph, "new/", 4))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	strnum[i = fmt_ulong(strnum, tmval.tv_sec)] = 0; /*- seconds */
	if (!stralloc_catb(&fnnewtph, strnum, i)
			|| !stralloc_append(&fnnewtph, "."))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	/*- in hexadecimal */
	if (!stralloc_append(&fnnewtph, "I"))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	s = alloc(fmt_xlong(0, st.st_ino) + fmt_xlong(0, st.st_dev)); /*- inode number */
	i = fmt_xlong(s, st.st_ino);
	if (!stralloc_catb(&fnnewtph, s, i)
			|| !stralloc_append(&fnnewtph, "V"))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	i = fmt_xlong(s, st.st_dev); /*- device number */
	if (!stralloc_catb(&fnnewtph, s, i))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	alloc_free(s);
	/*- in decimal */
	if (!stralloc_append(&fnnewtph, "M"))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	strnum[i = fmt_ulong(strnum, tmval.tv_usec)] = 0; /*- microseconds */
	if (!stralloc_catb(&fnnewtph, strnum, i)
			|| !stralloc_append(&fnnewtph, "P"))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	strnum[i = fmt_ulong(strnum, pid)] = 0; /*- pid */
	if (!stralloc_catb(&fnnewtph, strnum, i)
			|| !stralloc_append(&fnnewtph, ".")
			|| !stralloc_cat(&fnnewtph, &hostname)) /*- hostname */
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	strnum[i = fmt_ulong(strnum, st.st_size)] = 0; /*- size */
	if (!stralloc_catb(&fnnewtph, ",S=", 3)
			|| !stralloc_catb(&fnnewtph, strnum, i)
			|| !stralloc_0(&fnnewtph))
		strerr_die1x(111, "Out of memory. (#4.3.0)");
	if (link(fntmptph.s, fnnewtph.s) == -1)
		goto fail;
#if !defined(SYNCDIR_H) && defined(USE_FSYNC) && defined(LINUX)
	if (use_syncdir > 0) {
		if ((fd = open(fnnewtph.s, O_RDONLY)) == -1) {
			if (errno != error_noent)
				goto fail;
		} else
		if ((use_fdatasync > 0 ? fdatasync(fd) : fsync(fd)) == -1 || close(fd) == -1)
			goto fail;
	}
#endif
	/*
	 * if it was error_exist, almost certainly successful; i hate NFS
	 */
	tryunlinktmp();
	return (0);
fail:
	if (errno == error_dquot) {
		tryunlinktmp();
		return (5);
	} else {
		tryunlinktmp();
		return (1);
	}
}

void
getversion_maildir_deliver_c()
{
	const char     *x = "$Id: maildir_deliver.c,v 1.7 2025-01-22 00:30:37+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: maildir_deliver.c,v $
 * Revision 1.7  2025-01-22 00:30:37+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2022-09-18 23:01:36+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.4  2022-04-04 14:22:01+05:30  Cprogrammer
 * use USE_FSYNC, USE_FDATASYNC, USE_SYNCDIR to set sync to disk feature
 *
 * Revision 1.3  2022-03-31 00:08:07+05:30  Cprogrammer
 * replaced fsync() with fdatasync()
 *
 * Revision 1.2  2022-03-08 22:57:00+05:30  Cprogrammer
 * syncdir: do not treat error_noent as an error
 *
 * Revision 1.1  2021-05-16 22:53:41+05:30  Cprogrammer
 * Initial revision
 *
 */
