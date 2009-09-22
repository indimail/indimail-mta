/*
 * $Log: rrforward.c,v $
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
#include "sig.h"
#include "lock.h"
#include "str.h"
#include "scan.h"
#include "seek.h"
#include "exit.h"
#include "env.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"

#define FATAL "rrforward: fatal: "

#define QRR_FILE ".qmailrr"

#define QRR_LEN (sizeof(QRR_FILE)-1)
#define QRR_SEPARATOR(S) (*((S)+QRR_LEN))
#define QRR_EXTENSION(S) ((S)+QRR_LEN+1)

void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

struct qmail    qqt;

ssize_t
qqtwrite(fd, buf, len)
	int             fd;
	char           *buf;
	ssize_t         len;
{
	qmail_put(&qqt, buf, len);
	return len;
}

char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[1];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        ssout = SUBSTDIO_FDBUF(qqtwrite, -1, outbuf, sizeof outbuf);

void
rr_forward(rrto, sender, dtline, rrnum)
	char           *rrto;
	char           *sender;
	char           *dtline;
	char           *rrnum;
{
	char           *qqx;
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
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *rrfile;
	char           *extension;
	char           *sender;
	char           *dtline;
	int             rrfd;
	char            strpos[FMT_ULONG + 2];
	int             r;
	unsigned long   pos;

	extension = env_get("EXT");
	if (!extension)
		strerr_die2x(100, FATAL, "EXT not set");
	sender = env_get("NEWSENDER");
	if (!sender)
		strerr_die2x(100, FATAL, "NEWSENDER not set");
	dtline = env_get("DTLINE");
	if (!dtline)
		strerr_die2x(100, FATAL, "DTLINE not set");
	if (argc < 4)
		strerr_die2x(111, FATAL, "too few args (" QRR_FILE "[-ext] rr1@domain rr2@domain ...)");
	rrfile = argv[1];
	if (!str_start(rrfile, QRR_FILE))
		strerr_die3x(111, FATAL, argv[1], " doesn't begin with " QRR_FILE);
	if (*extension)
	{
		if (QRR_SEPARATOR(rrfile) != '-')
			_exit(0);
		if (str_diff(extension, QRR_EXTENSION(rrfile)))
			_exit(0);
	} else
	{
		if (QRR_SEPARATOR(rrfile) != '\0')
			_exit(0);
	}
	rrfd = open(rrfile, O_RDWR | O_CREAT, 0600);
	if (rrfd == -1)
		strerr_die3sys(111, FATAL, "unable to open rr: ", rrfile);
	if (lock_ex(rrfd) == -1)
		strerr_die3sys(111, FATAL, "unable to lock rr: ", rrfile);
	r = read(rrfd, strpos, FMT_ULONG);
	if (r < 0)
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
	rr_forward(argv[pos], sender, dtline, strpos);
	/*- Not reached */
	return(0);
}

void
getversion_rrforward_c()
{
	static char    *x = "$Id: rrforward.c,v 1.4 2008-07-15 19:53:39+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
