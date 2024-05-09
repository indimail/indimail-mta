/*
 * $Id: rd-remote.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <env.h>
#include <noreturn.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <scan.h>
#include <fmt.h>
#include <wait.h>
#include <getln.h>
#include <str.h>
#include "control.h"
#include "auto_control.h"
#include "auto_prefix.h"
#include "variables.h"
#include "do_match.h"
#include "quote.h"

static stralloc rdomain;

void
out(const char *s)
{
	if (substdio_puts(subfdoutsmall, s) == -1)
		_exit(0);
}

void
zero()
{
	if (substdio_put(subfdoutsmall, "\0", 1) == -1)
		_exit(0);
}

no_return void
zerodie()
{
	zero();
	substdio_flush(subfdoutsmall);
	_exit(0);
}

no_return void
temp_control()
{
	out("ZUnable to read control files. (#4.3.0)\n");
	zerodie();
}

no_return void
temp_nomem()
{
	out("ZOut of memory. (#4.3.0)\n");
	zerodie();
}

no_return void
temp_read_msg()
{
	out("ZUnable to read message. (#4.3.0)\n");
	zerodie();
}

no_return void
temp_read_maildirdeliver()
{
	out("ZUnable to read maildirdeliver error message. (#4.3.0)\n");
	zerodie();
}

no_return void
temp_write()
{
	out("ZUnable to write message. (#4.3.0)\n");
	zerodie();
}

/*-
 * qmail-rspawn doesn't use exit code of qmail-remote. It needs a report in
 * the following format
 * "[r,h,s]recipient_report\0[K,Z,D]message_report\0"
 *
 * recipient_report start with one of the letters r, h, s
 * as below
 * r - Recipient report: acceptance.
 * s - Recipient report: temporary rejection.
 * h - Recipient report: permanent rejection.
 *
 * message_report start with one of the letters K, Z, D
 * as below
 * K - Message report: success.
 * Z - Message report: temporary failure.
 * D - Message report: permanent failure.
 *
 * Examples of qmail-remote report
 *
 * Success
 * "rFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "KHost example.com accepted message\0\n"
 *
 * temp failure
 * "sFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "ZTemporary failure accepting message\0\n"
 *
 * perm failure
 * "hFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "Dexample.org does not like recipient\0\n"
 */
void
print_report(const char *s1, const char *msg, int msglen)
{
	out(s1);
	if (substdio_put(subfdoutsmall, msg, msglen) == -1)
		_exit(0);
	zero();
}

no_return void
perm_usage()
{
	out("DI (rd-remote) was invoked improperly. (#5.3.5)\n");
	zerodie();
}

static stralloc canonhost = { 0 };
static stralloc canonbox = { 0 };

void	 /*- host has to be canonical, box has to be quoted */
addrmangle(stralloc *saout, const char *sender)
{
	int             j;

	j = str_rchr(sender, '@');
	if (!sender[j]) {
		if (!quote2(saout, sender))
			temp_nomem();
		return;
	}
	if (!stralloc_copys(&canonbox, sender))
		temp_nomem();
	canonbox.len = j;
	if (!quote(saout, &canonbox) ||
			!stralloc_cats(saout, "@") ||
			!stralloc_copys(&canonhost, sender + j + 1))
		temp_nomem();
	if (!stralloc_cat(saout, &canonhost))
		temp_nomem();
}

/*
 * argv[0] - rd-remote
 * argv[1] - Recipient SMTP Domain/host
 * argv[2] - Sender
 * argv[3] - qqeh
 * argv[4] - Size
 * argv[5] - Recipient
 */
