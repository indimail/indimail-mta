/*
 * $Log: qnotify.c,v $
 * Revision 1.13  2024-01-23 01:23:21+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.12  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.11  2021-07-05 21:11:27+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.10  2021-05-13 14:44:21+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.9  2020-05-11 11:00:10+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.8  2020-04-04 12:43:17+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.7  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.6  2013-06-09 17:03:36+05:30  Cprogrammer
 * shortened variable declartion in addrparse() function
 *
 * Revision 1.5  2012-11-24 08:01:36+05:30  Cprogrammer
 * fixed display of usage
 *
 * Revision 1.4  2011-12-05 19:44:24+05:30  Cprogrammer
 * skip host prefix in the RECIPIENT address
 *
 * Revision 1.3  2011-12-05 17:43:25+05:30  Cprogrammer
 * added option to enclose headers only instead of full email
 *
 * Revision 1.2  2011-11-27 13:43:47+05:30  Cprogrammer
 * process headers only
 *
 * Revision 1.1  2011-11-27 11:58:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "stralloc.h"
#include <case.h>
#include <str.h>
#include <getln.h>
#include <substdio.h>
#include <datetime.h>
#include <date822fmt.h>
#include <mess822.h>
#include <now.h>
#include <env.h>
#include <fmt.h>
#include <error.h>
#include <envdir.h>
#include <pathexec.h>
#include <strerr.h>
#include <noreturn.h>
#include "buffer_defs.h"
#include "qmail.h"
#include "sgetopt.h"
#include "set_environment.h"

#define FATAL "qnotify: fatal: "
#define WARN  "qnotify: warn: "
#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define OPEN_ERR  4
#define DUP_ERR   5
#define LSEEK_ERR 6
#define USAGE_ERR 7

/*
 * RFC 3798
 * The presence of a Disposition-Notification-To header in a message is
 * merely a request for an MDN.  The recipients' user agents are always
 * free to silently ignore such a request.  Alternatively, an explicit
 * denial of the request for information about the disposition of the
 * message may be sent using the "denied" disposition in an MDN.
 *
 * An MDN MUST NOT itself have a Disposition-Notification-To header.  An
 * MDN MUST NOT be generated in response to an MDN.
 *
 * A user agent MUST NOT issue more than one MDN on behalf of each
 * particular recipient.
 *
 * MDNs SHOULD NOT be sent automatically if the address in the
 * Disposition-Notification-To header differs from the address in the
 * Return-Path header
 *
 * Confirmation from the user SHOULD be obtained (or no MDN sent) if
 * there is no Return-Path header in the message
 *
 * The comparison of the addresses should be done using only the addr-
 * spec (local-part "@" domain) portion, excluding any phrase and route.
 * The comparison MUST be case-sensitive for the local-part and case-
 * insensitive for the domain part.
 *
 * A message that contains a Disposition-Notification-To header SHOULD
 * also contain a Message-ID header
 *
 */

typedef const char c_char;
static char     strnum[FMT_ULONG];
static char     ssoutbuf[BUFSIZE_OUT];
static char     sserrbuf[BUFSIZE_OUT];
static c_char  *usage = "usage: qnotify [-n][-h]\n";
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static int      flagqueue = 1;
static struct qmail    qqt;

no_return void
die_fork()
{
	substdio_putsflush(&sserr, "qnotify: fatal: unable to fork\n");
	_exit (WRITE_ERR);
}

no_return void
die_qqperm()
{
	substdio_putsflush(&sserr, "qnotify: fatal: permanent qmail-queue error\n");
	_exit (100);
}

no_return void
die_qqtemp()
{
	substdio_putsflush(&sserr, "qnotify: fatal: temporary qmail-queue error\n");
	_exit (111);
}

void
logerr(const char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit (WRITE_ERR);
}

void
logerrf(const char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit (WRITE_ERR);
	if (substdio_flush(&sserr) == -1)
		_exit (WRITE_ERR);
}

