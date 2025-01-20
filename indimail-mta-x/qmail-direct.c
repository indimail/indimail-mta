/*
 * $Id: qmail-direct.c,v 1.14 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <sys/time.h>
#include <pwd.h>
#include <open.h>
#include <sig.h>
#include <env.h>
#include <getEnvConfig.h>
#include <stralloc.h>
#include <seek.h>
#include <str.h>
#include <fmt.h>
#include <alloc.h>
#include <substdio.h>
#include <datetime.h>
#include <now.h>
#include <scan.h>
#include <date822fmt.h>
#include <qgetpwgr.h>
#include <noreturn.h>
#include "pidopen.h"
#include "custom_error.h"
#ifdef USE_FSYNC
#include "syncdir.h"
#endif
#include "qmail.h"

#define ADDR   1003

static char     inbuf[2048];
static char     outbuf[256];
static struct substdio ssin, ssout;

static datetime_sec starttime;
static struct datetime dt;
static unsigned long   mypid, myuid, messnum;
static unsigned int    receivedlen;
static struct stat pidst;
static char    *messfn, *intdfn, *tmp_fn;
static int      intdfd, mailfd, flagmademess = 0, flagmadeintd = 0;
static int      qm_custom_err = 0;
static char    *received;
static stralloc err_str = { 0 };

void
cleanup()
{
	int             e;

	if (tmp_fn) {
		e = errno;
		seek_trunc(mailfd, 0);
		if (unlink(tmp_fn) == -1)
			return;
		errno = e;
	}
}

no_return void
die(int e, const char *str)
{
	cleanup();
	if (qm_custom_err)
		custom_error("qmail-direct", "Z", str, errno ? error_str(errno) : 0, "X.3.0");
	else
		_exit(e);
}

no_return void
sigalrm(int x)
{	/* thou shalt not clean up here */
	die(52, "timer expired");
}

no_return void
sigbug(int x)
{
	die(81, "internal bug");
}

/*
 * "Received: (qmail-queue invoked by alias); 26 Sep 1995 04:46:54 -0000\n"
 */

static unsigned int
receivedfmt(char *s)
{
	unsigned int    i;
	unsigned int    len;
	len = 0;
	i = fmt_str(s, "Received: (indimail-mta ");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, " invoked ");
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "by uid ");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, myuid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "); ");
	len += i;
	if (s)
		s += i;
	i = date822fmt(s, &dt);
	len += i;
	if (s)
		s += i;
	return len;
}

void
received_setup()
{
	receivedlen = receivedfmt((char *) 0);
	if (!(received = alloc(receivedlen + 1)))
		die(51, "out of memory");
	receivedfmt(received);
}

unsigned int
fmtqfn(char *s, const char *dirslash, unsigned long id, const char *suffix)
{
	unsigned int    len;
	unsigned int    i;

	len = 0;
	i = fmt_str(s, dirslash);
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, id);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, suffix);
	len += i;
	if (s)
		s += i;
	if (s)
		*s++ = 0;
	++len;
	return len;
}

char           *
fnnum(const char *dirslash, const char *suffix)
{
	char           *s;

	if (!(s = alloc(fmtqfn((char *) 0, dirslash, messnum, suffix))))
		die(51, "out of memory");
	fmtqfn(s, dirslash, messnum, suffix);
	return s;
}

stralloc        fntmptph, fnnewtph, hostname, uidlist;

