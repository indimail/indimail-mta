/*
 * $Log: forward.c,v $
 * Revision 1.6  2008-07-15 19:51:23+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.5  2004-10-22 20:25:27+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:35:14+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.3  2004-07-17 20:53:27+05:30  Cprogrammer
 * added qqeh code
 *
 */
#include <unistd.h>
#include "sig.h"
#include "exit.h"
#include "env.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"

#define FATAL "forward: fatal: "

void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

struct qmail    qqt;

ssize_t
mywrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	qmail_put(&qqt, buf, len);
	return len;
}

char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[1];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        ssout = SUBSTDIO_FDBUF(mywrite, -1, outbuf, sizeof outbuf);

char            num[FMT_ULONG];

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *sender, *dtline, *qqeh, *qqx;

	sig_pipeignore();
	if (!(sender = env_get("NEWSENDER")))
		strerr_die2x(100, FATAL, "NEWSENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	if (qmail_open(&qqt) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	qmail_puts(&qqt, dtline);
	if ((qqeh = env_get("QQEH")))
		qmail_puts(&qqt, qqeh);
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read message: ");
	substdio_flush(&ssout);
	num[fmt_ulong(num, qmail_qp(&qqt))] = 0;
	qmail_from(&qqt, sender);
	while (*++argv)
		qmail_to(&qqt, *argv);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(0, "forward: qp ", num);
	/*- Not reached */
	return(0);
}

void
getversion_forward_c()
{
	static char    *x = "$Id: forward.c,v 1.6 2008-07-15 19:51:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
