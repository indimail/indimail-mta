/*
 * $Log: new-inject.c,v $
 * Revision 1.11  2020-04-30 18:09:45+05:30  Cprogrammer
 * define rwhconfig_err variable locally
 *
 * Revision 1.10  2020-04-04 11:30:55+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.9  2016-05-21 14:48:09+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.8  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.7  2016-01-28 09:01:02+05:30  Cprogrammer
 * chdir to qmail_home before leapsecs_init()
 *
 * Revision 1.6  2010-06-08 21:59:42+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.5  2009-08-13 19:05:56+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.4  2005-08-23 17:33:21+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 20:27:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-11 23:53:48+05:30  Cprogrammer
 * include sgetopt.h after unistd.h
 *
 * Revision 1.1  2004-06-16 01:19:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "auto_sysconfdir.h"
#include "auto_control.h"
#include "env.h"
#include "envdir.h"
#include "pathexec.h"
#include "sgetopt.h"
#include "substdio.h"
#include "subfd.h"
#include "str.h"
#include "getln.h"
#include "mess822.h"
#include "strerr.h"
#include "exit.h"
#include "caltime.h"
#include "leapsecs.h"
#include "tai.h"
#include "stralloc.h"
#include "sconfig.h"
#include "case.h"
#include "constmap.h"
#include "qmail.h"
#include "sig.h"
#include "rewritehost.h"
#include "rwhconfig.h"
#include "variables.h"

#define FATAL "new-inject: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

config_str      qmailinject = CONFIG_STR;
config_str      name = CONFIG_STR;
config_str      user = CONFIG_STR;
config_str      host = CONFIG_STR;
config_str      suser = CONFIG_STR;
config_str      shost = CONFIG_STR;
config_str      fnmft = CONFIG_STR;
config_str      mft = CONFIG_STR;
struct constmap mapmft;
config_str      fnrewrite = CONFIG_STR;
config_str      rewrite = CONFIG_STR;
stralloc        idappend = { 0 };
struct tai      start;
stralloc        tmp = { 0 };
stralloc        tmp2 = { 0 };

void
rewritelist(list)
	stralloc       *list;
{
	if (!rewritehost_list(&tmp, list->s, list->len, config_data(&rewrite)))
		nomem();
	if (!stralloc_copy(list, &tmp))
		nomem();
}

int             recipstrategy = 'A';
int             flagkillfrom = 0;
int             flagkillmsgid = 0;
int             flagkillreturnpath = 0;
int             flagverpmess = 0;
int             flagverprecip = 0;
stralloc        sender = { 0 };
stralloc        recipients = { 0 };

void
recipient_add(addr)
	char           *addr;
{
	if (!rewritehost_addr(&tmp, addr, str_len(addr), config_data(&rewrite)))
		nomem();
	if (!stralloc_cat(&recipients, &tmp))
		nomem();
	if (!stralloc_0(&recipients))
		nomem();
}

void
recipient_addlist(list)
	stralloc       *list;
{
	int             i;
	int             j;

	for (j = i = 0; j < list->len; ++j) {
		if (!list->s[j]) {
			if (list->s[i] == '+' && !stralloc_catb(&recipients, list->s + i + 1, j - i))
				nomem();
			i = j + 1;
		}
	}
}

void
sender_get(list)
	stralloc       *list;
{
	int             i;
	int             j;

	for (j = i = 0; j < list->len; ++j) {
		if (!list->s[j]) {
			if (list->s[i] == '+' && !stralloc_copyb(&sender, list->s + i + 1, j - i))
				nomem();
			i = j + 1;
		}
	}
}

int             flagqueue = 1;
struct qmail    qq;

void
myput(buf, len)
	char           *buf;
	int             len;
{
	if (flagqueue)
		qmail_put(&qq, buf, len);
	else
		substdio_put(subfdoutsmall, buf, len);
}

void
myputs(buf)
	char           *buf;
{
	myput(buf, str_len(buf));
}

void
putlist(name, list)
	char           *name;
	stralloc       *list;
{
	if (!list->len)
		return;
	if (!mess822_quotelist(&tmp, list))
		nomem();
	if (!mess822_fold(&tmp2, &tmp, name, 78))
		nomem();
	myput(tmp2.s, tmp2.len);
}

mess822_time    date;
stralloc        enveloperecipients = { 0 };
int             flagenveloperecipients;
stralloc        envelopesender = { 0 };
int             flagenvelopesender;
stralloc        to = { 0 };
stralloc        cc = { 0 };
stralloc        bcc = { 0 };
stralloc        nrudt = { 0 };
stralloc        returnpath = { 0 };
stralloc        from = { 0 };
stralloc        headersender = { 0 };
stralloc        replyto = { 0 };
stralloc        mailreplyto = { 0 };
stralloc        followupto = { 0 };
stralloc        msgid = { 0 };
stralloc        top = { 0 };
stralloc        bottom = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"date", 0, 0, 0, 0, &date}
	, {"to", 0, 0, 0, &to, 0}
	, {"cc", 0, 0, 0, &cc, 0}
	, {"bcc", 0, 0, 0, &bcc, 0}
	, {"apparently-to", 0, 0, 0, &bcc, 0}
	, {"notice-requested-upon-delivery-to", 0, 0, 0, &nrudt, 0}
	, {"from", 0, 0, 0, &from, 0}
	, {"sender", 0, 0, 0, &headersender, 0}
	, {"reply-to", 0, 0, 0, &replyto, 0}
	, {"mail-reply-to", 0, 0, 0, &mailreplyto, 0}
	, {"mail-followup-to", 0, 0, 0, &followupto, 0}
	, {"return-path", 0, 0, 0, &returnpath, 0}
	, {"envelope-recipients", &flagenveloperecipients, 0, 0, &enveloperecipients, 0}
	, {"envelope-sender", &flagenvelopesender, 0, 0, &envelopesender, 0}
	, {"message-id", 0, &msgid, 0, 0, 0}
	, {"received", 0, &top, 0, 0, 0}
	, {"delivered-to", 0, &top, 0, 0, 0}
	, {"errors-to", 0, &top, 0, 0, 0}
	, {"return-receipt-to", 0, &top, 0, 0, 0}
	, {"resent-sender", 0, &top, 0, 0, 0}
	, {"resent-from", 0, &top, 0, 0, 0}
	, {"resent-reply-to", 0, &top, 0, 0, 0}
	, {"resent-to", 0, &top, 0, 0, 0}
	, {"resent-cc", 0, &top, 0, 0, 0}
	, {"resent-bcc", 0, &top, 0, 0, 0}
	, {"resent-date", 0, &top, 0, 0, 0}
	, {"resent-message-id", 0, &top, 0, 0, 0}
	, {"content-length", 0, 0, 0, 0, 0}
	, {0, 0, &bottom, 0, 0, 0}
};

void
envelope_make()
{
	if (recipstrategy != 'a') {
		if (flagenveloperecipients)
			recipient_addlist(&enveloperecipients);
		else {
			recipient_addlist(&to);
			recipient_addlist(&cc);
			recipient_addlist(&bcc);
		}
	}
	if (!sender.len)
		sender_get(&envelopesender);
	if (!sender.len)
		sender_get(&returnpath);
	if (!sender.len) {
		if (!rewritehost_addr(&sender, config_data(&suser)->s, config_data(&suser)->len, config_data(&rewrite)))
			nomem();
		if (flagverprecip && !stralloc_cats(&sender, "-@[]"))
			nomem();
		if (!stralloc_0(&sender))
			nomem();
	}
}

void
envelope_print()
{
	int             i;
	int             j;

	myputs("Envelope-Sender: ");
	if (!mess822_quote(&tmp, sender.s, (char *) 0))
		nomem();
	myput(tmp.s, tmp.len);
	myputs("\n");
	myputs("Envelope-Recipients:\n");
	for (j = i = 0; j < recipients.len; ++j) {
		if (!recipients.s[j]) {
			if (!mess822_quote(&tmp, recipients.s + i, (char *) 0))
				nomem();
			myputs("  ");
			myput(tmp.s, tmp.len);
			myputs(",\n");
			i = j + 1;
		}
	}
}

void
datemsgid_print()
{
	if (!date.known) {
		caltime_utc(&date.ct, &start, (int *) 0, (int *) 0);
		date.known = 1;
	}
	if (!mess822_date(&tmp, &date))
		nomem();
	myputs("Date: ");
	myput(tmp.s, tmp.len);
	myputs("\n");
	if (!msgid.len) {
		if (!stralloc_copys(&msgid, "Message-ID: <"))
			nomem();
		if (!stralloc_catlong(&msgid, date.ct.date.year))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.date.month, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.date.day, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.hour, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.minute, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.second, 2))
			nomem();
		if (!stralloc_cat(&msgid, &idappend))
			nomem();
		if (!stralloc_cats(&msgid, ">\n"))
			nomem();
	}
	myput(msgid.s, msgid.len);
}

void
from_print()
{
	putlist("From: ", &from);
	if (!from.len) {
		if (!rewritehost_addr(&tmp2, config_data(&user)->s, config_data(&user)->len, config_data(&rewrite)))
			nomem();
		if (!stralloc_0(&tmp2))
			nomem();
		myputs("From: ");
		if (!mess822_quote(&tmp, tmp2.s, config(&name) ? config_data(&name)->s : (char *) 0))
			nomem();
		myput(tmp.s, tmp.len);
		myputs("\n");
	}
	putlist("Sender: ", &headersender);
}

int
inmft(list)
	stralloc       *list;
{
	int             i;
	int             j;

	for (j = i = 0; j < list->len; ++j) {
		if (!list->s[j]) {
			if (list->s[i] == '+' && constmap(&mapmft, list->s + i + 1, j - i - 1))
				return 1;
			i = j + 1;
		}
	}
	return 0;
}

void
response_print()
{
	putlist("Reply-To: ", &replyto);
	putlist("Mail-Reply-To: ", &mailreplyto);

	if (!followupto.len && config(&mft) && (inmft(&to) || inmft(&cc))) {
		if (!stralloc_cat(&followupto, &to))
			nomem();
		if (!stralloc_cat(&followupto, &cc))
			nomem();
	}
	putlist("Mail-Followup-To: ", &followupto);
}

void
to_print()
{
	if (!to.len && !cc.len)
		myputs("Cc: recipient list not shown: ;\n");
	putlist("To: ", &to);
	putlist("Cc: ", &cc);
	putlist("Notice-Requested-Upon-Delivery-To: ", &nrudt);
}

void
finishheader()
{
	if (!mess822_end(&h))
		nomem();
	if (flagqueue && qmail_open(&qq) == -1)
		strerr_die2sys(111, FATAL, "unable to run qmail-queue: ");
	if (flagkillreturnpath)
		returnpath.len = 0;
	if (flagkillmsgid)
		msgid.len = 0;
	if (flagkillfrom)
		from.len = 0;
	rewritelist(&to);
	rewritelist(&cc);
	rewritelist(&bcc);
	rewritelist(&nrudt);
	rewritelist(&from);
	rewritelist(&headersender);
	rewritelist(&replyto);
	rewritelist(&mailreplyto);
	rewritelist(&followupto);
	rewritelist(&returnpath);
	rewritelist(&enveloperecipients);
	rewritelist(&envelopesender);
	envelope_make();
	if (!flagqueue)
		envelope_print();
	myput(top.s, top.len);
	datemsgid_print();
	from_print();
	response_print();
	to_print();
	myput(bottom.s, bottom.len);
}

void
finishmessage()
{
	char           *qqx;
	int             i;
	int             j;

	if (!flagqueue)
		substdio_flush(subfdoutsmall);
	else {
		qmail_from(&qq, sender.s);
		for (j = i = 0; j < recipients.len; ++j) {
			if (!recipients.s[j]) {
				qmail_to(&qq, recipients.s + i);
				i = j + 1;
			}
		}
		qqx = qmail_close(&qq);
		if (*qqx)
			strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	}
	_exit(0);
}

char           *argsender = 0;
stralloc        line = { 0 };
int             match;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             i, opt, flagheader = 1;
	char           *qbase, *home;
	char          **e;
	struct strerr   rwhconfig_err;

	sig_pipeignore();
	tai_now(&start);
	if (config_env(&qmailinject, "QMAILINJECT") == -1)
		nomem();
	if (config(&qmailinject)) {
		for (i = 0; i < config_data(&qmailinject)->len; ++i) {
			switch (config_data(&qmailinject)->s[i])
			{
			case 'F':
			case 'f':
				flagkillfrom = 1;
				break;
			case 'I':
			case 'i':
				flagkillmsgid = 1;
				break;
			case 'M':
			case 'm':
				flagverpmess = 1;
				break;
			case 'R':
			case 'r':
				flagverprecip = 1;
				break;
			case 'S':
			case 's':
				flagkillreturnpath = 1;
				break;
			}
		}
	}
	while ((opt = getopt(argc, argv, "nNaAhHFIMRSf:")) != opteof) {
		switch (opt)
		{
		case 'n':
			flagqueue = 0;
			break;
		case 'N':
			flagqueue = 1;
			break;
		case 'a':
		case 'A':
		case 'h':
		case 'H':
			recipstrategy = opt;
			break;
		case 'F':
			flagkillfrom = 1;
			break;
		case 'I':
			flagkillmsgid = 1;
			break;
		case 'M':
			flagverpmess = 1;
			break;
		case 'R':
			flagverprecip = 1;
			break;
		case 'S':
			flagkillreturnpath = 1;
			break;
		case 'f':
			argsender = optarg;
			break;
		default:
			strerr_die1x(100, "new-inject: usage: new-inject [ -nNaAhHFIMRS ] [ -f sender ] [ recip ... ]\n");
		}
	} /*- while ((opt = getopt(argc, argv, "nNaAhHFIMRSf:")) != opteof) */
	argv += optind;
	if (chdir(auto_sysconfdir) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", auto_sysconfdir, ": ");
	if (leapsecs_init() == -1)
		strerr_die2sys(111, FATAL, "unable to init leapsecs: ");
	if (config_env(&fnmft, "QMAILMFTFILE") == -1)
		nomem();
	if (config(&fnmft)) {
		if (!stralloc_0(config_data(&fnmft)))
			nomem();
		if (config_readfile(&mft, config_data(&fnmft)->s) == -1)
			strerr_die2sys(111, FATAL, "unable to read $QMAILMFTFILE: ");
		if (config(&mft))
			if (!constmap_init(&mapmft, config_data(&mft)->s, config_data(&mft)->len, 0))
				nomem();
	}
	if (config_env(&fnrewrite, "QMAILREWRITEFILE") == -1)
		nomem();
	if (config(&fnrewrite)) {
		if (!stralloc_0(config_data(&fnrewrite)))
			nomem();
		if (config_readfile(&rewrite, config_data(&fnrewrite)->s) == -1)
			strerr_die2sys(111, FATAL, "unable to read $QMAILREWRITEFILE: ");
	}
	if ((home = env_get("HOME"))) {
		if (chdir(home) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
		if (!access(".defaultqueue", X_OK)) {
			envdir_set(".defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		} else
			home = (char *) 0;
	}
	if (chdir(auto_sysconfdir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_sysconfdir, ": ");
	if (!(qbase = env_get("QUEUE_BASE"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (chdir(controldir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", controldir, ": ");
		if (!access("defaultqueue", X_OK)) {
			envdir_set("defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		}
		if (chdir(auto_sysconfdir) == -1)
			strerr_die4sys(111, FATAL, "unable to chdir to ", auto_sysconfdir, ": ");
	}
	if (config_env(&name, "QMAILNAME") == -1)
		nomem();
	if (config_env(&name, "MAILNAME") == -1)
		nomem();
	if (config_env(&name, "NAME") == -1)
		nomem();
	if (config(&name) && !stralloc_0(config_data(&name)))
		nomem();
	if (config_env(&user, "QMAILUSER") == -1)
		nomem();
	if (config_env(&user, "MAILUSER") == -1)
		nomem();
	if (config_env(&user, "USER") == -1)
		nomem();
	if (config_env(&user, "LOGNAME") == -1)
		nomem();
	if (config_default(&user, "anonymous") == -1)
		nomem();
	if (config_env(&suser, "QMAILSUSER") == -1)
		nomem();
	if (config_copy(&suser, &user) == -1)
		nomem();
	if (config_env(&host, "QMAILHOST") == -1)
		nomem();
	if (config_env(&host, "MAILHOST") == -1)
		nomem();
	if (config_default(&host, "") == -1)
		nomem();
	if (rwhconfig(&rewrite, &idappend) == -1)
		strerr_die1(111, FATAL, &rwhconfig_err);
	if (config_env(&shost, "QMAILSHOST") == -1)
		nomem();
	if (config_copy(&shost, &host) == -1)
		nomem();
	if (!stralloc_copys(&sender, ""))
		nomem();
	if (argsender) {
		if (!rewritehost_addr(&sender, argsender, str_len(argsender), config_data(&rewrite)))
			nomem();
		if (!stralloc_0(&sender))
			nomem();
	}
	if (recipstrategy == 'A')
		recipstrategy = (*argv ? 'a' : 'h');
	if (!stralloc_copys(&recipients, ""))
		nomem();
	if (recipstrategy != 'h') {
		while (*argv)
			recipient_add(*argv++);
	}
	if (flagverpmess) {
		unsigned char   nowpack[TAI_PACK];
		unsigned long   u;

		tai_pack((char *) nowpack, &start);
		u = nowpack[4];
		u <<= 8;
		u += nowpack[5];
		u <<= 8;
		u += nowpack[6];
		u <<= 8;
		u += nowpack[7];
		if (!stralloc_cats(config_data(&suser), "-"))
			nomem();
		if (!stralloc_catulong0(config_data(&suser), u, 0))
			nomem();
		u = getpid();
		if (!stralloc_cats(config_data(&suser), "."))
			nomem();
		if (!stralloc_catulong0(config_data(&suser), u, 0))
			nomem();
	}
	if (flagverprecip && !stralloc_cats(config_data(&suser), "-"))
		nomem();
	if (!stralloc_cats(config_data(&suser), "@"))
		nomem();
	if (!stralloc_cat(config_data(&suser), config_data(&shost)))
		nomem();
	if (!stralloc_cats(config_data(&user), "@"))
		nomem();
	if (!stralloc_cat(config_data(&user), config_data(&host)))
		nomem();
	if (!mess822_begin(&h, a))
		nomem();
	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match && !line.len)
			break;
		if (!match && !stralloc_append(&line, "\n"))
			nomem();
		if (flagheader && !mess822_ok(&line)) {
			finishheader();
			flagheader = 0;
			if (line.len != 1)
				myput("\n", 1);
		}
		if (!flagheader)
			myput(line.s, line.len);
		else
		if (!mess822_line(&h, &line))
			nomem();
		if (!match)
			break;
	}
	if (flagheader)
		finishheader();
	finishmessage();
	return(0);
}

void
getversion_new_inject_c()
{
	static char    *x = "$Id: new-inject.c,v 1.11 2020-04-30 18:09:45+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