void
mailopen(uid_t uid, gid_t gid)
{
	char            strnum[FMT_ULONG], host_a[64];
	char           *s;
	struct stat     st;
	int             loop, i;
	struct timeval  cur;
	pid_t           pid;

	pid = getpid();
	host_a[0] = 0;
	gethostname(host_a, sizeof(host_a));
	s = host_a;

	for (loop = 0; loop < str_len(host_a); ++loop) {
		if (host_a[loop] == '/') {
			if (!stralloc_cats(&hostname, "057"))
				die(51, "out of memory");
			continue;
		}
		if (host_a[loop] == ':') {
			if (!stralloc_cats(&hostname, "072"))
				die(51, "out of memory");
			continue;
		}
		if (!stralloc_append(&hostname, s + loop))
			die(51, "out of memory");
	}
	if (!stralloc_copyb(&fntmptph, "tmp/", 4))
		die(51, "out of memory");
	for (loop = 0;; ++loop) {
		gettimeofday(&cur, 0);
		strnum[fmt_ulong(strnum, cur.tv_sec)] = 0;
		fntmptph.len = 4;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_append(&fntmptph, "M"))
			die(51, "out of memory");
		strnum[fmt_ulong(strnum, cur.tv_usec)] = 0;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, "P"))
			die(51, "out of memory");
		strnum[fmt_ulong(strnum, pid)] = 0;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_cat(&fntmptph, &hostname)
				|| !stralloc_0(&fntmptph))
			die(51, "out of memory");
		fntmptph.len--;
		if ((mailfd = open_exclr(fntmptph.s)) >= 0) {
			tmp_fn = fntmptph.s;
			break;
		}
		if (errno == error_exist) {
			/*- really should never get to this point */
			if (loop == 2) {
				if (!stralloc_copyb(&err_str, "open: ", 6) ||
						!stralloc_cat(&err_str, &fntmptph) ||
						!stralloc_0(&err_str))
					die(51, "out of memory");
				die(68, err_str.s);
			}
			usleep(100);
		}
		if (!stralloc_copyb(&err_str, "open: ", 6) ||
				!stralloc_cat(&err_str, &fntmptph) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(68, err_str.s);
	}
	if (fstat(mailfd, &st) == -1) {
		if (!stralloc_copyb(&err_str, "fstat: ", 7) ||
				!stralloc_cats(&err_str, pidfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(68, err_str.s);
	}
	if (fchown(mailfd, uid, gid)) {
		if (!stralloc_copyb(&err_str, "fchown: ", 8) ||
				!stralloc_cat(&err_str, &fntmptph) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(68, err_str.s);
	}
	if (!stralloc_copyb(&fnnewtph, "new/", 4))
		die(51, "out of memory");
	strnum[fmt_ulong(strnum, cur.tv_sec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, "."))
		die(51, "out of memory");
	/*- in hexadecimal */
	if (!stralloc_append(&fnnewtph, "I"))
		die(51, "out of memory");
	s = alloc(fmt_xlong(0, st.st_ino) + fmt_xlong(0, st.st_dev));
	i = fmt_xlong(s, st.st_ino);
	if (!stralloc_catb(&fnnewtph, s, i)
			|| !stralloc_append(&fnnewtph, "V"))
		die(51, "out of memory");
	i = fmt_xlong(s, st.st_dev);
	if (!stralloc_catb(&fnnewtph, s, i))
		die(51, "out of memory");
	alloc_free(s);
	/*- in decimal */
	if (!stralloc_append(&fnnewtph, "M"))
		die(51, "out of memory");
	strnum[fmt_ulong(strnum, cur.tv_usec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, "P"))
		die(51, "out of memory");
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, ".")
			|| !stralloc_cat(&fnnewtph, &hostname)
			|| !stralloc_0(&fnnewtph))
		die(51, "out of memory");
	return;
}

