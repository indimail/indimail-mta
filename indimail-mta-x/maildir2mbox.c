/*
 * $Id: maildir2mbox.c,v 1.11 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <env.h>
#include <stralloc.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <error.h>
#include <open.h>
#include <lock.h>
#include <str.h>
#include <qtime.h>
#include <strerr.h>
#include <noreturn.h>
#include "maildir.h"
#include "prioq.h"
#include "gfrom.h"

#define FATAL   "maildir2mbox: fatal: "
#define WARNING "maildir2mbox: warn: "

int             rename(const char *, const char *);

static char    *mbox;
static char    *mboxtmp;
static char     inbuf[SUBSTDIO_INSIZE];
static char     outbuf[SUBSTDIO_OUTSIZE];
static stralloc filenames = { 0 };
static stralloc line = { 0 };
static stralloc ufline = { 0 };
static prioq    pq = { 0 };
static prioq    pq2 = { 0 };

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

int
main()
{
	substdio        ssin;
	substdio        ssout;
	struct prioq_elt pe;
	int             fdoldmbox;
	int             fdnewmbox;
	int             fd;
	int             match;
	int             fdlock;

	umask(077);
	if (!(mbox = env_get("MAIL")))
		strerr_die2x(111, FATAL, "MAIL not set");
	if (!(mboxtmp = env_get("MAILTMP")))
		strerr_die2x(111, FATAL, "MAILTMP not set");

	if (maildir_chdir() == -1)
		strerr_die1(111, FATAL, &maildir_chdir_err);
	maildir_clean(&filenames);
	if (maildir_scan(&pq, &filenames, 1, 1) == -1)
		strerr_die1(111, FATAL, &maildir_scan_err);
	if (!prioq_get(&pq, &pe))
		_exit(0);	/*- nothing new */
	if ((fdlock = open_append(mbox)) == -1)
		strerr_die4sys(111, FATAL, "unable to lock ", mbox, ": ");
	if (lock_ex(fdlock) == -1)
		strerr_die4sys(111, FATAL, "unable to lock ", mbox, ": ");
	if ((fdoldmbox = open_read(mbox)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", mbox, ": ");
	if ((fdnewmbox = open_trunc(mboxtmp)) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", mboxtmp, ": ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fdoldmbox, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fdnewmbox, outbuf, sizeof(outbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		strerr_die4sys(111, FATAL, "unable to read ", mbox, ": ");
	case -3:
		strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
	}
	while (prioq_get(&pq, &pe)) {
		prioq_del(&pq);
		if (!prioq_insert(min, &pq2, &pe))
			die_nomem();
		if ((fd = open_read(filenames.s + pe.id)) == -1)
			strerr_die4sys(111, FATAL, "unable to read $MAILDIR/", filenames.s + pe.id, ": ");
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') != 0)
			strerr_die4sys(111, FATAL, "unable to read $MAILDIR/", filenames.s + pe.id, ": ");
		if (!stralloc_copys(&ufline, "From XXX "))
			die_nomem();
		if (match) {
			if (stralloc_starts(&line, "Return-Path: <")) {
				if (line.s[14] == '>') {
					if (!stralloc_copys(&ufline, "From MAILER-DAEMON "))
						die_nomem();
				} else {
					int             i;
					if (!stralloc_ready(&ufline, line.len) ||
							!stralloc_copys(&ufline, "From "))
						die_nomem();
					for (i = 14; i < line.len - 2; ++i)
						if ((line.s[i] == ' ') || (line.s[i] == '\t'))
							ufline.s[ufline.len++] = '-';
						else
							ufline.s[ufline.len++] = line.s[i];
					if (!stralloc_cats(&ufline, " "))
						die_nomem();
				}
			}
		}
		if (!stralloc_cats(&ufline, qtime(pe.dt)))
			die_nomem();
		if (substdio_put(&ssout, ufline.s, ufline.len) == -1)
			strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
		while (match && line.len) {
			if (gfrom(line.s, line.len)) {
				if (substdio_puts(&ssout, ">") == -1)
					strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
			}
			if (substdio_put(&ssout, line.s, line.len) == -1)
				strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
			if (!match) {
				if (substdio_puts(&ssout, "\n") == -1)
					strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
				break;
			}
			if (getln(&ssin, &line, &match, '\n') != 0)
				strerr_die4sys(111, FATAL, "unable to read $MAILDIR/", filenames.s + pe.id, ": ");
		}
		if (substdio_puts(&ssout, "\n"))
			strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
		close(fd);
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
	if (fsync(fdnewmbox) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
	if (close(fdnewmbox) == -1)	/*- NFS dorks */
		strerr_die4sys(111, FATAL, "unable to write to ", mboxtmp, ": ");
	if (rename(mboxtmp, mbox) == -1)
		strerr_die6(111, FATAL, "unable to move ", mboxtmp, " to ", mbox, ": ", &strerr_sys);
	while (prioq_get(&pq2, &pe)) {
		prioq_del(&pq2);
		if (unlink(filenames.s + pe.id) == -1)
			strerr_warn4(WARNING, "$MAILDIR/", filenames.s + pe.id, " will be delivered twice; unable to unlink: ", &strerr_sys);
	}
	return(0);
}

void
getversion_maildir2mbox_c()
{
	const char     *x = "$Id: maildir2mbox.c,v 1.11 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmyctimeh;
	x++;
}
/*
 * $Log: maildir2mbox.c,v $
 * Revision 1.11  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.10  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.8  2021-06-03 18:12:04+05:30  Cprogrammer
 * use new prioq functions
 *
 * Revision 1.7  2021-06-01 10:05:09+05:30  Cprogrammer
 * replaced myctime() with libqmail qtime()
 *
 * Revision 1.6  2020-11-24 13:45:41+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.5  2004-10-22 20:26:09+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 14:58:53+05:30  Cprogrammer
 * added RCS log
 *
 */
