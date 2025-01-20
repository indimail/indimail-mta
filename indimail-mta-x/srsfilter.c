/*
 * $Log: srsfilter.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-03-08 20:06:55+05:30  Cprogrammer
 * discard double, triple bounces
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:48:31+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2019-07-18 10:53:18+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.1  2014-01-01 19:27:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hassrs.h"
#ifdef HAVESRS
#include <unistd.h>
#include <sig.h>
#include <env.h>
#include <str.h>
#include <strerr.h>
#include <substdio.h>
#include <fmt.h>
#include <stralloc.h>
#include <noreturn.h>
#include "control.h"
#include "qmail.h"
#include "srs.h"

#define FATAL  "srsfilter: fatal: "
#define WARN   "srsfilter: warn: "
#define IGNORE "srsfilter: ignore: "

static struct qmail    qqt;
static stralloc line = { 0 };
static stralloc doublebounceto = { 0 };
static stralloc doublebouncehost = { 0 };
static int      flagbody = 0, flagnewline = 0, flagto = 0, seento = 0;

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

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
mywrite(int fd, const char *buf, size_t len)
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
				} else
					flagto = 0;
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

void
do_control()
{
	if (control_init() == -1 ||
			control_rldef(&doublebouncehost, "doublebouncehost", 1, "doublebouncehost") != 1 ||
			control_rldef(&doublebounceto, "doublebounceto", 0, "postmaster") != 1)
		strerr_die2x(111, FATAL, "unable to read controls");
	if (!stralloc_cats(&doublebounceto, "@") ||
			!stralloc_cat(&doublebounceto, &doublebouncehost) ||
			!stralloc_0(&doublebounceto))
		die_nomem();
	return;
}

int
main(int argc, char **argv)
{
	char           *ext2, *host, *sender;
	const char     *qqx;
	char            inbuf[SUBSTDIO_INSIZE], outbuf[1], num[FMT_ULONG];
	substdio        ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof inbuf);
	substdio        ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) mywrite, -1, outbuf, sizeof outbuf);

	sig_pipeignore();
	if (!(sender = env_get("SENDER")))
		strerr_die2x(100, FATAL, "SENDER not set");
	if (*sender) /*- Return zero, the message will not bounce back */
		strerr_die2x(0, IGNORE, "SENDER must be empty");
	if (!(ext2 = env_get("EXT2")))
		strerr_die2x(100, FATAL, "EXT2 not set");
	if (!(host = env_get("HOST")))
		strerr_die2x(100, FATAL, "HOST not set");
	do_control();
	switch (srsreverse(ext2))
	{
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
	if (str_equal(srs_result.s, "#@[]"))
		strerr_die2x(0, WARN, "triple bounce: discarding message");
	else
	if (!srs_result.len && *doublebounceto.s == '@')
		strerr_die2x(0, WARN, "double bounce: discarding message");
	else {
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
	}
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
main(int argc, char **argv)
{
	substdio_fdbuf(&sserr, (ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof(sserrbuf));
	substdio_puts(&sserr, "not compiled with -DHAVESRS\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_srsfilter_c()
{
	const char     *x = "$Id: srsfilter.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