int
main(int argc, char **argv)
{
	unsigned int    ret, len, rcptcount, use_pwgr, fastqueue;
	unsigned long   death;
	uid_t           uid;
	gid_t           gid;
	char           *ptr, *home;
	struct passwd  *pw;
	char            ch;

	sig_blocknone();
	umask(033);
	if (!(ptr = env_get("FASTQUEUE")))
		fastqueue = 0;
	else
		scan_uint(ptr, &fastqueue);
	if (fastqueue) {
		uid = fastqueue;
		gid = -1;
		if (!(home = env_get("HOME")))
			die(78, "trouble getting home directory for user");
	} else {
		if (!(ptr = env_get("USER")))
			die(78, "trouble getting user");
		use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
		if (!(pw = (use_pwgr ? qgetpwnam : getpwnam) (ptr)))
			die(78, "trouble getting uid/gid");
		home = pw->pw_dir;
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
#ifdef USE_FSYNC
	ptr = env_get("USE_FSYNC");
	use_fsync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_FDATASYNC");
	use_fdatasync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_SYNCDIR");
	use_syncdir = (ptr && *ptr) ? 1 : 0;
#endif
	qm_custom_err = env_get("QMAIL_QUEUE_CUSTOM_ERROR") ? 1 : 0;
	if (!stralloc_copys(&fntmptph, home) ||
			!stralloc_catb(&fntmptph, "/Maildir", 8) ||
			!stralloc_0(&fntmptph))
		die(51, "out of memory");
	if (chdir(fntmptph.s) == -1) {
		if (!stralloc_copyb(&err_str, "chdir: $HOME/Maildir", 22) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(61, err_str.s);
	}
	myuid = getuid();
	mypid = getpid();
	starttime = now();
	datetime_tai(&dt, starttime);
	received_setup();
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	getEnvConfiguLong(&death, "DEATH", DEATH);
	alarm(death);

	/*- set pidfn and open with fd = messfd */
	if ((ret = pidopen(starttime, "tmp"))) {
		if (!stralloc_copyb(&err_str, "open: ", 6) ||
				!stralloc_cats(&err_str, pidfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(ret, err_str.s);
	}
	if (fstat(messfd, &pidst) == -1) {
		if (!stralloc_copyb(&err_str, "fstat: ", 7) ||
				!stralloc_cats(&err_str, pidfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(70, err_str.s);
	}
	messnum = pidst.st_ino;
	messfn = fnnum("tmp/", ".h");
	intdfn = fnnum("tmp/", ".m");
	/*- link mess file to pid file */
	if (link(pidfn, messfn) == -1) {
		if (!stralloc_copyb(&err_str, "link: ", 6) ||
				!stralloc_cats(&err_str, messfn) ||
				!stralloc_catb(&err_str, " -> ", 4) ||
				!stralloc_cats(&err_str, pidfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		unlink(pidfn);
		die(67, err_str.s);
	}
	/*- remove pid file */
	if (unlink(pidfn) == -1) {
		if (!stralloc_copyb(&err_str, "unlink: ", 8) ||
				!stralloc_cats(&err_str, pidfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		unlink(pidfn);
		die(70, err_str.s);
	}
	flagmademess = 1;
	if (unlink(messfn) == -1) {
		if (!stralloc_copyb(&err_str, "unlink: ", 8) ||
				!stralloc_cats(&err_str, messfn) ||
				!stralloc_0(&err_str))
			die(51, "out of memory");
		die(70, err_str.s);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, messfd, outbuf, sizeof (outbuf));
	/*- read the message body */
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof (inbuf)); /*- message is read from fd 0 */
	if (substdio_bput(&ssout, received, receivedlen) == -1)
		die(53, "trouble writing message");
	switch (substdio_copy(&ssout, &ssin)) /*- copy message body to messfn */
	{
	case -2:
		die(54, "trouble reading message");
	case -3:
		die(53, "trouble writing message");
	}
	if (substdio_flush(&ssout) == -1)
		die(53, "trouble writing message");
	/*- write the envelope */
	if ((intdfd = open_exclr(intdfn)) == -1)
		die(65, "trouble creating files in intd");
	flagmadeintd = 1;
	if (unlink(intdfn) == -1)
		die(80, "trouble removing intdfn");
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, intdfd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, 1, inbuf, sizeof (inbuf)); /*- envelope is read from fd 1 */

	/*- Return-Path */
	if (substdio_get(&ssin, &ch, 1) < 1)
		die(54, "trouble reading envelope");
	if (ch != 'F')
		die(79, "envelope format error");
	if (substdio_bput(&ssout, "Return-Path: ", 13) == -1)
		die(53, "trouble writing envelope");
	for (len = 0; len < ADDR; ++len) {
		if (substdio_get(&ssin, &ch, 1) < 1)
			die(54, "trouble reading envelope");
		if (!ch) {
			if (substdio_put(&ssout, "\n", 1) == -1)
				die(53, "trouble writing envelope");
			break;
		} else
		if (substdio_put(&ssout, &ch, 1) == -1)
			die(53, "trouble writing envelope");
	}
	if (len >= ADDR)
		die(11, "envelope address too long");
	/*- get recipients */
	if (substdio_bput(&ssout, "X-Recipients: ", 14) == -1)
		die(53, "trouble writing envelope");
	for (rcptcount = 0;;) {
		if (substdio_get(&ssin, &ch, 1) < 1)
			die(54, "trouble reading recipients");
		if (!ch) {
			if (substdio_put(&ssout, "\n", 1) == -1)
				die(53, "trouble writing envelope");
			break;
		}
		if (rcptcount) {
			if (substdio_put(&ssout, ", ", 2) == -1)
				die(53, "trouble writing envelope");
		}
		/*- new recipient */
		if (ch != 'T')
			die(79, "envelope format error");
		rcptcount++;
		for (len = 0; len < ADDR; ++len) {
			if (substdio_get(&ssin, &ch, 1) < 1)
				die(54, "trouble reading envelope");
			if (!ch)
				break;
			if (substdio_bput(&ssout, &ch, 1) == -1)
				die(53, "trouble writing envelope");
		}
		if (len >= ADDR)
			die(11, "envelope address too long");
	} /*- for (rcptcount = 0;;) */
	if (substdio_flush(&ssout) == -1)
		die(53, "trouble writing envelope");
	if (seek_set(messfd, 0) == -1)
		die(54, "trouble reading messfd");
	if (seek_set(intdfd, 0) == -1)
		die(54, "trouble seeking intdfd");
	if (chdir(home) == -1 || chdir("Maildir") == -1)
		die(61, "trouble doing cd to Maildir");
	/*- write the mail */
	mailopen(uid, gid);
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, mailfd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, intdfd, inbuf, sizeof (inbuf)); /*- envelope read earlier from fd 1 */
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die(54, "trouble reading envelope");
	case -3:
		die(53, "trouble writing message");
	}
	if (substdio_flush(&ssout) == -1)
		die(53, "trouble writing message");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, messfd, inbuf, sizeof (inbuf)); /*- body read earlier from fd 0 */
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die(54, "trouble reading message body");
	case -3:
		die(53, "trouble writing message");
	}
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(mailfd) : fsync(mailfd)) == -1)
		die(53, "trouble syncing message to disk");
#else
	if (fsync(mailfd) == -1)
		die(53, "trouble syncing message to disk");
#endif
	close(mailfd);
	close(messfd);
	close(intdfd);
	/*- link new/file to tmp/file */
	if (link(fntmptph.s, fnnewtph.s) == -1)
		die(68, "trouble linking files in Maildir/new to Maildir/tmp");
	/*- remove tmp_fn file */
	if (unlink(fntmptph.s) == -1)
		die(68, "trouble removing file in Maildir/tmp");
#if !defined(SYNCDIR_H) && defined(USE_FSYNC) && defined(LINUX)
	if (use_syncdir > 0) {
		if ((mailfd = open(fnnewtph.s, O_RDONLY)) == -1 ||
				(use_fdatasync > 0 ? fdatasync(mailfd) : fsync(mailfd)) == -1 || close(mailfd) == -1) {
			if (!stralloc_copyb(&err_str, "trouble syncing ", 16) ||
					!stralloc_cats(&err_str, home) ||
					!stralloc_catb(&err_str, " directory", 10) ||
					!stralloc_0(&err_str))
				die(51, "out of memory");
			die(69, err_str.s);
		}
	}
#endif
	_exit(0);
}

void
getversion_qmail_direct_c()
{
	const char     *x = "$Id: qmail-direct.c,v 1.14 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail-direct.c,v $
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2023-12-25 09:40:23+05:30  Cprogrammer
 * made DEATH configurable
 *
 * Revision 1.12  2022-04-04 14:23:27+05:30  Cprogrammer
 * refactored fastqueue and added setting of fdatasync()
 *
 * Revision 1.11  2022-04-03 18:41:49+05:30  Cprogrammer
 * use custom_error() for error messages
 *
 * Revision 1.10  2021-09-11 19:00:55+05:30  Cprogrammer
 * replace qmail with indimail-mta in received header
 *
 * Revision 1.9  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.8  2021-07-05 21:23:10+05:30  Cprogrammer
 * use qgetpw interface from libqmail if USE_QPWGR is set
 *
 * Revision 1.7  2021-06-24 12:16:56+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.6  2021-06-15 21:51:31+05:30  Cprogrammer
 * pass local Maildir/tmp as argument to pidopen
 *
 * Revision 1.5  2021-06-12 18:16:49+05:30  Cprogrammer
 * moved pidopen out to its own file
 *
 * Revision 1.4  2021-05-01 22:31:32+05:30  Cprogrammer
 * use standard Maildir for queue operation
 * removed control file direct_mail_users
 *
 * Revision 1.3  2021-05-01 18:37:30+05:30  Cprogrammer
 * use modified Maildir as queue
 *
 * Revision 1.2  2021-05-01 14:50:33+05:30  Cprogrammer
 * removed uidinit() and auto_uids to run on a minimal system
 *
 * Revision 1.1  2021-04-29 21:16:22+05:30  Cprogrammer
 * Initial revision
 *
 */
