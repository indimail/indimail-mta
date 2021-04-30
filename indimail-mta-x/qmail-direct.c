/*
 * $Log: qmail-direct.c,v $
 * Revision 1.1  2021-04-29 21:16:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <sys/time.h>
#include <pwd.h>
#include "sig.h"
#include "control.h"
#include "env.h"
#include "stralloc.h"
#include "seek.h"
#include "str.h"
#include "fmt.h"
#include "alloc.h"
#include "substdio.h"
#include "datetime.h"
#include "now.h"
#include "date822fmt.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "fmtqfn.h"
#include "variables.h"

#define DEATH 86400				/* 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR 1003

int             uidinit(int);

static char     inbuf[2048];
static char     outbuf[256];
static struct substdio ssin, ssout;

static datetime_sec starttime;
static struct datetime dt;
static unsigned long   mypid, myuid, messnum;
static char    *pidfn;
static struct stat pidst;
static char    *messfn, *intdfn, *tmp_fn;
static int      messfd, intdfd, mailfd, flagmademess = 0, flagmadeintd = 0;

void
die(int e)
{
	_exit(e);
}

void
cleanup()
{
	if (tmp_fn) {
		seek_trunc(mailfd, 0);
		if (unlink(tmp_fn) == -1)
			return;
	}
}

void
die_write()
{
	die(53);
}

void
die_read()
{
	die(54);
}

void
die_nomem()
{
	cleanup();
	die(51);
}

void
sigalrm()
{								/* thou shalt not clean up here */
	die(52);
}

void
sigbug()
{
	die(81);
}

int
open_exclr(const char *fn)
{
	return open(fn, O_RDWR | O_EXCL | O_CREAT, 0644);
}

unsigned int    receivedlen;
char           *received;
/*
 * "Received: (qmail-queue invoked by alias); 26 Sep 1995 04:46:54 -0000\n" 
 */

static unsigned int
receivedfmt(char *s)
{
	unsigned int    i;
	unsigned int    len;
	len = 0;
	i = fmt_str(s, "Received: (qmail ");
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
	if (myuid == auto_uida) {
		i = fmt_str(s, "by alias");
		len += i;
		if (s)
			s += i;
	} else
	if (myuid == auto_uidd) {
		i = fmt_str(s, "from network");
		len += i;
		if (s)
			s += i;
	} else
	if (myuid == auto_uids) {
		i = fmt_str(s, "for bounce");
		len += i;
		if (s)
			s += i;
	} else {
		i = fmt_str(s, "by uid ");
		len += i;
		if (s)
			s += i;
		i = fmt_ulong(s, myuid);
		len += i;
		if (s)
			s += i;
	}
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
		die(51);
	receivedfmt(received);
}

unsigned int
pidfmt(char *s, unsigned long seq)
{
	unsigned int    i;
	unsigned int    len;

	len = 0;
	i = fmt_str(s, "pid/");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, starttime);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, seq);
	len += i;
	if (s)
		s += i;
	++len;
	if (s)
		*s++ = 0;
	return len;
}

char           *
fnnum(char *dirslash, int flagsplit)
{
	char           *s;

	if (!(s = alloc(fmtqfn((char *) 0, dirslash, messnum, flagsplit))))
		die(51);
	fmtqfn(s, dirslash, messnum, flagsplit);
	return s;
}

