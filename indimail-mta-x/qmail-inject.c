/*
 * $Log: qmail-inject.c,v $
 * Revision 1.44  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.43  2021-07-15 09:22:02+05:30  Cprogrammer
 * removed unused function
 *
 * Revision 1.42  2021-07-05 21:27:47+05:30  Cprogrammer
 * allow processing $HOME/.defaultqueue for root
 *
 * Revision 1.41  2021-06-15 11:55:01+05:30  Cprogrammer
 * moved token822.h to libqmail
 *
 * Revision 1.40  2021-05-27 11:25:45+05:30  Cprogrammer
 * removed spam ignore code
 *
 * Revision 1.39  2021-05-23 07:10:48+05:30  Cprogrammer
 * include wildmat.h for wildmat_internal
 *
 * Revision 1.38  2021-05-13 14:44:07+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.37  2021-05-12 15:49:24+05:30  Cprogrammer
 * removed redundant initialization
 *
 * Revision 1.36  2021-04-29 12:04:00+05:30  Cprogrammer
 * use 'n' option in QMAILINJECT env variable to print message rather than queue
 *
 * Revision 1.35  2020-11-24 13:46:52+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.34  2020-11-22 23:12:03+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.33  2020-05-10 17:46:59+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.32  2020-04-04 11:53:24+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
 * Revision 1.31  2020-04-03 22:09:46+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.30  2019-07-08 16:09:39+05:30  Cprogrammer
 * to not parse mail header in qmail-inject if -a is given
 *
 * Revision 1.29  2018-05-01 01:42:41+05:30  Cprogrammer
 * indented code
 *
 * Revision 1.28  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.27  2014-01-29 14:02:54+05:30  Cprogrammer
 * made domainqueue file configurable through env variable DOMAINQUEUE
 *
 * Revision 1.26  2014-01-22 20:38:19+05:30  Cprogrammer
 * added hassrs.h
 *
 * Revision 1.25  2013-11-21 15:40:52+05:30  Cprogrammer
 * added domainqueue functionality
 *
 * Revision 1.24  2013-08-25 18:37:55+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.23  2011-07-28 19:39:08+05:30  Cprogrammer
 * chdir to home after envdir_set()
 *
 * Revision 1.22  2010-06-08 22:00:27+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.21  2010-01-20 09:25:09+05:30  Cprogrammer
 * corrected error message
 *
 * Revision 1.20  2009-09-08 12:33:51+05:30  Cprogrammer
 * removed dependency of indimail on qmail-inject
 *
 * Revision 1.19  2009-08-13 19:08:00+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.18  2009-05-01 10:40:58+05:30  Cprogrammer
 * added errstr argument to envrules()
 *
 * Revision 1.17  2009-04-14 11:31:09+05:30  Cprogrammer
 * added maxrecipient check
 *
 * Revision 1.16  2008-06-12 08:39:27+05:30  Cprogrammer
 * added rulesfile argument
 *
 * Revision 1.15  2007-12-20 12:47:04+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.14  2005-08-23 17:33:49+05:30  Cprogrammer
 * added case 's' for deleting sender
 *
 * Revision 1.13  2005-04-02 23:03:17+05:30  Cprogrammer
 * replaced wildmat with wildmat_internal
 *
 * Revision 1.12  2004-10-22 20:28:21+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.11  2004-10-11 23:59:48+05:30  Cprogrammer
 * include sgetopt.h after unistd.h
 * use control_readulong() instead of control_readint()
 *
 * Revision 1.10  2004-05-23 22:17:50+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.9  2004-02-05 18:47:21+05:30  Cprogrammer
 * replaced qforward with envrules
 *
 * Revision 1.8  2003-12-15 13:50:06+05:30  Cprogrammer
 * renamed QMAILFILTER to SPAMFILTER
 *
 * Revision 1.7  2003-12-07 13:03:31+05:30  Cprogrammer
 * added spamignore and spamignorepatterns
 *
 * Revision 1.6  2003-10-23 01:23:41+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.5  2003-10-03 11:47:46+05:30  Cprogrammer
 * changed puts() to myputs() to avoid conflict in stdio.h
 * added qforward() code
 *
 * Revision 1.4  2002-10-08 20:35:06+05:30  Cprogrammer
 * added databytes control check
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <envdir.h>
#include <pathexec.h>
#include <sgetopt.h>
#include <sig.h>
#include <scan.h>
#include <substdio.h>
#include <stralloc.h>
#include <subfd.h>
#include <getln.h>
#include <alloc.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <error.h>
#include <now.h>
#include <constmap.h>
#include <case.h>
#include <byte.h>
#include <token822.h>
#include <noreturn.h>
#include "hfield.h"
#include "control.h"
#include "qmail.h"
#include "quote.h"
#include "headerbody.h"
#include "newfield.h"
#include "envrules.h"
#include "hassrs.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "set_environment.h"
#include "wildmat.h"

#define FATAL "qmail-inject: fatal: "
#define WARN  "qmail-inject: warn: "

#define LINELEN 80

datetime_sec    starttime;

static unsigned long   databytes = 0;
static unsigned long   size = 0;
static token822_alloc  envs = { 0 };
static struct qmail    qqt;
static char    *qmopts;
static char    *mailhost;
static char    *mailuser;
static char    *mailrhost;
static char    *mailruser;
static int      flagdeletesender = 0;
static int      flagdeletefrom = 0;
static int      flagdeletemessid = 0;
static int      flagnamecomment = 0;
static int      flaghackmess = 0;
static int      flaghackrecip = 0;
static int      mailusertokentype;
static int      rcptcount;
static int      maxrcptcount = -1;
static int      flagrh;
static int      flagqueue;
static int      flagresent;
static int      htypeseen[H_NUM];
static stralloc control_idhost = { 0 };
static stralloc control_defaultdomain = { 0 };
static stralloc control_defaulthost = { 0 };
static stralloc control_plusdomain = { 0 };
static stralloc sender = { 0 };
static stralloc envsbuf = { 0 };
static stralloc sauninit = { 0 };
static stralloc defaultdomainbuf = { 0 };
static stralloc defaulthostbuf = { 0 };
static stralloc plusdomainbuf = { 0 };
static stralloc hfbuf = { 0 };
static stralloc torecip = { 0 };
static stralloc defaultfrom = { 0 };
static stralloc defaultreturnpath = { 0 };
static stralloc hackedruser = { 0 };

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)

static saa      savedh = { 0 };
static saa      hrlist = { 0 };
static saa      tocclist = { 0 };
static saa      hrrlist = { 0 };
static saa      reciplist = { 0 };
static token822_alloc  defaultdomain = { 0 };
static token822_alloc  defaulthost = { 0 };
static token822_alloc  plusdomain = { 0 };
static token822_alloc  hfin = { 0 };
static token822_alloc  hfrewrite = { 0 };
static token822_alloc  hfaddr = { 0 };
static token822_alloc  tr = { 0 };
static token822_alloc  df = { 0 };
static token822_alloc  drp = { 0 };

void
put(char *s, int len)
{
	if (flagqueue) {
		qmail_put(&qqt, s, len);
		size += len;
		if (databytes && size > databytes)
			qmail_fail(&qqt);
	} else
		substdio_put(subfdout, s, len);
}

void
my_puts(char *s)
{
	put(s, str_len(s));
}

no_return void
perm()
{
	_exit(100);
}

no_return void
temp()
{
	_exit(111);
}

no_return void
die_rcpt()
{
	substdio_putsflush(subfderr, "qmail-inject: fatal: max number of recipients exceeded\n");
	perm();
}

no_return void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-inject: fatal: out of memory\n");
	temp();
}

#ifdef HAVESRS
no_return void die_srs()
{
	substdio_puts(subfderr, "qmail-inject: fatal: ");
	substdio_puts(subfderr, srs_error.s);
	substdio_putsflush(subfderr, "\n");
	perm();
}
#endif

no_return void
die_regex()
{
	substdio_putsflush(subfderr, "qmail-inject: fatal: regex compilation failed\n");
	temp();
}

no_return void
die_control(char *arg)
{
	substdio_puts(subfderr, "qmail-inject: fatal: unable to read control file ");
	substdio_puts(subfderr, arg);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	temp();
}

no_return void
die_invalid(stralloc *sa)
{
	substdio_putsflush(subfderr, "qmail-inject: fatal: invalid header field: ");
	substdio_putflush(subfderr, sa->s, sa->len);
	perm();
}

no_return void
die_qqt()
{
	substdio_putsflush(subfderr, "qmail-inject: fatal: unable to run qmail-queue\n");
	temp();
}

no_return void
die_read()
{
	if (errno == error_nomem)
		die_nomem();
	substdio_putsflush(subfderr, "qmail-inject: fatal: read error\n");
	temp();
}

no_return void
die_stat()
{
	char            strnum[FMT_ULONG];

	strnum[fmt_ulong(strnum, errno)] = 0;
	substdio_puts(subfderr, "qmail-inject: fatal: fstat error: errno ");
	substdio_puts(subfderr, strnum);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	temp();
}

no_return void
die_size()
{
	substdio_puts(subfderr, "qmail-inject: fatal: ");
	substdio_putsflush(subfderr, "message size exceeds my databytes limit\n");
	perm();
}

void
doordie(stralloc *sa, int r)
{
	if (r == 1)
		return;
	if (r == -1)
		die_nomem();
	substdio_putsflush(subfderr, "qmail-inject: fatal: unable to parse this line:\n");
	substdio_putflush(subfderr, sa->s, sa->len);
	perm();
}

no_return void
exitnicely()
{
	char           *qqx;

	if (!flagqueue)
		substdio_flush(subfdout);
	if (flagqueue) {
		int             i;

		if (!stralloc_0(&sender))
			die_nomem();
#ifdef HAVESRS
	if (!env_get("QMAILINJECT_SKIP_SRS") && 
		(env_get("QMAILINJECT_FORCE_SRS") || (env_get("EXT") && env_get("HOST")))) {
		switch(srsforward(sender.s))
		{
		case -3:
			die_srs();
			break;
		case -2:
			die_nomem();
			break;
		case -1:
			die_read();
			break;
		case 0:
			break;
		case 1:
			if (!stralloc_copy(&sender, &srs_result))
				die_nomem();
			break;
		}
	}
#endif
		qmail_from(&qqt, sender.s);
		for (i = 0; i < reciplist.len; ++i) {
			if (!stralloc_0(&reciplist.sa[i]))
				die_nomem();
			qmail_to(&qqt, reciplist.sa[i].s);
		}
		if (flagrh) {
			if (flagresent) {
				for (i = 0; i < hrrlist.len; ++i) {
					if (!stralloc_0(&hrrlist.sa[i]))
						die_nomem();
					qmail_to(&qqt, hrrlist.sa[i].s);
				}
			} else {
				for (i = 0; i < hrlist.len; ++i) {
					if (!stralloc_0(&hrlist.sa[i]))
						die_nomem();
					qmail_to(&qqt, hrlist.sa[i].s);
				}
			}
		}
		qqx = qmail_close(&qqt);
		if (*qqx) {
			if (databytes && size > databytes)
				die_size();
			substdio_puts(subfderr, "qmail-inject: fatal: ");
			substdio_puts(subfderr, qqx + 1);
			substdio_puts(subfderr, "\n");
			substdio_flush(subfderr);
			_exit(*qqx == 'D' ? 100 : 111);
		}
	}
	_exit(0);
}

void
savedh_append(stralloc *h)
{
	if (!saa_readyplus(&savedh, 1))
		die_nomem();
	savedh.sa[savedh.len] = sauninit;
	if (!stralloc_copy(savedh.sa + savedh.len, h))
		die_nomem();
	++savedh.len;
}

void
savedh_print()
{
	int             i;

	for (i = 0; i < savedh.len; ++i)
		put(savedh.sa[i].s, savedh.sa[i].len);
}

void
rwroute(token822_alloc *addr)
{
	if (addr->t[addr->len - 1].type == TOKEN822_AT) {
		while (addr->len) {
			if (addr->t[--addr->len].type == TOKEN822_COLON)
				return;
		}
	}
}

void
rwextraat(token822_alloc *addr)
{
	int             i;

	if (addr->t[0].type == TOKEN822_AT) {
		--addr->len;
		for (i = 0; i < addr->len; ++i)
			addr->t[i] = addr->t[i + 1];
	}
}

void
rwextradot(token822_alloc *addr)
{
	int             i;

	if (addr->t[0].type == TOKEN822_DOT) {
		--addr->len;
		for (i = 0; i < addr->len; ++i)
			addr->t[i] = addr->t[i + 1];
	}
}

void
rwnoat(token822_alloc *addr)
{
	int             i;
	int             shift;

	for (i = 0; i < addr->len; ++i) {
		if (addr->t[i].type == TOKEN822_AT)
			return;
	}
	shift = defaulthost.len;
	if (!token822_readyplus(addr, shift))
		die_nomem();
	for (i = addr->len - 1; i >= 0; --i)
		addr->t[i + shift] = addr->t[i];
	addr->len += shift;
	for (i = 0; i < shift; ++i)
		addr->t[i] = defaulthost.t[shift - 1 - i];
}

void
rwnodot(token822_alloc *addr)
{
	int             i;
	int             shift;

	for (i = 0; i < addr->len; ++i) {
		if (addr->t[i].type == TOKEN822_DOT)
			return;
		if (addr->t[i].type == TOKEN822_AT)
			break;
	}
	for (i = 0; i < addr->len; ++i) {
		if (addr->t[i].type == TOKEN822_LITERAL)
			return;
		if (addr->t[i].type == TOKEN822_AT)
			break;
	}
	shift = defaultdomain.len;
	if (!token822_readyplus(addr, shift))
		die_nomem();
	for (i = addr->len - 1; i >= 0; --i)
		addr->t[i + shift] = addr->t[i];
	addr->len += shift;
	for (i = 0; i < shift; ++i)
		addr->t[i] = defaultdomain.t[shift - 1 - i];
}

void
rwplus(token822_alloc *addr)
{
	int             i;
	int             shift;

	if (addr->t[0].type != TOKEN822_ATOM)
		return;
	if (!addr->t[0].slen)
		return;
	if (addr->t[0].s[addr->t[0].slen - 1] != '+')
		return;
	--addr->t[0].slen;	/*- remove + */
	shift = plusdomain.len;
	if (!token822_readyplus(addr, shift))
		die_nomem();
	for (i = addr->len - 1; i >= 0; --i)
		addr->t[i + shift] = addr->t[i];
	addr->len += shift;
	for (i = 0; i < shift; ++i)
		addr->t[i] = plusdomain.t[shift - 1 - i];
}

