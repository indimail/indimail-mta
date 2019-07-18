/*
 * $Log: qarf.c,v $
 * Revision 1.10  2016-01-02 17:45:20+05:30  Cprogrammer
 * reformatted error strings
 *
 * Revision 1.9  2013-06-09 17:02:52+05:30  Cprogrammer
 * shortened variable declartion in addrparse() function
 *
 * Revision 1.8  2012-11-24 08:01:22+05:30  Cprogrammer
 * fixed display of usage
 *
 * Revision 1.7  2011-11-27 13:43:25+05:30  Cprogrammer
 * process headers only
 *
 * Revision 1.6  2011-11-27 11:56:53+05:30  Cprogrammer
 * exit on address parsing error
 *
 * Revision 1.5  2011-11-26 15:35:32+05:30  Cprogrammer
 * removed useless statements
 *
 * Revision 1.4  2011-02-12 12:36:10+05:30  Cprogrammer
 * set copy_subj, copy_rpath automatically
 *
 * Revision 1.3  2011-02-12 10:40:15+05:30  Cprogrammer
 * added more fields as per http://www.mipassoc.org/arf
 *
 * Revision 1.2  2011-02-11 23:45:32+05:30  Cprogrammer
 * added usage
 *
 * Revision 1.1  2011-02-11 23:28:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "stralloc.h"
#include "str.h"
#include "getln.h"
#include "substdio.h"
#include "datetime.h"
#include "date822fmt.h"
#include "mess822.h"
#include "now.h"
#include "env.h"
#include "fmt.h"
#include "error.h"
#include "sgetopt.h"

#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define OPEN_ERR  4
#define DUP_ERR   5
#define LSEEK_ERR 6
#define USAGE_ERR 7

char            strnum[FMT_ULONG];
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
char           *usage = "usage: qarf [-i] -t recipient -s subject -f sender [-m filename]\n";

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
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
	_exit(exit_val);
}

void
my_puts(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		my_error("qarf: write", 0, WRITE_ERR);
}

void
my_putb(char *s, int len)
{
	if (substdio_bput(&ssout, s, len) == -1)
		my_error("qarf: write", 0, WRITE_ERR);
}

static int
mkTempFile(int seekfd)
{
	char            inbuf[2048], outbuf[2048], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = {0};
	struct substdio ssin;
	struct substdio ssout;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF)
		my_error("qarf: read", 0, READ_ERR);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if (!stralloc_0(&tmpFile))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		my_error("qarf: open", tmpFile.s, OPEN_ERR);
	unlink(tmpFile.s);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof(inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(fd);
		my_error("qarf: read", 0, READ_ERR);
	case -3: /*- write error */
		close(fd);
		my_error("qarf: write", 0, WRITE_ERR);
	}
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		my_error("qarf: write", 0, WRITE_ERR);
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			close(fd);
			my_error("qarf: dup2", 0, DUP_ERR);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0) {
		close(seekfd);
		my_error("qarf: lseek", 0, LSEEK_ERR);
	}
	return (0);
}