void
pidopen()
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq);
	if (!(pidfn = alloc(len)))
		die(51);
	for (seq = 1; seq < 10; ++seq) {
		if (pidfmt((char *) 0, seq) > len)
			die(81); /* paranoia */
		pidfmt(pidfn, seq);
		if ((messfd = open_exclr(pidfn)) != -1)
			return;
	}
	die(63);
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
				die_nomem();
			continue;
		}
		if (host_a[loop] == ':') {
			if (!stralloc_cats(&hostname, "072"))
				die_nomem();
			continue;
		}
		if (!stralloc_append(&hostname, s + loop))
			die_nomem();
	}
	if (!stralloc_copyb(&fntmptph, "tmp/", 4))
		die_nomem();
	for (loop = 0;; ++loop) {
		gettimeofday(&cur, 0);
		strnum[fmt_ulong(strnum, cur.tv_sec)] = 0;
		fntmptph.len = 4;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_append(&fntmptph, "M"))
			die_nomem();
		strnum[fmt_ulong(strnum, cur.tv_usec)] = 0;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, "P"))
			die_nomem();
		strnum[fmt_ulong(strnum, pid)] = 0;
		if (!stralloc_cats(&fntmptph, strnum)
				|| !stralloc_append(&fntmptph, ".")
				|| !stralloc_cat(&fntmptph, &hostname)
				|| !stralloc_0(&fntmptph))
			die_nomem();
		if ((mailfd = open_exclr(fntmptph.s)) >= 0) {
			tmp_fn = fntmptph.s;
			break;
		}
		if (errno == error_exist) {
			/*- really should never get to this point */
			if (loop == 2) {
				cleanup();
				die(61);
			}
			usleep(100);
		} 
		cleanup();
		die(61);
	}
	if (fstat(mailfd, &st) == -1) {
		cleanup();
		die(61);
	}
	if (fchown(mailfd, uid, gid)) {
		cleanup();
		die(50);
	}
	if (!stralloc_copyb(&fnnewtph, "new/", 4))
		die_nomem();
	strnum[fmt_ulong(strnum, cur.tv_sec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, "."))
		die_nomem();
	/*- in hexadecimal */
	if (!stralloc_append(&fnnewtph, "I"))
		die_nomem();
	s = alloc(fmt_xlong(0, st.st_ino) + fmt_xlong(0, st.st_dev));
	i = fmt_xlong(s, st.st_ino);
	if (!stralloc_catb(&fnnewtph, s, i)
			|| !stralloc_append(&fnnewtph, "V"))
		die_nomem();
	i = fmt_xlong(s, st.st_dev);
	if (!stralloc_catb(&fnnewtph, s, i))
		die_nomem();
	alloc_free(s);
	/*- in decimal */
	if (!stralloc_append(&fnnewtph, "M"))
		die_nomem();
	strnum[fmt_ulong(strnum, cur.tv_usec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, "P"))
		die_nomem();
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum)
			|| !stralloc_append(&fnnewtph, ".")
			|| !stralloc_cat(&fnnewtph, &hostname)
			|| !stralloc_0(&fnnewtph))
		die_nomem();
	return;
}