void
rwgeneric(token822_alloc *addr)
{
	if (!addr->len)
		return;	/*- don't rewrite <> */
	if (addr->len >= 2) {
		if (addr->t[1].type == TOKEN822_AT) {
			if (addr->t[0].type == TOKEN822_LITERAL) {
				if (!addr->t[0].slen)	/*- don't rewrite <foo@[]> */
					return;
			}
		}
	}
	rwroute(addr);
	if (!addr->len)
		return;	/*- <@foo:> -> <> */
	rwextradot(addr);
	if (!addr->len)
		return;	/*- <.> -> <> */
	rwextraat(addr);
	if (!addr->len)
		return;	/*- <@> -> <> */
	rwnoat(addr);
	rwplus(addr);
	rwnodot(addr);
}

int
setreturn(token822_alloc *addr)
{
	if (!sender.s) {
		token822_reverse(addr);
		if (token822_unquote(&sender, addr) != 1)
			die_nomem();
		if (flaghackrecip)
			if (!stralloc_cats(&sender, "-@[]"))
				die_nomem();
		token822_reverse(addr);
	}
	return 1;
}

int
rwreturn(token822_alloc *addr)
{
	rwgeneric(addr);
	setreturn(addr);
	return 1;
}

int
rwsender(token822_alloc *addr)
{
	rwgeneric(addr);
	return 1;
}

