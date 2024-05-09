/*
 * $Log: rrforward.c,v $
 * Revision 1.11  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.10  2021-07-05 21:11:44+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.9  2021-05-13 14:44:37+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.8  2020-11-24 13:48:01+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2020-04-04 12:56:49+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.6  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.5  2010-06-08 22:00:46+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.4  2008-07-15 19:53:39+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.3  2004-10-22 20:30:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:38:47+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-01-04 23:18:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <envdir.h>
#include <pathexec.h>
#include <sig.h>
#include <lock.h>
#include <str.h>
#include <scan.h>
#include <seek.h>
#include <env.h>
#include <strerr.h>
#include <substdio.h>
#include <fmt.h>
#include <noreturn.h>
#include "qmail.h"
#include "set_environment.h"

#define FATAL "rrforward: fatal: "
#define WARN  "rrforward: warn: "
#define QRR_FILE ".qmailrr"
#define QRR_LEN (sizeof(QRR_FILE)-1)
#define QRR_SEPARATOR(S) (*((S)+QRR_LEN))
#define QRR_EXTENSION(S) ((S)+QRR_LEN+1)

ssize_t         qqtwrite(int fd, char *buf, size_t len);

static struct qmail    qqt;
static char     inbuf[SUBSTDIO_INSIZE], outbuf[1];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
static substdio ssout = SUBSTDIO_FDBUF(qqtwrite, -1, outbuf, sizeof outbuf);

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

ssize_t
qqtwrite(int fd, char *buf, size_t len)
{
	qmail_put(&qqt, buf, len);
	return len;
}

no_return void
rr_forward(char *rrto, char *sender, char *dtline, char *rrnum)
{
	const char     *qqx;
	char            num[FMT_ULONG];

	sig_pipeignore();
	if (qmail_open(&qqt) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	qmail_puts(&qqt, dtline);
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read message: ");
	substdio_flush(&ssout);
	num[fmt_ulong(num, qmail_qp(&qqt))] = 0;
	qmail_from(&qqt, sender);
	qmail_to(&qqt, rrto);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die4x(99, "rrforward: qp ", num, " rr ", rrnum);
}

int
main(int argc, char **argv)
{
	char            strpos[FMT_ULONG + 2];
	char           *rrfile, *extension, *sender, *dtline;
	int             rrfd, r;
	unsigned long   pos;

	if (!(extension = env_get("EXT")))
		strerr_die2x(100, FATAL, "EXT not set");
	if (!(sender = env_get("NEWSENDER")))
		strerr_die2x(100, FATAL, "NEWSENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	if (argc < 4)
		strerr_die2x(111, FATAL, "too few args (" QRR_FILE "[-ext] rr1@domain rr2@domain ...)");
	rrfile = argv[1];
	if (!str_start(rrfile, QRR_FILE))
		strerr_die3x(111, FATAL, argv[1], " doesn't begin with " QRR_FILE);
	if (*extension) {
		if (QRR_SEPARATOR(rrfile) != '-')
			_exit(0);
		if (str_diff(extension, QRR_EXTENSION(rrfile)))
			_exit(0);
	} else {
		if (QRR_SEPARATOR(rrfile) != '\0')
			_exit(0);
	}
	if ((rrfd = open(rrfile, O_RDWR | O_CREAT, 0600)) == -1)
		strerr_die3sys(111, FATAL, "unable to open rr: ", rrfile);
	if (lock_ex(rrfd) == -1)
		strerr_die3sys(111, FATAL, "unable to lock rr: ", rrfile);
	if ((r = read(rrfd, strpos, FMT_ULONG)) < 0)
		strerr_die3sys(111, FATAL, "unable to read rr: ", rrfile);
	strpos[r] = '\0';
	argc -= 2;
	argv += 2;
	if (!scan_ulong(strpos, &pos))
		pos = 0;
	else
		pos++;
	if (pos >= argc)
		pos = 0;
	r = fmt_ulong(strpos, pos);
	strpos[r] = '\n';
	if (seek_begin(rrfd) == -1)
		strerr_die3sys(111, FATAL, "unable to rewind rr: ", rrfile);
	if (write(rrfd, strpos, r + 1) != r + 1)
		strerr_die3sys(111, FATAL, "unable to write rr: ", rrfile);
	lock_un(rrfd);
	close(rrfd);
	strpos[r] = '\0';
	set_environment(WARN, FATAL, 0);
	rr_forward(argv[pos], sender, dtline, strpos);
}

void
getversion_rrforward_c()
{
	const char     *x = "$Id: rrforward.c,v 1.11 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
