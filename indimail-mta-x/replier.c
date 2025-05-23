/*
 * $Id: replier.c,v 1.15 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alloc.h>
#include <envdir.h>
#include <stralloc.h>
#include <byte.h>
#include <strerr.h>
#include <error.h>
#include <env.h>
#include <sig.h>
#include <open.h>
#include <getln.h>
#include <case.h>
#include <scan.h>
#include <str.h>
#include <fmt.h>
#include <substdio.h>
#include <constmap.h>
#include <fd.h>
#include <wait.h>
#include <pathexec.h>
#include <noreturn.h>
#include "getconf.h"
#include "qmail.h"
#include "set_environment.h"

#define FATAL "replier: fatal: "
#define WARN  "replier: warn: "

void (*sig_defaulthandler)(int) = SIG_DFL;
void (*sig_ignorehandler)(int) = SIG_IGN;

no_return void
usage()
{
	strerr_die1x(100, "replier: usage: replier dir addr prog [ args ]");
}

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
badaddr()
{
	strerr_die2x(100, FATAL, "I do not accept messages at this address (#5.1.1)");
}

static struct qmail qq;

ssize_t
mywrite(int fd, const char *buf, size_t len)
{
	qmail_put(&qq, buf, len);
	return len;
}

void
put(const char *buf, size_t len)
{
	qmail_put(&qq, buf, len);
}

void
myputs(const char *buf)
{
	qmail_puts(&qq, buf);
}

no_return void
sigalrm(int x)
{
	strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
}

int
main(int argc, char **argv)
{
	stralloc        mydtline = { 0 }, line = { 0 }, mailinglist = { 0 },
					inlocal = { 0 }, outlocal = { 0 }, outhost = { 0 },
					headerremove = { 0 }, headeradd = { 0 };
	struct constmap headerremovemap;
	char            inbuf[1024], outbuf[512], strnum[FMT_ULONG];
	substdio        ssin, ssout;
	char           *dir, *addr, *sender, *local, *action;
	const char     *qqx;
	char          **e;
	int             flagmlwasthere, match, i, flaginheader, flagbadfield, pid, wstat,
					tmperrno;
	int             pf[2];

	if (!(dir = argv[1]) ||
			!(addr = argv[2]))
		usage();
	umask(022);
	sig_ignore(SIGPIPE);
	sig_catch(SIGALRM, sigalrm);
	alarm(86400);
	set_environment(WARN, FATAL, 0);
	if (chdir(dir) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", dir, ": ");
	if ((sender = env_get("SENDER"))) {
		if (!*sender || str_equal(sender, "#@[]"))
			strerr_die2x(100, FATAL, "I don't reply to bounce messages (#5.7.2)");
	}
	if (!(local = env_get("LOCAL")))
		strerr_die2sys(111, FATAL, "LOCAL not set");
	getconf_line(&inlocal, "inlocal", 1, FATAL, dir);
	if (inlocal.len > str_len(local))
		badaddr();
	if (case_diffb(inlocal.s, inlocal.len, local))
		badaddr();
	action = local + inlocal.len;
	if (*action != '-')
		badaddr();
	++action;
	if (pipe(pf) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		close(pf[0]);
		if (fd_move(1, pf[1]) == -1)
			_exit(111);
		if (!pathexec_env("REQUEST", action))
			_exit(111);
		action += str_chr(action, '-');
		if (*action)
			++action;
		if (!pathexec_env("REQUEST2", action))
			_exit(111);
		action += str_chr(action, '-');
		if (*action)
			++action;
		if (!pathexec_env("REQUEST3", action))
			_exit(111);
		action += str_chr(action, '-');
		if (*action)
			++action;
		if (!pathexec_env("REQUEST4", action))
			_exit(111);
		if ((e = pathexec(argv + 3))) {
			tmperrno = errno;
			alloc_free((char *) e);
			errno = tmperrno;
		}
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	close(pf[1]);
	if (qmail_open(&qq) == -1)
		strerr_die2sys(111, FATAL, "unable to run qmail-queue: ");
	getconf_line(&outhost, "outhost", 1, FATAL, dir);
	getconf_line(&outlocal, "outlocal", 1, FATAL, dir);
	getconf_line(&mailinglist, "mailinglist", 1, FATAL, dir);
	getconf(&headerremove, "headerremove", 1, FATAL, dir);
	constmap_init(&headerremovemap, headerremove.s, headerremove.len, 0);
	getconf(&headeradd, "headeradd", 1, FATAL, dir);
	for (i = 0; i < headeradd.len; ++i)
		if (!headeradd.s[i])
			headeradd.s[i] = '\n';
	if (!stralloc_copys(&mydtline, "Delivered-To: replier ") ||
			!stralloc_cat(&mydtline, &outlocal) ||
			!stralloc_cats(&mydtline, action) ||
			!stralloc_cats(&mydtline, "@") ||
			!stralloc_catb(&mydtline, outhost.s, outhost.len) ||
			!stralloc_cats(&mydtline, "\n"))
		nomem();
	myputs("Mailing-List: ");
	put(mailinglist.s, mailinglist.len);
	myputs("\n");
	put(headeradd.s, headeradd.len);
	put(mydtline.s, mydtline.len);
	flagmlwasthere = 0;
	flaginheader = 1;
	flagbadfield = 0;
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, pf[0], inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (flaginheader && match) {
			if (line.len == 1)
				flaginheader = 0;
			if ((line.s[0] != ' ') && (line.s[0] != '\t')) {
				flagbadfield = 0;
				if (constmap(&headerremovemap, line.s, byte_chr(line.s, line.len, ':')))
					flagbadfield = 1;
				if (case_startb(line.s, line.len, "mailing-list:"))
					flagmlwasthere = 1;
				if (line.len == mydtline.len && !byte_diff(line.s, line.len, mydtline.s))
					strerr_die2x(100, FATAL, "this message is looping: it already has my Delivered-To line (#5.4.6)");
			}
		}
		if (!(flaginheader && flagbadfield))
			put(line.s, line.len);
		if (!match)
			break;
	}
	if (flagmlwasthere)
		strerr_die2x(100, FATAL, "message already has Mailing-List (#5.7.2)");
	if (!stralloc_copy(&line, &outlocal) ||
			!stralloc_cats(&line, "return-@") ||
			!stralloc_cat(&line, &outhost) ||
			!stralloc_0(&line))
		nomem();
	qmail_from(&qq, line.s);
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) mywrite, -1, outbuf, sizeof(outbuf));
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	if (substdio_flush(&ssout) == -1)
		strerr_die2sys(111, FATAL, "unable to write output: ");
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		_exit(0);
	}
	close(pf[0]);
	strnum[fmt_ulong(strnum, qmail_qp(&qq))] = 0;
	qmail_to(&qq, addr);
	qqx = qmail_close(&qq);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(99, "replier: qp ", strnum);
	/*- Not reached */
	return(0);
}

void
getversion_replier_c()
{
	const char     *x = "$Id: replier.c,v 1.15 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: replier.c,v $
 * Revision 1.15  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2022-10-17 19:45:16+05:30  Cprogrammer
 * collapsed multiple stralloc lines
 *
 * Revision 1.12  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.11  2021-07-05 21:11:41+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.10  2021-05-13 14:44:33+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.9  2020-11-24 13:47:58+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2020-04-04 12:48:43+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.7  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.6  2010-06-08 22:00:41+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.5  2008-07-15 20:04:44+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.4  2004-10-22 20:29:58+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:38:42+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.2  2004-10-09 19:21:47+05:30  Cprogrammer
 * moved sig_ignore() and sig_uncatch() to sig.h
 *
 * Revision 1.1  2004-07-17 21:00:25+05:30  Cprogrammer
 * Initial revision
 *
 */
