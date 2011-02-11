/*
 * $Log: arfgen.c,v $
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
#include "now.h"
#include "env.h"
#include "fmt.h"
#include "error.h"
#include "sgetopt.h"

char            strnum[FMT_ULONG];
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
char           *usage = "usage: arfgen -t recipient -s subject -f sender [-T filename]\n";

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
	if (s2)
	{
		logerr(s2);
		logerr(": ");
	}
	if (exit_val == 7)
	{
		logerr("\n");
		logerrf(usage);
		_exit(exit_val);
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

void
my_puts(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		my_error("write", 0, 2);
}

void
my_putb(char *s, int len)
{
	if (substdio_bput(&ssout, s, len) == -1)
		my_error("write", 0, 2);
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
		my_error("read error", 0, 1);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir))
		my_error("out of memory", 0, 3);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		my_error("out of memory", 0, 3);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		my_error("out of memory", 0, 3);
	if (!stralloc_0(&tmpFile))
		my_error("out of memory", 0, 3);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		my_error("open error", tmpFile.s, 4);
	unlink(tmpFile.s);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof(inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(fd);
		my_error("read error", 0, 1);
	case -3: /*- write error */
		close(fd);
		my_error("write error", 0, 2);
	}
	if (substdio_flush(&ssout) == -1)
	{
		close(fd);
		my_error("write error", 0, 2);
	}
	if (fd != seekfd)
	{
		if (dup2(fd, seekfd) == -1)
		{
			close(fd);
			my_error("dup2 error", 0, 5);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0)
	{
		close(seekfd);
		my_error("lseek error", 0, 6);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	stralloc        boundary = {0};
	stralloc        line = { 0 };
	int             ch, match, fd, r;
	unsigned long   id;
	datetime_sec    birth;
	struct datetime dt;
	struct substdio ssin;
	static char     ssinbuf[1024];
	char            buf[DATE822FMT], inbuf[128];
	char           *to, *from, *subject, *text, *ip, *reported_ip;

	to = from = subject = text = 0;
	reported_ip = 0;
	while ((ch = sgopt(argc, argv, "t:f:s:T:")) != sgoptdone)
	{
		switch (ch)
		{
		case 't':
			to = optarg;
			break;
		case 'f':
			from = optarg;
			break;
		case 's':
			subject = optarg;
			break;
		case 'r':
			reported_ip = optarg;
			break;
		case 'T':
			text = optarg;
			break;
		}
	}
	if (!(ip = env_get("TCPREMOTEIP")))
		ip = "localhost";
	if (!to)
		my_error("recipient not specified", 0, 7);
	if (!from)
		my_error("sender not specified", 0, 7);
	if (!subject)
		my_error("subject not specified", 0, 7);
	birth = now();
	id = getpid();
	datetime_tai(&dt, birth);

	my_putb("From: ", 6);
	my_puts(from);
	my_putb("\n", 1);
	my_putb("Date: ", 6);
	my_putb(buf, date822fmt(buf, &dt));
	my_putb("Subject: ", 9);
	my_puts(subject);
	my_putb("\n", 1);
	my_putb("To: ", 4);
	my_puts(to);
	my_putb("\n", 1);
	my_puts(
			"MIME-Version: 1.0\n"
			"Content-Transfer-Encoding: binary\n"
			"Content-Type: multipart/report; "
			"boundary=\"");
	if (!stralloc_copyb(&boundary, "_----------=_", 13))
		my_error("out of memory", 0, 3);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, birth)))
		my_error("out of memory", 0, 3);
	if (!stralloc_catb(&boundary, strnum, fmt_ulong(strnum, id)))
		my_error("out of memory", 0, 3);
	my_putb(boundary.s, boundary.len);
	my_putb("\"; ", 3);
	my_puts(
			"report-type=\"feedback-report\"\n"
			"X-Mailer: IndiMail ARF Generator\n");

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
			my_error("open error", text, 4);
		substdio_fdbuf(&ssin, read, fd, ssinbuf, sizeof(ssinbuf));
		while ((r = substdio_get(&ssin, inbuf, sizeof(inbuf))) > 0)
			my_putb(inbuf, r);
		close(fd);
		if (r == -1)
			my_error("read", 0, 1);
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
			"Feedback-Agent: $Id: arfgen.c,v 1.2 2011-02-11 23:45:32+05:30 Cprogrammer Exp mbhangui $\n"
			"Version: $Revision: 1.2 $\n");
	if (reported_ip) {
		my_putb("Source-IP: ", 11);
		my_puts(reported_ip);
		my_putb("\n", 1);
	}
	/*- original mail on stdin */
	mkTempFile(0);
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read error", 0, 1);
		if (!match && line.len == 0)
			break;
		if (!str_diffn(line.s, "Date: ", 6))
			break;
	}
	my_putb("Received-Date: ", 15);
	my_putb(line.s + 6, line.len - 6);
	my_putb("\n", 1);

	/*- enclose the original mail as attachment */
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("\n", 1);
	my_puts(
			"Content-Disposition: attachment\n"
			"Content-Transfer-Encoding: 8bit\n"
			"Content-Type: message/rfc822\n\n");

	if (lseek(0, 0, SEEK_SET) == -1)
		my_error("lseek error", 0, 6);
	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read error", 0, 1);
		if (!match && line.len == 0)
			break;
		my_putb(line.s, line.len);
	}
	my_putb("--", 2);
	my_putb(boundary.s, boundary.len);
	my_putb("--\n", 3);
	my_putb("\n", 1);
	if (substdio_flush(&ssout) == -1)
		my_error("write", 0, 2);
	return(0);
}

void
getversion_arfgen_c()
{
	static char    *x = "$Id: arfgen.c,v 1.2 2011-02-11 23:45:32+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
