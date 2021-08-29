/*
 * $Log: rrt.c,v $
 * Revision 1.11  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.10  2021-07-05 21:11:47+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.9  2021-06-13 17:29:14+05:30  Cprogrammer
 * removed chdir(auto_sysconfdir)
 *
 * Revision 1.8  2021-05-13 14:44:40+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.7  2020-04-04 13:01:03+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.6  2017-04-10 20:42:36+05:30  Cprogrammer
 * renamed ONTRANSIENT_ERROR to ONTEMPORARY_ERROR
 *
 * Revision 1.5  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.4  2013-06-09 17:03:41+05:30  Cprogrammer
 * shortened variable declartion in addrparse() function
 *
 * Revision 1.3  2012-11-25 07:56:34+05:30  Cprogrammer
 * modify subject according to type
 *
 * Revision 1.2  2012-11-24 11:06:37+05:30  Cprogrammer
 * improved readability
 *
 * Revision 1.1  2012-11-24 08:19:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <errno.h>
#include <sgetopt.h>
#include <case.h>
#include <fmt.h>
#include <str.h>
#include <now.h>
#include <datetime.h>
#include <mess822.h>
#include <date822fmt.h>
#include <getln.h>
#include <stralloc.h>
#include <error.h>
#include <envdir.h>
#include <env.h>
#include <strerr.h>
#include <pathexec.h>
#include <noreturn.h>
#include "variables.h"
#include "control.h"
#include "qmail.h"
#include "set_environment.h"

#define FATAL     "rrt: fatal: "
#define WARN      "rrt: warn: "
#define SUCCESS   1
#define FAILURE   2
#define TEMPORARY 3
#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define OPEN_ERR  4
#define DUP_ERR   5
#define LSEEK_ERR 6
#define USAGE_ERR 7
#define PARSE_ERR 8

static char     ssoutbuf[512], sserrbuf[512], strnum[FMT_ULONG];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static char    *usage = "usage: rrt [-n][-b]\n";
static struct qmail    qqt;
static int      flagqueue = 1;
static stralloc line = { 0 };

no_return void
die_fork()
{
	substdio_putsflush(&sserr, "rrt: fatal: unable to fork\n");
	_exit (WRITE_ERR);
}

no_return void
die_qqperm()
{
	substdio_putsflush(&sserr, "rrt: fatal: permanent qmail-queue error\n");
	_exit (100);
}

no_return void
die_qqtemp()
{
	substdio_putsflush(&sserr, "rrt: fatal: temporary qmail-queue error\n");
	_exit (111);
}

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit (WRITE_ERR);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit (WRITE_ERR);
	if (substdio_flush(&sserr) == -1)
		_exit (WRITE_ERR);
}

no_return void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit (exit_val);
}

void
my_puts(char *s)
{
	if (flagqueue)
		qmail_puts(&qqt, s);
	else
	if (substdio_puts(&ssout, s) == -1)
		my_error("write", 0, WRITE_ERR);
}

void
my_putb(char *s, int len)
{
	if (flagqueue)
		qmail_put(&qqt, s, len);
	else
	if (substdio_bput(&ssout, s, len) == -1)
		my_error("write", 0, WRITE_ERR);
}

stralloc        addr = { 0 }, rpath = {0};

int
addrparse(char *arg)
{
	int             i, flagesc, flagquoted;
	char            ch, terminator;

	terminator = '>';
	i = str_chr(arg, '<');
	if (arg[i])
		arg += i + 1;
	else {	/*- partner should go read rfc 821 */
		terminator = ' ';
		arg += str_chr(arg, ':');
		if (*arg == ':')
			++arg;
		if (!*arg)
			return (0);
		while (*arg == ' ')
			++arg;
	}
	/*- strip source route */
	if (*arg == '@') {
		while (*arg) {
			if (*arg++ == ':')
				break;
		}
	}
	if (!stralloc_copys(&addr, ""))
		my_error("out of memory", 0, MEM_ERR);
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) {	/*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				my_error("out of memory", 0, MEM_ERR);
			flagesc = 0;
		} else {
			if (!flagquoted && ch == terminator)
				break;
			switch (ch)
			{
			case '\\':
				flagesc = 1;
				break;
#ifdef STRIPSINGLEQUOTES
			case '\'':
				flagquoted = !flagquoted;
				break;
#endif
			case '"':
				flagquoted = !flagquoted;
				break;
			default:
				if (!stralloc_append(&addr, &ch))
					my_error("out of memory", 0, MEM_ERR);
			}
		}
	}
	/*- could check for termination failure here, but why bother? */
	if (addr.len > 900)
		return 0;
	return 1;
}