no_return void
my_error(const char *s1, const char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	if (exit_val == 7) {
		logerr("\n");
		logerrf(usage);
		_exit (exit_val);
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit (exit_val);
}

void
my_puts(const char *s)
{
	if (flagqueue)
		qmail_puts(&qqt, s);
	else
	if (substdio_puts(&ssout, s) == -1)
		my_error("write", 0, WRITE_ERR);
}

void
my_putb(const char *s, int len)
{
	if (flagqueue)
		qmail_put(&qqt, s, len);
	else
	if (substdio_bput(&ssout, s, len) == -1)
		my_error("write", 0, WRITE_ERR);
}

static int
mkTempFile(int seekfd)
{
	char            inbuf[2048], outbuf[2048];
	const char     *tmpdir;
	static stralloc tmpFile = {0};
	struct substdio ssin;
	struct substdio sstmp;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF)
		my_error("read error", 0, READ_ERR);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_0(&tmpFile))
		my_error("out of memory", 0, MEM_ERR);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		my_error("open error", tmpFile.s, OPEN_ERR);
	unlink(tmpFile.s);
	substdio_fdbuf(&sstmp, write, fd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof(inbuf));
	switch (substdio_copy(&sstmp, &ssin))
	{
	case -2: /*- read error */
		close(fd);
		my_error("read error", 0, READ_ERR);
	case -3: /*- write error */
		close(fd);
		my_error("write error", 0, WRITE_ERR);
	}
	if (substdio_flush(&sstmp) == -1) {
		close(fd);
		my_error("write error", 0, WRITE_ERR);
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			close(fd);
			my_error("dup2 error", 0, DUP_ERR);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0) {
		close(seekfd);
		my_error("lseek error", 0, LSEEK_ERR);
	}
	return (0);
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

stralloc        line = { 0 };
stralloc        email_date = { 0 };
stralloc        email_subj = { 0 };
stralloc        email_from = { 0 };
stralloc        email_msgid = { 0 };
stralloc        email_disp = { 0 };
stralloc        disp_hdr = { 0 };
int             got_date, got_subj, got_rpath, got_msgid,
				got_from, got_disposition;

void
parse_email(int get_subj, int get_rpath)
{
	struct substdio ssin;
	static char     ssinbuf[1024];
	int             match;
	const char     *disposition_hdr;

	/*- original mail on stdin */
	got_disposition = got_msgid = got_from = got_date = 0;
	if (get_subj)
		got_subj = 0;
	if (get_rpath)
		got_rpath = 0;
	if (!(disposition_hdr = env_get("DISPOSITION_HEADER")))
		disposition_hdr = "Disposition-Notification-To";
	if (!stralloc_copys(&disp_hdr, disposition_hdr))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&disp_hdr, ": ", 2))
		my_error("out of memory", 0, MEM_ERR);
	mkTempFile(0);
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read error", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		if (!mess822_ok(&line))
			break;
		if (!got_date && !str_diffn(line.s, "Date: ", 6)) {
			got_date = 1;
			if (!stralloc_copyb(&email_date, line.s + 6, line.len - 6))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_subj && !str_diffn(line.s, "Subject: ", 9)) {
			got_subj = 1;
			if (!stralloc_copyb(&email_subj, line.s + 9, line.len - 9))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_rpath && !str_diffn(line.s, "Return-Path: ", 13)) {
			got_rpath = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				_exit (0);
			if (!stralloc_copy(&rpath, &addr))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_from && !str_diffn(line.s, "From: ", 6))
		{
			got_from = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				_exit (0);
			if (!stralloc_copy(&email_from, &addr))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_msgid && !case_diffb(line.s, 12, "Message-ID: ")) {
			got_msgid = 1;
			if (!stralloc_copyb(&email_msgid, line.s + 12, line.len - 12))
				my_error("out of memory", 0, MEM_ERR);
		} else
		if (!got_disposition && !case_diffb(line.s, disp_hdr.len, disp_hdr.s)) {
			got_disposition = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) {
				if (!stralloc_copyb(&email_disp, line.s + disp_hdr.len, line.len - disp_hdr.len))
					my_error("out of memory", 0, MEM_ERR);
			} else
			if (!stralloc_copy(&email_disp, &addr))
				my_error("out of memory", 0, MEM_ERR);
		}
		if (got_date && got_subj && got_rpath && got_msgid && got_from &&
			got_disposition)
			break;
	}
	if (lseek(0, 0, SEEK_SET) == -1)
		my_error("lseek error", 0, LSEEK_ERR);
}