stralloc        addr = { 0 };

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
		my_error("qarf: out of memory", 0, MEM_ERR);
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) {	/*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				my_error("qarf: out of memory", 0, MEM_ERR);
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
					my_error("qarf: out of memory", 0, MEM_ERR);
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
stralloc        rpath = { 0 };
stralloc        email_dkimstat = { 0 };
stralloc        email_deliveredto = { 0 };

void
parse_email(int get_subj, int get_rpath)
{
	struct substdio ssin;
	static char     ssinbuf[1024];
	int             match, got_date, got_subj, got_rpath, got_from, got_msgid, got_dkimstat,
					got_deliveredto;

	/*- original mail on stdin */
	got_deliveredto = got_dkimstat = got_msgid = got_from = got_date = 0;
	got_subj = !get_subj;
	got_rpath = !get_rpath;
	mkTempFile(0);
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("qarf: read", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		if (!mess822_ok(&line))
			break;
		if (!got_date && !str_diffn(line.s, "Date: ", 6)) {
			got_date = 1;
			if (!stralloc_copyb(&email_date, line.s + 6, line.len - 6))
				my_error("qarf: out of memory", 0, MEM_ERR);
		} else
		if (!got_subj && !str_diffn(line.s, "Subject: ", 9)) {
			got_subj = 1;
			if (!stralloc_copyb(&email_subj, line.s + 9, line.len - 9))
				my_error("qarf: out of memory", 0, MEM_ERR);
		} else
		if (!got_rpath && !str_diffn(line.s, "Return-Path: ", 13)) {
			got_rpath = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				my_error ("qarf: unable to parse address", line.s, 111);
			if (!stralloc_copy(&rpath, &addr))
				my_error("qarf: out of memory", 0, MEM_ERR);
		} else
		if (!got_from && !str_diffn(line.s, "From: ", 6)) {
			got_from = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				my_error ("qarf: unable to parse address", line.s, 111);
			if (!stralloc_copy(&email_from, &addr))
				my_error("qarf: out of memory", 0, MEM_ERR);
		} else
		if (!got_msgid && !str_diffn(line.s, "Message-ID: ", 12)) {
			got_msgid = 1;
			if (!stralloc_copyb(&email_msgid, line.s + 12, line.len - 12))
				my_error("qarf: out of memory", 0, MEM_ERR);
		} else
		if (!got_dkimstat && !str_diffn(line.s, "DKIM-Status: ", 13)) {
			got_dkimstat = 1;
			if (str_diffn(line.s + 13, "good", 4)) {
				if (!stralloc_copyb(&email_dkimstat, line.s + 13, line.len - 13))
					my_error("qarf: out of memory", 0, MEM_ERR);
			}
		} else
		if (!got_deliveredto && !str_diffn(line.s, "Delivered-To: ", 14)) {
			got_deliveredto = 1;
			line.s[line.len - 1] = 0;
			if (!addrparse(line.s)) /*- sets addr */
				my_error ("qarf: unable to parse address", line.s, 111);
			if (!stralloc_copy(&email_deliveredto, &addr))
				my_error("qarf: out of memory", 0, MEM_ERR);
		}
		if (got_date && got_subj && got_rpath && got_from && got_msgid 
				&& got_dkimstat && got_deliveredto)
			break;
	}
	if (lseek(0, 0, SEEK_SET) == -1)
		my_error("qarf: lseek", 0, LSEEK_ERR);
}
int
main(int argc, char **argv)
{
	stralloc        boundary = {0};
	int             ch, match, fd, r, at, inLine = 0, copy_subj = 0, copy_rpath = 0;
	unsigned long   id;
	datetime_sec    birth;
	struct datetime dt;
	struct substdio ssin;
	static char     ssinbuf[1024];
	char            buf[DATE822FMT], inbuf[128];
	char           *to, *from, *subject, *text, *ip, *reported_ip;

	to = from = subject = text = 0;
	reported_ip = 0;
	while ((ch = getopt(argc, argv, "it:f:s:m:I:")) != sgoptdone) {
		switch (ch)
		{
		case 'i':
			inLine = 1;
			break;
		case 't':
			to = optarg;
			break;
		case 'f':
			from = optarg;
			break;
		case 's':
			subject = optarg;
			break;
		case 'm':
			text = optarg;
			break;
		case 'I':
			reported_ip = optarg;
			break;
		default:
			logerrf(usage);
			_exit(1);
		}
	}
	if (!(ip = env_get("TCPREMOTEIP")))
		ip = "localhost";
	if (!to)
		copy_rpath = 1;
	if (to && copy_rpath)
		my_error("qarf: you cannot specify -t & -T option together", 0, USAGE_ERR);
	if (!from)
		my_error("qarf: sender not specified", 0, USAGE_ERR);
	if (!subject)
		copy_subj = 1;
	birth = now();
	id = getpid();
	datetime_tai(&dt, birth);
	parse_email(copy_subj, copy_rpath);
	my_putb("From: ", 6);
	my_puts(from);
	my_putb("\n", 1);
	my_putb("Date: ", 6);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("Subject: ", 9);
	if (copy_subj) {
		my_putb("FW: ", 4);
		my_putb(email_subj.s, email_subj.len);
	} else {
		my_puts(subject);
		my_putb("\n", 1);
	}
	my_putb("To: ", 4);
	if (copy_rpath)
	{
		if (rpath.s[at = str_chr(rpath.s, '@')])
		{
			my_putb("abuse", 5);
			my_putb(rpath.s + at, rpath.len - at);
		} else
			my_putb(rpath.s, rpath.len);
	} else 
		my_puts(to);
	my_putb("\n", 1);
	my_puts(
			"MIME-Version: 1.0\n"
			"Content-Transfer-Encoding: binary\n"
			"Content-Type: multipart/report; "
			"boundary=\"");
	if (!stralloc_copyb(&boundary, "_----------=_", 13))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, birth)))
		my_error("qarf: out of memory", 0, MEM_ERR);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, id)))
		my_error("qarf: out of memory", 0, MEM_ERR);
	my_putb(boundary.s, boundary.len);
	my_putb("\"; ", 3);
	my_puts(
			"report-type=\"feedback-report\"\n"
			"X-Mailer: qarf $Revision: 1.10 $\n");

	/*- Body */
	my_puts("\nThis is a multi-part message in MIME format\n\n");
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_puts(
			"Content-Disposition: inline\n"
			"Content-Type: text/plain; charset=\"US-ASCII\"\n"
			"Content-Transfer-Encoding: 7bit\n\n");
	if (text)
	{
		if ((fd = open(text, O_RDONLY)) == -1)
			my_error("qarf: open", text, OPEN_ERR);
		substdio_fdbuf(&ssin, read, fd, ssinbuf, sizeof(ssinbuf));
		while ((r = substdio_get(&ssin, inbuf, sizeof(inbuf))) > 0)
			my_putb(inbuf, r);
		close(fd);
		if (r == -1)
			my_error("qarf: read", 0, READ_ERR);
	} else
	{
		my_puts("This is an email abuse report for an email received");
		if (reported_ip)
		{
			my_puts("from IP\n");
			my_puts(reported_ip);
		}
		my_putb(" on ", 4);
		my_putb(buf, date822fmt(buf, &dt) - 1);
		my_putb(".\n", 2);
		my_puts("For more information about this format please see http://www.mipassoc.org/arf/.\n");
	}
	my_putb("\n--", 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_puts(
			"Content-Disposition: inline\n"
			"Content-Transfer-Encoding: 7bit\n"
			"Content-Type: message/feedback-report\n\n");

	my_puts(
			"Feedback-Type: abuse\n"
			"User-Agent: $Id: qarf.c,v 1.10 2016-01-02 17:45:20+05:30 Cprogrammer Stab mbhangui $\n"
			"Version: 0.1\n");
	if (email_from.len) {
		my_putb("Original-Mail-From: ", 20);
		my_putb(email_from.s, email_from.len);
	}
	if (email_deliveredto.len) {
		my_putb("Original-Rcpt-To: ", 18);
		my_putb(email_deliveredto.s, email_deliveredto.len);
	}
	if (email_msgid.len) {
		my_putb("Original-Envelope-Id: ", 22);
		my_putb(email_msgid.s, email_msgid.len);
	}
	if (email_dkimstat.len) {
		my_putb("DKIM-Failure: ", 14);
		my_putb(email_dkimstat.s, email_dkimstat.len);
	}
	if (reported_ip) {
		my_putb("Source-IP: ", 11);
		my_puts(reported_ip);
		my_putb("\n", 1);
	}
	if (email_date.len) {
		my_putb("Arrival-Date: ", 14);
		my_putb(email_date.s, email_date.len);
	}
	my_putb("Received-Date: ", 15);
	my_putb (buf, date822fmt(buf, &dt));

	/*- enclose the original mail as attachment */
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_puts("Content-Disposition: ");
	my_puts(inLine ? "inline\n" : "attachment\n");
	my_puts(
			"Content-Transfer-Encoding: 8bit\n"
			"Content-Type: message/rfc822\n\n");

	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("qarf: read", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		my_putb(line.s, line.len);
	}
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("--\n", 3);
	my_putb("\n", 1);
	if (substdio_flush(&ssout) == -1)
		my_error("qarf: write", 0, WRITE_ERR);
	return(0);
}

void
getversion_qarf_c()
{
	static char    *x = "$Id: qarf.c,v 1.10 2016-01-02 17:45:20+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