void
rwappend(token822_alloc *addr, saa *xl)
{
	token822_reverse(addr);
	if (!saa_readyplus(xl, 1))
		die_nomem();
	xl->sa[xl->len] = sauninit;
	if (token822_unquote(&xl->sa[xl->len], addr) != 1)
		die_nomem();
	++xl->len;
	token822_reverse(addr);
}

int
rwhrr(token822_alloc *addr)
{
	rwgeneric(addr);
	rwappend(addr, &hrrlist);
	return 1;
}

int
rwhr(token822_alloc *addr)
{
	rwgeneric(addr);
	rwappend(addr, &hrlist);
	return 1;
}

int
rwtocc(token822_alloc *addr)
{
	rwgeneric(addr);
	rwappend(addr, &hrlist);
	rwappend(addr, &tocclist);
	return 1;
}

void
doheaderfield(stralloc *h)
{
	int             htype;
	int             (*rw) () = 0;

	htype = hfield_known(h->s, h->len);
	if (flagdeletefrom && htype == H_FROM)
		return;
	if (flagdeletemessid && htype == H_MESSAGEID)
		return;
	if (flagdeletesender && htype == H_RETURNPATH)
		return;
	if (htype)
		htypeseen[htype] = 1;
	else
	if (!hfield_valid(h->s, h->len))
		die_invalid(h);
	switch (htype)
	{
	case H_TO:
	case H_CC:
		if (flagrh)
			rw = rwtocc;
		break;
	case H_BCC:
	case H_APPARENTLYTO:
		if (flagrh)
			rw = rwhr;
		break;
	case H_R_TO:
	case H_R_CC:
	case H_R_BCC:
		if (flagrh)
			rw = rwhrr;
		break;
	case H_RETURNPATH:
		rw = rwreturn;
		break;
	case H_SENDER:
	case H_FROM:
	case H_REPLYTO:
	case H_RETURNRECEIPTTO:
	case H_ERRORSTO:
	case H_R_SENDER:
	case H_R_FROM:
	case H_R_REPLYTO:
		rw = rwsender;
		break;
	}
	if (rw) {
		doordie(h, token822_parse(&hfin, h, &hfbuf));
		doordie(h, token822_addrlist(&hfrewrite, &hfaddr, &hfin, rw));
		if (token822_unparse(h, &hfrewrite, LINELEN) != 1)
			die_nomem();
	}
	if (htype == H_BCC)
		return;
	if (htype == H_R_BCC)
		return;
	if (htype == H_RETURNPATH)
		return;
	if (htype == H_CONTENTLENGTH)
		return;	/*- some things are just too stupid */
	savedh_append(h);
}