int
main(int argc, char **argv)
{
	int             r, use_regex, lcount, len, nullflag, count, wstat,
					match;
	int             pipefd[2], errpipe[2];
	unsigned long   size;
	const char     *errStr;
	char           *ptr, *cptr, *addr, *dest_addr, *dest_domain,
				   *prefix;
	char            ssibuf[512], ssobuf[512], ssebuf[512], strnum[FMT_ULONG];
	const char     *(qrargs[]) = { "queue-remote", 0, 0, 0, 0, 0, (char *) 0};
	const char     *(mdargs[]) = { "maildirdeliver", 0, (char *) 0};
	pid_t           pid;
	substdio        ssi, sso, sse;
	stralloc        q = {0}, sender = {0}, recip = {0}, dto = {0}, line = {0};

	if (argc < 6)
		perm_usage();
	addrmangle(&sender, argv[2]);
	addrmangle(&recip, argv[5]);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if ((r = control_readfile(&rdomain, "redirectremote", 1)) == -1)
		temp_control();
	if ((ptr = env_get("QREGEX")))
		scan_int(ptr, &use_regex);
	/*-
	 * format for redirectremote is
	 * address:type:dest
	 * where
	 * address - recipient domain, recipient or sender
	 * Type    - r for recipient, s for sender, d for recipient domain
	 * dest    - destination address
	 * e.g.
	 * yahoo.com:d:root@localhost
	 * test1@example.com.com:r:dave@yahoo.com
	 * test2@example.com.com:s:postmaster@localhost
	 */
	if (!stralloc_copyb(&dto, "Delivered-To: ", 14))
		temp_nomem();
	for (count = lcount = len = 0, ptr = rdomain.s; len < rdomain.len;) {
		len += (str_len(ptr) + 1);
		for (cptr = ptr;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':' && cptr[2] == ':')
			*cptr = 0;
		else {
			ptr = rdomain.s + len;
			continue;
		}
		if (cptr[1] == 'd') { /*- match on recipient domain */
			addr = argv[1];
		} else
		if (cptr[1] == 'r') { /*- match on recipient */
			addr = recip.s;
		} else
		if (cptr[1] == 's') { /*- match on sender */
			addr = sender.s;
		} else {
			lcount++;
			ptr = rdomain.s + len;
			continue;
		}
		if (!*addr && (!*ptr || !str_diffn(ptr, "<>", 3)))
			nullflag = 1;
		else
			nullflag = 0;
		if (nullflag || do_match(use_regex, addr, ptr, &errStr) > 0) {
			dest_addr = cptr + 3;
			if (*dest_addr != '/') { /*- maildir delivery */
				r = str_rchr(dest_addr, '@');
				if (!dest_addr[r]) { /*- invalid dest address */
					out("DI cannot deliver to an invalid address. (#5.3.5)\n");
					zerodie();
				}
				dest_domain = dest_addr + r + 1;
			} else
				dest_domain = NULL;
			r = str_chr(dest_addr, ':');
			if (dest_addr[r]) {
				dest_addr[r] = '\0';
				prefix = dest_addr + r + 1;
			} else
				prefix = NULL;
			count = lcount + 1; /*- set line number where match occured*/
			break;
		}
		ptr = rdomain.s + len;
		lcount++;
	} /*- for (count = lcount = len = 0, ptr = rdomain.s; len < rdomain.len;) */
	if (!count) { /*- send it using qmail-remote */
		if (!stralloc_copys(&q, auto_prefix) ||
				!stralloc_catb(&q, "/sbin/qmail-remote", 18) ||
				!stralloc_0(&q))
			temp_nomem();
		execv(q.s, argv); /* run qmail-remote with original arguments */
		out("ZUnable to run qmail-remote (#4.3.0)\n");
		zerodie();
	}
	if (!stralloc_catb(&dto, prefix ? prefix : sender.s, prefix ? str_len(prefix) : sender.len) ||
			!stralloc_append(&dto, "-") ||
			!stralloc_catb(&dto, recip.s, recip.len) ||
			!stralloc_append(&dto, "\n") ||
			!stralloc_0(&dto))
		temp_nomem();
	dto.len--;
	if (pipe(pipefd) == -1) {
		out("ZTrouble creating pipe. (#4.3.0)\n");
		zerodie();
	}
	substdio_fdbuf(&ssi, read, 0, ssibuf, sizeof(ssibuf));
	substdio_fdbuf(&sso, write, pipefd[1], ssobuf, sizeof(ssobuf));
	if (*dest_addr == '/') {
		if (pipe(errpipe) == -1) {
			out("ZTrouble creating pipe. (#4.3.0)\n");
			zerodie();
		}
		substdio_fdbuf(&sse, read, errpipe[0], ssebuf, sizeof(ssebuf));
	}
	switch ((pid = fork()))
	{
	case -1:
		out("Zqq unable to fork (#4.3.0)\n");
		zerodie();
		break;
	case 0:
		close(pipefd[1]);
		if (dup2(pipefd[0], 0) == -1) {
			out("Zqq unable to duplicate descriptor for input (#4.3.0)\n");
			zerodie();
		}
		if (*dest_addr == '/') {
			close(errpipe[0]);
			if (dup2(errpipe[1], 1) == -1) {
				out("Zqq unable to duplicate descriptor for output (#4.3.0)\n");
				zerodie();
			}
			if (dup2(errpipe[1], 2) == -1) {
				out("Zqq unable to duplicate descriptor for error (#4.3.0)\n");
				zerodie();
			}
			if (!stralloc_copys(&q, auto_prefix) ||
					!stralloc_catb(&q, "/bin/maildirdeliver", 19) ||
					!stralloc_0(&q))
				temp_nomem();
			mdargs[0] = q.s;
			mdargs[1] = dest_addr;
			execv(q.s, (char **) mdargs); /*- run maildirdeliver */
		} else {
			if (!stralloc_copys(&q, auto_prefix) ||
					!stralloc_catb(&q, "/sbin/qmail-remote", 18) ||
					!stralloc_0(&q))
				temp_nomem();
			qrargs[0] = q.s;
			qrargs[1] = dest_domain;
			qrargs[2] = argv[2];
			qrargs[3] = argv[3];
			scan_ulong(argv[4], &size);
			strnum[fmt_ulong(strnum, size + dto.len)] = 0;
			qrargs[4] = strnum;
			qrargs[5] = dest_addr;
			execv(q.s, (char **) qrargs); /*- run qmail-remote */
		}
		_exit(111);
	default:
		close(pipefd[0]);
		if (*dest_addr == '/')
			close(errpipe[1]);
		break;
	} /*- switch ((pid = fork())) */
	if (substdio_put(&sso, dto.s, dto.len) == -1)
		temp_read_msg();
	switch (substdio_copy(&sso, &ssi))
	{
	case -2: /*- read error */
		temp_read_msg();
	case -3: /*- write error */
		temp_write();
	}
	if (substdio_flush(&sso) == -1)
		temp_write();
	close(pipefd[1]);
	if (*dest_addr == '/') {
		if (getln(&sse, &line, &match, '\n') == -1)
			temp_read_maildirdeliver();
		close(errpipe[0]);
	}
	if (wait_pid(&wstat, pid) != pid) {
		out("Zqmail-remote waitpid surprise (#4.3.0)\n");
		zerodie();
	}
	if (wait_crashed(wstat)) {
		out("Zqmail-remote crashed (#4.3.0)\n");
		zerodie();
	}
	if (*dest_addr == '/') {
		switch (wait_exitcode(wstat))
		{
		case 0:
			if (match)
				print_report("r", line.s, line.len);
			out("Kmail has been successfully redirected\n");
			zerodie();
		case 100:
			if (match)
				print_report("h", line.s, line.len);
			out("Dpermanent error while redirecting mail (#5.4.4)\n");
			zerodie();
		default:
			if (match)
				print_report("s", line.s, line.len);
			out("Zerror redirecting mail (#4.4.4)\n");
			zerodie();
		}
	} else
		_exit(wait_exitcode(wstat));
}

/*
 * $Log: rd-remote.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2023-12-06 17:01:44+05:30  Cprogrammer
 * added comment on report format for qmail-rspawn
 *
 * Revision 1.2  2023-12-06 15:28:10+05:30  Cprogrammer
 * added report for qmail-rspawn for maildir delivery
 *
 */