int             got_rr, got_subj, got_msgid, got_from;
stralloc        email_from = { 0 };
stralloc        email_subj = { 0 };
stralloc        email_msgid = { 0 };
stralloc        email_rr = { 0 };

void
parse_email()
{
	struct substdio ssin;
	static char     ssinbuf[1024];
	int             match;

	if (lseek(0, 0, SEEK_SET) == -1)
		my_error("lseek error", 0, LSEEK_ERR);
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	got_rr = got_subj = got_msgid = got_from = 0;
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read error", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		if (!mess822_ok(&line))
			break;
		if (!got_from && !str_diffn(line.s, "From: ", 6)) {
			got_from = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				_exit (PARSE_ERR);
			if (!stralloc_copy(&email_from, &addr))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_msgid && !case_diffb(line.s, 12, "Message-ID: ")) {
			got_msgid = 1;
			if (!stralloc_copyb(&email_msgid, line.s + 12, line.len - 12))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_subj && !str_diffn(line.s, "Subject: ", 9)) {
			got_subj = 1;
			if (!stralloc_copyb(&email_subj, line.s + 9, line.len - 9))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_rr && !case_diffb(line.s, 19, "Return-Receipt-To: ")) {
			got_rr = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s))
				_exit (PARSE_ERR);
			if (!stralloc_copy(&email_rr, &addr))
				my_error("out of memory", 0, MEM_ERR);
		}
		if (got_rr && got_msgid && got_subj && got_from)
			break;
	}
	if (lseek(0, 0, SEEK_SET) == -1)
		my_error("lseek error", 0, LSEEK_ERR);
}