int
main(int argc, char **argv)
{
	unsigned int    len, rcptcount, token_len, auth_flag;
	char           *ptr;
	struct passwd  *pw;
	char            ch;

	sig_blocknone();
	umask(033);
	if (uidinit(1) == -1)
		die(67);
	if (chdir(auto_qmail) == -1)
		die(61);
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/queue1";
	if (chdir(queuedir) == -1)
		die(62);
	myuid = getuid();
	if (myuid) {
		if (!(pw = getpwuid(myuid)))
			die(67);
		if (control_readfile(&uidlist, "direct_mail_users", 0) == -1)
			die(55);
		if (!uidlist.len)
			die(55);
		for (auth_flag = len = 0, ptr = uidlist.s;len < uidlist.len;) {
			len += ((token_len = str_len(ptr)) + 1);
			if (!str_diffn(ptr, pw->pw_name, token_len)) {
				auth_flag = 1;
				break;
			}
			ptr = uidlist.s + len;
		}
		if (!auth_flag)
			die(55);
	} else /*- allow uid 0 to send without restriction */
	if (!(pw = getpwnam("alias")))
		die(67);
	mypid = getpid();
	starttime = now();
	datetime_tai(&dt, starttime);
	received_setup();
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);

	/*- open pid file */
	pidopen();
	if (fstat(messfd, &pidst) == -1)
		die(63);
	messnum = pidst.st_ino;
	messfn = fnnum("mess/", 0);
	intdfn = fnnum("intd/", 0);
	/*- link mess file to pid file */
	if (link(pidfn, messfn) == -1) {
		unlink(pidfn);
		die(64);
	}
	/*- remove pid file */
	if (unlink(pidfn) == -1) {
		unlink(pidfn);
		die(63);
	}
	flagmademess = 1;
	if (unlink(messfn) == -1)
		die(63);
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof (outbuf));
	/*- read the message body */
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof (inbuf)); /*- message is read from fd 0 */
	if (substdio_bput(&ssout, received, receivedlen) == -1)
		die_write();
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die_read();
	case -3:
		die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (fsync(messfd) == -1)
		die_write();
	/*- write the envelope */
	if ((intdfd = open_exclr(intdfn)) == -1)
		die(65);
	flagmadeintd = 1;
	if (unlink(intdfn) == -1)
		die(63);
	substdio_fdbuf(&ssout, write, intdfd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, read, 1, inbuf, sizeof (inbuf)); /*- envelop is read from fd 1 */

	/*- Return-Path */
	if (substdio_get(&ssin, &ch, 1) < 1)
		die_read();
	if (ch != 'F')
		die(91);
	if (substdio_bput(&ssout, "Return-Path: ", 13) == -1)
		die_write();
	for (len = 0; len < ADDR; ++len) {
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (!ch) {
			if (substdio_put(&ssout, "\n", 1) == -1)
				die_write();
			break;
		} else
		if (substdio_put(&ssout, &ch, 1) == -1)
			die_write();
	}
	if (len >= ADDR)
		die(11);
	/*- get recipients */
	if (substdio_bput(&ssout, "X-Recipients: ", 14) == -1)
		die_write();
	for (rcptcount = 0;;) {
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (!ch) {
			if (substdio_put(&ssout, "\n", 1) == -1)
				die_write();
			break;
		}
		if (rcptcount) {
			if (substdio_put(&ssout, ", ", 2) == -1)
				die_write();
		}
		/*- new recipient */
		if (ch != 'T')
			die(91);
		rcptcount++;
		for (len = 0; len < ADDR; ++len) {
			if (substdio_get(&ssin, &ch, 1) < 1)
				die_read();
			if (!ch)
				break;
			if (substdio_bput(&ssout, &ch, 1) == -1)
				die_write();
		}
		if (len >= ADDR)
			die(11);
	} /*- for (rcptcount = 0;;) */
	if (substdio_flush(&ssout) == -1 || fsync(intdfd) == -1)
		die_write();
	if (seek_set(messfd, 0) == -1 || seek_set(intdfd, 0) == -1)
		die_read();
	if (chdir(pw->pw_dir) == -1 || chdir("Maildir") == -1)
		die(61);
	/*- write the mail */
	mailopen(pw->pw_uid, pw->pw_gid);
	substdio_fdbuf(&ssout, write, mailfd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, read, intdfd, inbuf, sizeof (inbuf)); /*- envelop is read from fd 1 */
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die_read();
	case -3:
		die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	substdio_fdbuf(&ssin, read, messfd, inbuf, sizeof (inbuf)); /*- envelop is read from fd 1 */
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die_read();
	case -3:
		die_write();
	}
	if (fsync(mailfd) == -1)
		die_write();
	close(mailfd);
	close(messfd);
	close(intdfd);
	/*- link new/file to pid/file */
	if (link(fntmptph.s, fnnewtph.s) == -1) {
		unlink(fntmptph.s);
		die(61);
	}
	/*- remove tmp_fn file */
	if (unlink(fntmptph.s) == -1)
		die(61);
	die(0);
}

void
getversion_qmail_direct_c()
{
	static char    *x = "$Id: qmail-direct.c,v 1.1 2021-04-29 21:16:22+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