void
dobody(stralloc *h)
{
	put(h->s, h->len);
}

void
dorecip(char *s)
{
	if (!quote2(&torecip, s))
		die_nomem();
	switch (token822_parse(&tr, &torecip, &hfbuf))
	{
	case -1:
		die_nomem();
	case 0:
		substdio_puts(subfderr, "qmail-inject: fatal: unable to parse address: ");
		substdio_puts(subfderr, s);
		substdio_putsflush(subfderr, "\n");
		perm();
	}
	rcptcount++;
	if (maxrcptcount > 0 && rcptcount > maxrcptcount)
		die_rcpt();
	token822_reverse(&tr);
	rwgeneric(&tr);
	rwappend(&tr, &reciplist);
}

void
defaultfrommake()
{
	char           *fullname;

	if (!(fullname = env_get("QMAILNAME")))
		fullname = env_get("MAILNAME");
	if (!fullname)
		fullname = env_get("NAME");
	if (!token822_ready(&df, 20))
		die_nomem();
	df.len = 0;
	df.t[df.len].type = TOKEN822_ATOM;
	df.t[df.len].s = "From";
	df.t[df.len].slen = 4;
	++df.len;
	df.t[df.len].type = TOKEN822_COLON;
	++df.len;
	if (fullname && !flagnamecomment) {
		df.t[df.len].type = TOKEN822_QUOTE;
		df.t[df.len].s = fullname;
		df.t[df.len].slen = str_len(fullname);
		++df.len;
		df.t[df.len].type = TOKEN822_LEFT;
		++df.len;
	}
	df.t[df.len].type = mailusertokentype;
	df.t[df.len].s = mailuser;
	df.t[df.len].slen = str_len(mailuser);
	++df.len;
	if (mailhost) {
		df.t[df.len].type = TOKEN822_AT;
		++df.len;
		df.t[df.len].type = TOKEN822_ATOM;
		df.t[df.len].s = mailhost;
		df.t[df.len].slen = str_len(mailhost);
		++df.len;
	}
	if (fullname && !flagnamecomment) {
		df.t[df.len].type = TOKEN822_RIGHT;
		++df.len;
	}
	if (fullname && flagnamecomment) {
		df.t[df.len].type = TOKEN822_COMMENT;
		df.t[df.len].s = fullname;
		df.t[df.len].slen = str_len(fullname);
		++df.len;
	}
	if (token822_unparse(&defaultfrom, &df, LINELEN) != 1)
		die_nomem();
	doordie(&defaultfrom, token822_parse(&df, &defaultfrom, &hfbuf));
	doordie(&defaultfrom, token822_addrlist(&hfrewrite, &hfaddr, &df, rwsender));
	if (token822_unparse(&defaultfrom, &hfrewrite, LINELEN) != 1)
		die_nomem();
}