int
main(int argc, char **argv)
{
	int             ch, match, headers_only = 1, type = SUCCESS;
	unsigned long   id;
	struct datetime dt;
	stralloc        boundary = {0}, bouncefrom = { 0 }, bouncehost = { 0 };
	datetime_sec    birth;
	struct substdio ssin;
	static char     ssinbuf[1024];
	char            buf[DATE822FMT];
	char           *rpline, *recipient, *qqx, *ptr,
				   *smtptext = 0, *qmtptext = 0;

	while ((ch = getopt(argc, argv, "nb")) != opteof) {
		switch (ch)
		{
		case 'b':
			headers_only = 0;
			break;
		case 'n':
			flagqueue = 0;
			break;
		default:
			logerrf(usage);
			_exit(1);
			break;
		}
	}
	argc -= optind;
	argv += optind;
	birth = now();
	id = getpid();
	datetime_tai(&dt, birth);

	rpline = argv[1];
	recipient = argv[4];
	if (!addrparse(rpline) && !stralloc_copys(&rpath, rpline))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_0(&rpath))
		my_error("out of memory", 0, MEM_ERR);
	rpath.len--;
	parse_email();
	if (!got_rr || !got_msgid)
		_exit (0);
	if (!stralloc_0(&email_rr))
		my_error("out of memory", 0, MEM_ERR);
	email_rr.len--;
	if (control_init() == -1)
		strerr_die2sys(111, FATAL, "unable to read init controls: ");
	if (control_rldef(&bouncefrom, "bouncefrom", 0, "MAILER-DAEMON") != 1)
		strerr_die2sys(111, FATAL, "unable to read bouncefrom controls: ");
	if (control_rldef(&bouncehost, "bouncehost", 1, "bouncehost") != 1)
		strerr_die2sys(111, FATAL, "unable to read bouncehost controls: ");

	if (env_get("ONSUCCESS_REMOTE"))
		type = SUCCESS;
	else
	if (env_get("ONFAILURE_REMOTE"))
		type = FAILURE;
	else
	if (env_get("ONTEMPORARY_REMOTE"))
		type = TEMPORARY;
	else
		_exit (0);
	/*- no need of processing bounce */
	if (type == FAILURE) {
		if (rpath.len == email_rr.len && !str_diffn(rpath.s, email_rr.s, rpath.len))
			return (0);
	}
	if (flagqueue) {
		set_environment(WARN, FATAL, 0);
		if (qmail_open(&qqt) == -1)
			die_fork();
	}
	my_putb("From: <", 7);
	my_putb(bouncefrom.s, bouncefrom.len);
	my_putb("@", 1);
	my_putb(bouncehost.s, bouncehost.len);
	my_putb(">\n", 2);
	my_putb("Date: ", 6);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("Subject: ", 9);
	if (type == SUCCESS)
		my_putb("Successful Mail Delivery Report: ", 33);
	else
	if (type == FAILURE)
		my_putb("Permanent Failure: Mail Delivery Report: ", 41);
	else
	if (type == TEMPORARY)
		my_putb("Temporary Failure: Mail Delivery Report: ", 41);
	my_putb(email_subj.s, email_subj.len);
	my_putb("To: ", 4);
	my_putb(email_rr.s, email_rr.len);
	my_putb("\n", 1);
	my_puts( "MIME-Version: 1.0\n"
			"Content-Type: multipart/report;  report-type=delivery-status;\n"
			" boundary=\"");
	if (!stralloc_copyb(&boundary, "_----------=_", 13))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, birth)))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, id)))
		my_error("out of memory", 0, MEM_ERR);
	my_putb(boundary.s, boundary.len);
	my_putb("\"\n", 2);

	/*- first report */
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_putb("Content-Type: message/delivery-status\n\n", 39);
	my_putb("This is the indimail rrt generator at host ", 43);
	my_putb(bouncehost.s, bouncehost.len);
	my_putb(".\n\n", 3);
	my_putb("This message was received for a delivery attempt at ", 52);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("from ", 5);
	my_puts(rpline);
	my_putb("\n\n", 2);
	if (type == SUCCESS) {
		my_puts("Your message was successfully delivered to the destination(s)\n"
				"listed below. If the message was delivered to mailbox you will\n"
				"receive no further notifications. Otherwise you may still receive\n"
				"notifications of mail delivery errors from other systems.\n\n");
	} else
	if (type == TEMPORARY) {
		my_puts("Your message has not been delivered to the destination(s)\n"
				"listed below. You may receive further notifications until the\n"
				"message gets delivered\n\n");
	}
	my_puts(recipient);
	my_putb("\n\n", 2);
	if ((smtptext = env_get("SMTPTEXT")) || (qmtptext = env_get("QMTPTEXT"))) {
		my_puts(smtptext ? "SMTP Text: " : "QMTP Text: ");
		my_puts(smtptext ? smtptext + 1 : qmtptext + 1);
	}
	my_putb("\n\n", 2);

	if (type == SUCCESS) {
		my_putb("There is no guarantee that the message has been read or understood.\n", 68);
	} else
	if (type == TEMPORARY) {
		my_putb("There is no guarantee that the message will be deliverd.\n", 57);
	}

	/*- RRT report */
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_putb("Content-Type: message/delivery-status\n\n", 39);
	my_putb("Reporting-MTA: ", 15);
	my_putb(bouncehost.s, bouncehost.len);
	my_putb("\n", 1);
	my_putb("Arrival-Date: ", 14);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("Original-Recipient: rfc822; ", 28);
	my_puts(recipient);
	my_putb("\n", 1);
	my_putb("Final-Recipient: rfc822; ", 25);
	my_puts(recipient);
	my_putb("\n", 1);

	my_putb("Action: ", 8);
	my_puts(type == SUCCESS ? "relayed\n" : "failed\n");
	if ((ptr = env_get("SMTPCODE")) || (ptr = env_get("QMTPCODE"))) {
		my_putb("Status: ", 8);
		my_puts(ptr);
		my_putb("\nDiagnostic code: smtp; ", 24);
		my_puts(smtptext ? smtptext + 1 : qmtptext + 1);
	}
	/*- enclose the original mail header as attachment */
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_putb("Content-Transfer-Encoding: 8bit\n", 32);
	my_putb("Content-Disposition: attachment\n", 32);
	if (headers_only)
		my_putb("Content-Type: text/rfc822-headers\n\n", 35);
	else
		my_putb("Content-Type: message/rfc822\n\n", 30);

	/* You wanted an MDN and you shall get one - the entire lot back to you */
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read error", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		if (headers_only && !mess822_ok(&line))
			break;
		my_putb(line.s, line.len);
	}
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("--\n\n", 4);

	if (flagqueue) {
		qmail_from(&qqt, "");
		qmail_to(&qqt, email_rr.s);
		qqx = qmail_close(&qqt);
		if (*qqx) {
			if (*qqx == 'D')
				die_qqperm();
			else
				die_qqtemp();
		}
	} else
	if (substdio_flush(&ssout) == -1)
		my_error("write", 0, WRITE_ERR);
	_exit (0);
}

void
getversion_rr_c()
{
	static char    *x = "$Id: rrt.c,v 1.11 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