int
compare_local(char *s1, int i1, char *s2, int i2)
{
	int             at1, at2;

	if (s1[at1 = str_chr(s1, '@')] && s2[at2 = str_chr(s2, '@')]) {
		/*- match domain */
		if (case_diffb(s1 + at1 + 1, i1 - at1 - 1, s2 + at2 + 1))
			return (0);
		else
		if (str_diffn(s1, s2, at1))
			return (0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	stralloc        boundary = {0};
	int             ch, match, headers_only = 0;
	unsigned long   id;
	datetime_sec    birth;
	struct datetime dt;
	struct substdio ssin;
	static char     ssinbuf[1024];
	char            buf[DATE822FMT];
	char           *rpline, *recipient, *host;
	const char     *qqx;

	while ((ch = getopt(argc, argv, "nh")) != sgoptdone) {
		switch (ch)
		{
		case 'h':
			headers_only = 1;
			break;
		case 'n':
			flagqueue = 0;
			break;
		default:
			logerrf(usage);
			_exit(1);
		}
	}
	birth = now();
	id = getpid();
	datetime_tai(&dt, birth);
	/*- read the email */
	parse_email(1, 1);
	if (!got_disposition || !got_msgid)
		_exit (0);
	if ((rpline = env_get("SENDER"))) {
		if (!addrparse(rpline) && !stralloc_copys(&rpath, rpline))
			my_error("out of memory", 0, MEM_ERR);
	} else
	if ((rpline = env_get("RPLINE"))) {
		if (!addrparse(rpline) && !stralloc_copys(&rpath, rpline))
			my_error("out of memory", 0, MEM_ERR);
	} else
	if (!got_rpath)
		_exit (0);
	if (!stralloc_0(&rpath))
		my_error("out of memory", 0, MEM_ERR);
	rpath.len--;
	if (!(recipient = env_get("RECIPIENT"))) {
		logerrf("recipient not set\n");
		_exit (0);
	}
	if (!(host = env_get("HOST"))) {
		logerrf("HOST not set\n");
		_exit (0);
	}
	recipient += str_len(host) + 1; /*- testindi.com-mbhangui@testindi.com */
	/*-
	 * compare the disposition and return path addresses
	 */
	if (!compare_local(email_disp.s, email_disp.len, rpath.s, rpath.len))
		_exit (0);
	if (flagqueue) {
		set_environment(WARN, FATAL, 0);
		if (qmail_open(&qqt) == -1)
			die_fork();
	}
	my_putb("From: <", 7);
	my_puts(recipient);
	my_putb(">\n", 2);
	my_putb("Date: ", 6);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("Subject: ", 9);
	my_putb("Disposition Notification - ", 27);
	my_putb(email_subj.s, email_subj.len);
	my_putb("To: ", 4);
	my_putb(email_disp.s, email_disp.len);
	my_putb("\n", 1);
	my_puts( "MIME-Version: 1.0\n\n"
			"Content-Type: multipart/report;  report-type=disposition-notification;\n"
			" boundary=\"");
	if (!stralloc_copyb(&boundary, "_----------=_", 13))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, birth)))
		my_error("out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, id)))
		my_error("out of memory", 0, MEM_ERR);
	my_putb(boundary.s, boundary.len);
	my_putb("\"\n", 2);

	/*- Body */
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n\n", 2);
	my_putb("The message sent on ", 20);
	my_putb(email_date.s, email_date.len);
	my_putb("From ", 5);
	my_putb(email_from.s, email_from.len);
	my_putb(" to ", 4);
	my_puts(recipient);
	my_putb(" with \nsubject ", 15);
	my_putb(email_subj.s, email_subj.len);
	my_putb("has been delivered at ", 22);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("There is no guarantee that the message has been read or understood.\n", 68);

	/*- MDN report */
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_putb("Content-Type: message/disposition-notification\n\n", 48);
	my_putb("Reporting-UA: ", 14);
	my_putb(email_disp.s, email_disp.len);
	my_putb("; IndiMail MDA", 14);
	my_putb("\n", 1);
	my_putb("Original-Recipient: rfc822; ", 28);
	my_puts(recipient);
	my_putb("\n", 1);
	my_putb("Final-Recipient: rfc822; ", 25);
	my_puts(recipient);
	my_putb("\n", 1);
	my_putb("Disposition: automatic-action/MDN-sent-automatically; displayed\n", 64);

	/*- enclose the original mail as attachment */
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
		qmail_to(&qqt, rpath.s);
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
getversion_qnotify_c()
{
	const char     *x = "$Id: qnotify.c,v 1.13 2024-01-23 01:23:21+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