void
dodefaultreturnpath()
{
	char            strnum[FMT_ULONG];

	if (!stralloc_copys(&hackedruser, mailruser))
		die_nomem();
	if (flaghackmess) {
		if (!stralloc_cats(&hackedruser, "-"))
			die_nomem();
		if (!stralloc_catb(&hackedruser, strnum, fmt_ulong(strnum, (unsigned long) starttime)))
			die_nomem();
		if (!stralloc_cats(&hackedruser, "."))
			die_nomem();
		if (!stralloc_catb(&hackedruser, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
			die_nomem();
	}
	if (flaghackrecip && !stralloc_cats(&hackedruser, "-"))
		die_nomem();
	if (!token822_ready(&drp, 10))
		die_nomem();
	drp.len = 0;
	drp.t[drp.len].type = TOKEN822_ATOM;
	drp.t[drp.len].s = "Return-Path";
	drp.t[drp.len].slen = 11;
	++drp.len;
	drp.t[drp.len].type = TOKEN822_COLON;
	++drp.len;
	drp.t[drp.len].type = TOKEN822_QUOTE;
	drp.t[drp.len].s = hackedruser.s;
	drp.t[drp.len].slen = hackedruser.len;
	++drp.len;
	if (mailrhost) {
		drp.t[drp.len].type = TOKEN822_AT;
		++drp.len;
		drp.t[drp.len].type = TOKEN822_ATOM;
		drp.t[drp.len].s = mailrhost;
		drp.t[drp.len].slen = str_len(mailrhost);
		++drp.len;
	}
	if (token822_unparse(&defaultreturnpath, &drp, LINELEN) != 1)
		die_nomem();
	doordie(&defaultreturnpath, token822_parse(&drp, &defaultreturnpath, &hfbuf));
	doordie(&defaultreturnpath, token822_addrlist(&hfrewrite, &hfaddr, &drp, rwreturn));
	if (token822_unparse(&defaultreturnpath, &hfrewrite, LINELEN) != 1)
		die_nomem();
}

int             flagmft = 0;
stralloc        mft = { 0 };
struct constmap mapmft;

void
mft_init()
{
	char           *x;
	int             r;

	if (!(x = env_get("QMAILMFTFILE")))
		return;
	if ((r = control_readfile(&mft, x, 0)) == -1)
		die_read(); /*XXX*/
	if (!r)
		return;
	if (!constmap_init(&mapmft, mft.s, mft.len, 0))
		die_nomem();
	flagmft = 1;
}

void
finishmft()
{
	int             i;
	static stralloc sa = { 0 };
	static stralloc sa2 = { 0 };

	if (!flagmft)
		return;
	if (htypeseen[H_MAILFOLLOWUPTO])
		return;
	for (i = 0; i < tocclist.len; ++i) {
		if (constmap(&mapmft, tocclist.sa[i].s, tocclist.sa[i].len))
			break;
	}
	if (i == tocclist.len)
		return;
	my_puts("Mail-Followup-To: ");
	i = tocclist.len;
	while (i--) {
		if (!stralloc_copy(&sa, &tocclist.sa[i]))
			die_nomem();
		if (!stralloc_0(&sa))
			die_nomem();
		if (!quote2(&sa2, sa.s))
			die_nomem();
		put(sa2.s, sa2.len);
		if (i)
			my_puts(",\n  ");
	}
	my_puts("\n");
}

void
finishheader()
{
	int             ret;

	flagresent = htypeseen[H_R_SENDER] || htypeseen[H_R_FROM] || htypeseen[H_R_REPLYTO] || htypeseen[H_R_TO] || htypeseen[H_R_CC] ||
		htypeseen[H_R_BCC] || htypeseen[H_R_DATE] || htypeseen[H_R_MESSAGEID];
	if (!sender.s)
		dodefaultreturnpath();
	if (!flagqueue) {
		static stralloc sa = { 0 };
		static stralloc sa2 = { 0 };

		if (!stralloc_copy(&sa, &sender))
			die_nomem();
		if (!stralloc_0(&sa))
			die_nomem();
		if (!quote2(&sa2, sa.s))
			die_nomem();
		my_puts("Return-Path: <");
		put(sa2.s, sa2.len);
		my_puts(">\n");
	}
	/*
	 * could check at this point whether there are any recipients 
	 */
	if (flagqueue) {
		static stralloc satmp = { 0 };
	
		if (!stralloc_copy(&satmp, &sender))
			die_nomem();
		if (!stralloc_0(&satmp))
			die_nomem();
		if ((ret = envrules(satmp.s, "from.envrules", "FROMRULES", 0)) == -1)
			die_nomem();
		else
		if (ret == -2)
			die_control("from.envrules");
		else
		if (ret == -4)
			die_regex();
		if ((ret = domainqueue(satmp.s, "domainqueue", "DOMAINQUEUE", 0)) == -1)
			die_nomem();
		else
		if (ret == -2)
			die_control("domainqueue");
		else
		if (ret == -4)
			die_regex();
		if (qmail_open(&qqt) == -1)
			die_qqt();
	} /*- if (flagqueue) */
	if (flagresent) {
		if (!htypeseen[H_R_DATE]) {
			if (!newfield_datemake(starttime))
				die_nomem();
			my_puts("Resent-");
			put(newfield_date.s, newfield_date.len);
		}
		if (!htypeseen[H_R_MESSAGEID]) {
			if (!newfield_msgidmake(control_idhost.s, control_idhost.len, starttime))
				die_nomem();
			my_puts("Resent-");
			put(newfield_msgid.s, newfield_msgid.len);
		}
		if (!htypeseen[H_R_FROM]) {
			defaultfrommake();
			my_puts("Resent-");
			put(defaultfrom.s, defaultfrom.len);
		}
		if (!htypeseen[H_R_TO] && !htypeseen[H_R_CC])
			my_puts("Resent-Cc: recipient list not shown: ;\n");
	} else {
		if (!htypeseen[H_DATE]) {
			if (!newfield_datemake(starttime))
				die_nomem();
			put(newfield_date.s, newfield_date.len);
		}
		if (!htypeseen[H_MESSAGEID]) {
			if (!newfield_msgidmake(control_idhost.s, control_idhost.len, starttime))
				die_nomem();
			put(newfield_msgid.s, newfield_msgid.len);
		}
		if (!htypeseen[H_FROM]) {
			defaultfrommake();
			put(defaultfrom.s, defaultfrom.len);
		}
		if (!htypeseen[H_TO] && !htypeseen[H_CC])
			my_puts("Cc: recipient list not shown: ;\n");
		finishmft();
	}
	savedh_print();
}

void
getcontrols()
{
	struct stat     statbuf;
	static stralloc sa = { 0 };
	char           *x;

	mft_init();
	set_environment(WARN, FATAL, 1);
	if (!(x = env_get("QMAILDEFAULTDOMAIN"))) {
		if (control_rldef(&control_defaultdomain, "defaultdomain", 1, "defaultdomain") != 1)
			die_read();
	} else
	if (!stralloc_copys(&control_defaultdomain, x))
		die_nomem();
	if (!stralloc_copys(&sa, "."))
		die_nomem();
	if (!stralloc_cat(&sa, &control_defaultdomain))
		die_nomem();
	doordie(&sa, token822_parse(&defaultdomain, &sa, &defaultdomainbuf));
	if (!(x = env_get("QMAILDEFAULTHOST"))) {
		if (control_rldef(&control_defaulthost, "defaulthost", 1, "defaulthost") != 1)
			die_read();
	} else
	if (!stralloc_copys(&control_defaulthost, x))
		die_nomem();
	if (!stralloc_copys(&sa, "@"))
		die_nomem();
	if (!stralloc_cat(&sa, &control_defaulthost))
		die_nomem();
	doordie(&sa, token822_parse(&defaulthost, &sa, &defaulthostbuf));
	if (!(x = env_get("QMAILPLUSDOMAIN"))) {
		if (control_rldef(&control_plusdomain, "plusdomain", 1, "plusdomain") != 1)
			die_read();
	} else
	if (!stralloc_copys(&control_plusdomain, x))
		die_nomem();
	if (!stralloc_copys(&sa, "."))
		die_nomem();
	if (!stralloc_cat(&sa, &control_plusdomain))
		die_nomem();
	doordie(&sa, token822_parse(&plusdomain, &sa, &plusdomainbuf));
	if (!(x = env_get("QMAILIDHOST"))) {
		if (control_rldef(&control_idhost, "idhost", 1, "idhost") != 1)
			die_read();
	} else
	if (!stralloc_copys(&control_idhost, x))
		die_nomem();
	if (!(x = env_get("DATABYTES"))) {
		if (control_readulong(&databytes, "databytes") == -1)
			die_read();
	} else
		scan_ulong(x, &databytes);
	if (!(databytes + 1))
		--databytes;
	if (!(x = env_get("MAXRECIPIENTS"))) {
		if (control_readint(&maxrcptcount,"maxrecipients") == -1)
			die_read();
		if (maxrcptcount < 0)
			maxrcptcount = 0;
	} else
		scan_int(x, &maxrcptcount);
	if (fstat(0, &statbuf))
		die_stat();
	if (databytes && statbuf.st_size > databytes)
		die_size();
}

#define RECIP_DEFAULT 1
#define RECIP_ARGS 2
#define RECIP_HEADER 3
#define RECIP_AH 4

int
main(int argc, char **argv)
{
	int             i;
	int             opt;
	int             recipstrategy;

	sig_pipeignore();
	starttime = now();
	qmopts = env_get("QMAILINJECT");
	flagqueue = 1;
	if (qmopts) {
		while (*qmopts) {
			switch (*qmopts++)
			{
			case 'c':
				flagnamecomment = 1;
				break;
			case 's':
				flagdeletesender = 1;
				break;
			case 'f':
				flagdeletefrom = 1;
				break;
			case 'i':
				flagdeletemessid = 1;
				break;
			case 'r':
				flaghackrecip = 1;
				break;
			case 'm':
				flaghackmess = 1;
				break;
			case 'n':
				flagqueue = 0;
				break;
			}
		}
	}
	if (!(mailhost = env_get("QMAILHOST")))
		mailhost = env_get("MAILHOST");
	if (!(mailrhost = env_get("QMAILSHOST")))
		mailrhost = mailhost;
	if (!(mailuser = env_get("QMAILUSER")))
		mailuser = env_get("MAILUSER");
	if (!mailuser)
		mailuser = env_get("USER");
	if (!mailuser)
		mailuser = env_get("LOGNAME");
	if (!mailuser)
		mailuser = "anonymous";
	mailusertokentype = TOKEN822_ATOM;
	if (quote_need(mailuser, str_len(mailuser)))
		mailusertokentype = TOKEN822_QUOTE;
	if (!(mailruser = env_get("QMAILSUSER")))
		mailruser = mailuser;
	for (i = 0; i < H_NUM; ++i)
		htypeseen[i] = 0;
	recipstrategy = RECIP_DEFAULT;
	getcontrols();
	if (!saa_readyplus(&hrlist, 1))
		die_nomem();
	if (!saa_readyplus(&tocclist, 1))
		die_nomem();
	if (!saa_readyplus(&hrrlist, 1))
		die_nomem();
	if (!saa_readyplus(&reciplist, 1))
		die_nomem();
	rcptcount = 0;
	while ((opt = getopt(argc, argv, "asAhHnNf:")) != opteof) {
		switch (opt)
		{
		case 'a':
			recipstrategy = RECIP_ARGS;
			break;
		case 's':
			flagdeletesender = 1;
			break;
		case 'A':
			recipstrategy = RECIP_DEFAULT;
			break;
		case 'h':
			recipstrategy = RECIP_HEADER;
			break;
		case 'H':
			recipstrategy = RECIP_AH;
			break;
		case 'n':
			flagqueue = 0;
			break;
		case 'N':
			flagqueue = 1;
			break;
		case 'f':
			if (!quote2(&sender, optarg))
				die_nomem();
			doordie(&sender, token822_parse(&envs, &sender, &envsbuf));
			token822_reverse(&envs);
			rwgeneric(&envs);
			token822_reverse(&envs);
			if (token822_unquote(&sender, &envs) != 1)
				die_nomem();
			break;
		case '?':
		default:
			perm();
		}
	}
	argc -= optind;
	argv += optind;
	if (recipstrategy == RECIP_DEFAULT)
		recipstrategy = (*argv ? RECIP_ARGS : RECIP_HEADER);
	if (recipstrategy != RECIP_HEADER) {
		while (*argv)
			dorecip(*argv++);
	}
	flagrh = (recipstrategy != RECIP_ARGS);
	if (headerbody(subfdin, doheaderfield, finishheader, dobody) == -1)
		die_read();
	exitnicely();
	/*- never comes here - just added for compiler warnings */
	return(0);
}

void
getversion_qmail_inject_c()
{
	static char    *x = "$Id: qmail-inject.c,v 1.44 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x++;
}
