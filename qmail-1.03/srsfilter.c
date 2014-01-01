/*
 * $Log: srsfilter.c,v $
 * Revision 1.1  2014-01-01 19:27:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hassrs.h"
#ifdef HAVESRS
#include "sig.h"
#include "readwrite.h"
#include "exit.h"
#include "env.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"
#include "stralloc.h"
#include "srs.h"

#define FATAL  "srsfilter: fatal: "
#define IGNORE "srsfilter: ignore: "

void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

struct qmail    qqt;

stralloc        line = { 0 };

int             flagbody = 0;
int             flagnewline = 0;
int             flagto = 0;
int             seento = 0;

void
newheader()
{
	if (!stralloc_copyb(&line, "To: ", 4))
		die_nomem();
	if (!stralloc_cat(&line, &srs_result))
		die_nomem();
	++flagto;
	++seento;
}

void
skipheader()
{
	if (!stralloc_copys(&line, ""))
		die_nomem();
}

void
printheader()
{
	qmail_put(&qqt, line.s, line.len);
	qmail_put(&qqt, "\n", 1);
	if (!stralloc_copys(&line, ""))
		die_nomem();
}

ssize_t
mywrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             i;
	if (flagbody) {
		qmail_put(&qqt, buf, len);
		return len;
	} else {
		i = 0;
		while (buf[i]) {
			if (buf[i] == '\n') {
				if (flagnewline) {
					if (!seento) {
						newheader();
						printheader();
					}
					qmail_put(&qqt, "\n", 1);
					i++;
					flagbody = 1;
					continue;
				}
				if (flagto && (line.s[0] == ' ' || line.s[0] == '\t')) {
					skipheader();
					i++;
					continue;
				}
				if (line.len > 2 && line.s[2] == ':' && (line.s[1] == 'o' || line.s[1] == 'O')
					&& (line.s[0] == 'T' || line.s[0] == 't')) {
					if (seento) {
						skipheader();
						i++;
						continue;
					}
					newheader();
				} else {
					flagto = 0;
				}
				printheader();
				flagnewline = 1;
			} else {
				if (!stralloc_append(&line, &buf[i]))
					die_nomem();
				flagnewline = 0;
			}
			++i;
		}
		return len;
	}
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
	char           *ext2;
	char           *host;
	char           *sender;
	char           *qqx;

	sig_pipeignore();
	if (!(sender = env_get("SENDER")))
		strerr_die2x(100, FATAL, "SENDER not set");
	if (*sender) {
	/*
	 * Return zero, the message will not bounce back 
	 */
		strerr_die2x(0, IGNORE, "SENDER must be empty");
	}
	if (!(ext2 = env_get("EXT2")))
		strerr_die2x(100, FATAL, "EXT2 not set");
	if (!(host = env_get("HOST")))
		strerr_die2x(100, FATAL, "HOST not set");
	switch (srsreverse(ext2)) {
	case -3:
		strerr_die2x(100, FATAL, srs_error.s);
		break;
	case -2:
		die_nomem();
		break;
	case -1:
		strerr_die2x(111, FATAL, "unable to read controls");
		break;
	case 0:
		strerr_die2x(100, FATAL, "unable to rewrite envelope");
		break;
	}
	if (qmail_open(&qqt) == -1)
		strerr_die2x(111, FATAL, "unable to fork");
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2x(111, FATAL, "unable to read message");
	substdio_flush(&ssout);
	if (!flagbody) {
		qmail_fail(&qqt);
		strerr_die2x(100, FATAL, "unable to read message body");
	}
	num[fmt_ulong(num, qmail_qp(&qqt))] = 0;
	/* Always from nullsender */
	qmail_from(&qqt, "");
	qmail_to(&qqt, srs_result.s);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(0, "srsfilter: qp ", num);
	/*- no return */
	return (0);
}
#else
#warning "not compiled with -DHAVESRS"
#include "substdio.h"
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	substdio_fdbuf(&sserr, write, 2, sserrbuf, sizeof(sserrbuf));
	substdio_puts(&sserr, "not compiled with -DHAVESRS\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_srsfilter_c()
{
	static char    *x = "$Id: srsfilter.c,v 1.1 2014-01-01 19:27:49+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
