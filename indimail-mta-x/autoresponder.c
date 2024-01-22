/*
 * $Id: autoresponder.c,v 1.41 2024-01-23 01:20:28+05:30 Cprogrammer Exp mbhangui $
 *
 * This is a simple program to automatically respond to emails.
 *
 * This is based on qmail-autoresponder
 * Rate-limited autoresponder for qmail
 * by Bruce Guenter <bruce@untroubled.org>
 * Version 2.0
 * 2018-04-13
 *
 * Original qmail-qfilter was based on some ideas (but little or no code)
 * from a similar autoresponder by Eric Huss <e-huss@netmeridian.com>,
 * and ideas presented in the qmail mailing list.
 */
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "substdio.h"
#ifndef _XOPEN_SOURCE
#define __USE_XOPEN
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include <now.h>
#include <sgetopt.h>
#include <subfd.h>
#include <mess822.h>
#include <date822fmt.h>
#include <getln.h>
#include <stralloc.h>
#include <env.h>
#include <case.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#include <wait.h>
#include <alloc.h>
#include <fmt.h>
#include <byte.h>
#include <tai.h>
#include <constmap.h>
#include <noreturn.h>
#include <mktempfile.h>
#include "hassrs.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "qregex.h"
#include "ip.h"
#include "ipme.h"
#include "quote.h"
#include "control.h"
#include "auto_sysconfdir.h"
#include "auto_prefix.h"
#include "buffer_defs.h"

#define strcasecmp(x,y)    case_diffs((x), (y))
#define strncasecmp(x,y,z) case_diffb((x), (z), (y))
#define FATAL "autoresponder: fatal: "
#define WARN  "autoresponder: warn: "

static int      opt_quiet, opt_copyinput, opt_nosend, opt_nolinks, msgfilefd, fixed_subject;
static int      liphostok, dtline_len, opt_laxmode;
unsigned long   opt_maxmsgs = 1;
time_t          when, opt_timelimit = 86400 * 7; /*- RFC 3834 */
static char    *opt_subject_prefix = "Autoreply: Re: ";
static char    *argv0, *dtline, *recipient, *from_addr = 0;
static char     ssinbuf[BUFSIZE_IN], ssoutbuf[BUFSIZE_OUT];
static char     strnum[FMT_ULONG];
substdio        ssin, ssout;
static pid_t    inject_pid;
static stralloc liphost = { 0 }, content_type = { 0 }, srs_domain = { 0 };
struct header
{
	char           *returnpath;
	char           *from;
	stralloc        to;
	stralloc        cc;
	stralloc        bcc;
	stralloc        resent_to;
	stralloc        resent_cc;
	stralloc        resent_bcc;
	char           *date;
	char           *bogosity;
	char           *mailer;
	char           *subject;
	char           *messageid;
	char           *importance;
};

struct header header = {
	0,
	0,
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	0,
	0,
	"unknown",
	"Your mail",
	0,
	0
};
char           *daytab[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char           *usage_str =
	" [OPTIONS] MSGFILE DIR\n"
	"\n"
	"OPTIONS\n"
	" -c       Enclose original message as a MIME attachment in response\n"
	" -q       Don't show routine error messages\n"
	" -L       Don't try to make links to conserve inodes\n"
	" -N       Don't send, just send autoresponse to standard output\n"
	" -l       Lax Mode, allow even if RECIPIENT is not in to, cc and bcc\n"
	" -n NUM   Set the maximum number of replies to be sent in TIME secs (defaults to 1)\n"
	" -t TIME  Set the time interval, in seconds (defaults to 1 week, -1 for no rate limiting)\n"
	" -T STR   Added Content-Type field. This option is valid without\n"
	"          the -c option. STR can be a path to a file having\n"
	"          the content-type. A newline at the end will be removed\n"
	" -s STR   Prefix subject by STR (defaults to \"Autoreply: Re:\"\n"
	" -S STR   The subject in autoresponse (defaults to the subject in received mail)\n"
	" -b SDate Date (YYYY-mm-dd HH:MM:SS) from which the autoresponder should start functioning\n"
	" -e EDate Date (YYYY-mm-dd HH:MM:SS) after which the autoresponder should stop functioning\n"
	" -f addr  Set addr as the From field in autoresponse (defaults to recipient)\n"
	"\n"
	" MSGFILE is a file containing autoresponse text.\n"
	" Temporary files put into DIR track senders' rates.\n"
	" If more than NUM messages are received from the same sender\n"
	" within TIME seconds of each other, no response is sent.\n"
	" This program must be run by qmail";

/*- Ignore message and do not respond */
no_return void
ignore(char *msg)
{
	if (opt_quiet)
		_exit(0);
	if (msg) {
		substdio_puts(subfderr, argv0);
		substdio_puts(subfderr, ": Ignoring message, ");
		substdio_puts(subfderr, msg);
		substdio_puts(subfderr, "\n");
		substdio_flush(subfderr);
	}
	_exit(0);
}

void
store_addr(stralloc *addrlist, char *line)
{
	char           *start, *ptr;

	for(start = line;*start && isspace((int) *start);start++);
	for (ptr = start;*ptr;ptr++) {
		if (*ptr == ',' || *ptr == ';') {
			*ptr = 0;
			if (!stralloc_cats(addrlist, start) ||
					!stralloc_0(addrlist))
				strerr_die2x(111, FATAL, "out of memory");
			for(start = ptr + 1;*start && isspace((int) *start);start++);
		}
	}
	if (!stralloc_cats(addrlist, start) ||
			!stralloc_0(addrlist))
		strerr_die2x(111, FATAL, "out of memory");
	return;
}

stralloc        addr = { 0 }; /*- will be 0-terminated, if addrparse returns 1 */

/*
 * Taken from qmail-smtpd.c and
 * modified
 */
int
addrparse(arg)
	char           *arg;
{
	int             i, flagesc, flagquoted;
	ip_addr         ip;
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
		strerr_die2x(111, FATAL, "out of memory");
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) {	/*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				strerr_die2x(111, FATAL, "out of memory");
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
					strerr_die2x(111, FATAL, "out of memory");
			}
		}
	}
	if (!stralloc_append(&addr, ""))
		strerr_die2x(111, FATAL, "out of memory");
	if (liphostok) {
		if ((i = byte_rchr(addr.s, addr.len, '@')) < addr.len) {
			/*- if not, partner should go read rfc 821 */
			if (addr.s[i + 1] == '[') {
				if (!addr.s[i + 1 + ip4_scanbracket(addr.s + i + 1, &ip)]) {
					if (ipme_is(&ip)) {
						addr.len = i + 1;
						if (!stralloc_cat(&addr, &liphost) ||
								!stralloc_0(&addr))
							strerr_die2x(111, FATAL, "out of memory");
					}
				}
			}
		}
	}
	if (addr.len > 900)
		return 0;
	return 1;
}

int
match_addr(stralloc *addrlist, char *email_addr)
{
	char           *ptr;
	int             len;
	stralloc        quoted = { 0 };

	if (!addrlist->s)
		return(0);
	if (!quote2(&quoted, email_addr))
		strerr_die2x(111, FATAL, "out of memory");
	if (!stralloc_0(&quoted))
		strerr_die2x(111, FATAL, "out of memory");
	for (len = 0, ptr = addrlist->s;len < addrlist->len;) {
		if (!strcasecmp(ptr, email_addr))
			return(1);
		len += (str_len(ptr) + 1);
		if (str_start(ptr, "recipient list not shown: ")) /*- Bcc */
			return(1);
		if (!addrparse(ptr))
			continue;
		if (!strncasecmp(addr.s, quoted.s, quoted.len))
			return(1);
		ptr = addrlist->s + len;
	}
	return(0);
}

no_return void
usage(char *msg)
{
	if (msg) {
		substdio_puts(subfderr, argv0);
		substdio_puts(subfderr, ": ");
		substdio_puts(subfderr, msg);
		substdio_puts(subfderr, "\n");
	}
	substdio_puts(subfderr, "USAGE: ");
	substdio_puts(subfderr, argv0);
	substdio_puts(subfderr, usage_str);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	_exit(111);
}

void
assign_datetime(struct datetime *dt, struct tm *tm)
{
	dt->year = tm->tm_year;
	dt->mon = tm->tm_mon;
	dt->yday = tm->tm_yday;
	dt->mday = tm->tm_mday;
	dt->wday = tm->tm_wday;
	dt->hour = tm->tm_hour;
	dt->min = tm->tm_min;
	dt->sec = tm->tm_sec;
}

void
get_arguments(int argc, char *argv[])
{
	char           *ptr;
	int             ch, i;
	struct tm       tm;

	for (ch = 0, i = 0, ptr = argv[0];*ptr;i++, ptr++) {
		if (*ptr == '/')
			ch = i;
	}
	if (i)
		argv0 = argv[0] + ch + 1;
	else
		argv0 = argv[0];
	when = now();
	while ((ch = getopt(argc, argv, "cn:s:S:t:T:f:b:e:qlLN")) != opteof) {
		switch (ch)
		{
		case 'b':
			/*- YYYY-mm-dd HH:MM:SS */
			ptr = strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm);
			if (!ptr || (ptr && *ptr)) {
				substdio_puts(subfderr, "Invalid date ");
				substdio_puts(subfderr, "[");
				substdio_puts(subfderr, optarg);
				substdio_puts(subfderr, "] ");
				substdio_puts(subfderr, "in SDATE\n");
				usage(0);
			}
			if (when < mktime(&tm))
				ignore("autoresponder yet to mature");
			break;
		case 'e':
			/*- YYYY-mm-dd HH:MM:SS */
			ptr = strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm);
			if (!ptr || (ptr && *ptr)) {
				substdio_puts(subfderr, "Invalid date ");
				substdio_puts(subfderr, "[");
				substdio_puts(subfderr, optarg);
				substdio_puts(subfderr, "] ");
				substdio_puts(subfderr, "in EDATE\n");
				usage(0);
			}
			if (when > mktime(&tm))
				ignore("autoresponder expired");
			break;
		case 'c':
			opt_copyinput = 1;
			break;
		case 'l':
			opt_laxmode = 1;
			break;
		case 'n':
			opt_maxmsgs = strtoul(optarg, &ptr, 10);
			if ((opt_maxmsgs == ULONG_MAX && errno == ERANGE)
				|| (!opt_maxmsgs && ptr == optarg) || *ptr) {
				substdio_puts(subfderr, "Invalid number ");
				if (*ptr) {
					substdio_puts(subfderr, "[");
					substdio_puts(subfderr, ptr);
					substdio_puts(subfderr, "] ");
				}
				substdio_puts(subfderr, "in NUM\n");
				usage(0);
			}
			break;
		case 'q':
			opt_quiet = 1;
			break;
		case 's':
			opt_subject_prefix = optarg;
			fixed_subject = 0;
			break;
		case 'S':
			fixed_subject = 1;
			header.subject = optarg;
			break;
		case 'f':
			from_addr = optarg;
			break;
		case 'T':
			if (!optarg[i = str_chr(optarg, '/')]) {
				if (!stralloc_copys(&content_type, optarg))
					strerr_die2x(111, FATAL, "out of memory");
			} else { /*- content-type is in a file */
				if (control_readnativefile(&content_type, optarg, 0) != 1)
					strerr_die3sys(111, FATAL, optarg, ": ");
				if (content_type.s[content_type.len - 1] == '\n')
					content_type.len--;
			}
			break;
		case 't':
			opt_timelimit = strtoul(optarg, &ptr, 10);
			if ((opt_timelimit == ULONG_MAX && errno == ERANGE)
				|| (!opt_timelimit && ptr == optarg) || *ptr) {
				substdio_puts(subfderr, "Invalid number ");
				if (*ptr) {
					substdio_puts(subfderr, "[");
					substdio_puts(subfderr, ptr);
					substdio_puts(subfderr, "] ");
				}
				substdio_puts(subfderr, "in TIME\n");
				usage(0);
			}
			break;
		case 'L':
			opt_nolinks = 1;
			break;
		case 'N':
			opt_nosend = 1;
			break;
		default:
			usage(0);
		}
	}
	if (argc - optind < 2)
		usage("Too few command-line arguments");
	if (argc - optind > 2)
		usage("Too many command-line arguments");
	if ((msgfilefd = open(argv[optind], O_RDONLY)) == -1)
		usage("Could not open message file.");
	if (chdir(argv[optind + 1]) == -1)
		usage("Could not change directory to DIRECTORY");
}

static stralloc badrcptto = { 0 };
static stralloc badrcptpatterns = { 0 };
struct constmap mapbadrcpt;

void
check_sender(stralloc *sender)
{
	int             i;
	char           *errStr = 0;

	/*- Ignore messages with an empty SENDER (sent from system) */
	if (str_equal(sender->s, "mailer-daemon"))
		ignore("SENDER was mailer-daemon");
	if (!sender->s[0])
		ignore("SENDER is empty, mail came from system account");
	if (str_equal(sender->s, "#@[]"))
		ignore("SENDER is <#@[]> (double bounce message)");
	if (!sender->s[i = str_chr(sender->s, '@')])
		ignore("SENDER did not contain a hostname");
	if (i > 8 && !str_diffn(sender->s + i - 8, "-request", 8))
		ignore("SENDER was owner list");
	if (!str_diffn(sender->s, "owner-", 6))
		ignore("SENDER was owner list");
	if ((control_readfile(&badrcptto, "badrcptto", 0)) == -1)
		strerr_die2sys(111, FATAL, "badrcptto: ");
	if ((control_readfile(&badrcptpatterns, "badrcptpatterns", 0)) == -1)
		strerr_die2sys(111, FATAL, "badrcptto: ");
	if (!constmap_init(&mapbadrcpt, badrcptto.s, badrcptto.len, 0))
		strerr_die2x(111, FATAL, "out of memory");
	switch (address_match("badrcptto", sender, &badrcptto, &mapbadrcpt, &badrcptpatterns, &errStr))
	{
	case 1:
		/*- match occurred */
		ignore("SENDER is blacklisted");
		return;
	case 0:
		break;
	case -1:
		strerr_die2x(111, FATAL, "out of memory");
	case -2:
		strerr_die2sys(111, FATAL, "unable to read badrcpto.cdb: ");
	case -3:
		strerr_die2sys(111, FATAL, "unable to lseek: ");
	case -4:
		strerr_die4sys(111, FATAL, "regex compilation failed: ", errStr, ": ");
	default:
		strerr_die4sys(111, FATAL, "unknown error: ", errStr, ": ");
		break;
	}
}

void
parse_header(char *str, unsigned length)
{
	int             pos, len;
	char          **ptr;
	char           *headertab[] = {
		"Return-Path:", /*-  0 */
		"From:",        /*-  1 */
		"To:",          /*-  2 */
		"Cc:",          /*-  3 */
		"Bcc:",         /*-  4 */
		"Resent-To:",   /*-  5 */
		"Resent-Cc:",   /*-  6 */
		"Resent-Bcc:",  /*-  7 */
		"Date:",        /*-  8 */
		"X-Bogosity:",  /*-  9 */
		"X-Mailer:",    /*- 10 */
		"Subject:",     /*- 11 */
		"Message-ID:",  /*- 12 */
		"Importance:",  /*- 13 */
		"User-Agent:",  /*- 14 */
		0
	};

	if (!strncasecmp(str, "List-ID:", 8)) /*- More cases to be added for List-* */
		ignore("Message appears to be from a mailing list (List-ID header)");
	else
	if (!strncasecmp(str, "Mailing-List:", 13))
		ignore("Message appears to be from a mailing list (Mailing-List header)");
	else
	if (!strncasecmp(str, "X-Mailing-List:", 15))
		ignore("Message appears to be from a mailing list (X-Mailing-List header)");
	else
	if (!strncasecmp(str, "X-ML-Name:", 10))
		ignore("Message appears to be from a mailing list (X-ML-Name header)");
	else
	if (!strncasecmp(str, dtline, dtline_len))
		ignore("Message already has my Delivered-To line");
	else
	if (!strncasecmp(str, "Auto-Submitted:", 15)) /*- RFC 3834 */ {
		char           *start = str + 15;
		char           *end;

		while (start < str + length && isspace((int) *start))
			++start;
		end = start;
		while (end < str + length && !isspace((int) *end))
			++end;
		if (!strncasecmp(start, "auto-replied", 12))
			ignore("Message has a Auto-Submitted header as auto-replied");
		else
		if (strncasecmp(start, "no", 2))
			ignore("Message does not haveAuto-Submitted header as \"no\"");
	} else
	if (!strncasecmp(str, "Precedence:", 11)) {
		char           *start = str + 11;
		char           *end;

		while (start < str + length && isspace((int) *start))
			++start;
		end = start;
		while (end < str + length && !isspace((int) *end))
			++end;
		if (!strncasecmp(start, "junk", end - start) || !strncasecmp(start, "bulk", end - start) ||
			!strncasecmp(start, "list", end - start))
			ignore("Message has a junk, bulk, or list precedence header");
	} else
	for(pos = 0, ptr = headertab;*ptr;ptr++,pos++) {
		len = str_len(*ptr);
		if (!strncasecmp(str, *ptr, len)) {
			char           *start = str + len;
			char           *end = str + length - 1;
			char           *cptr1, *cptr2;
			int             i = 0;

			while (start < str + length && isspace((int) *start))
				++start;
			while (end > start && isspace((int) *end)) {
				--end;
				i++;
			}
			if (!(cptr1 = alloc(length - (start - str) - i + 1)))
				strerr_die2x(111, FATAL, "out of memory");
			for(cptr2 = cptr1;start != end + 1;start++)
				*cptr2++ = *start;
			*cptr2 = 0;
			switch (pos)
			{
			case 0:
				header.returnpath = cptr1;
				break;
			case 1:
				header.from = cptr1;
				break;
			case 2:
				store_addr(&header.to, cptr1);
				break;
			case 3:
				store_addr(&header.cc, cptr1);
				break;
			case 4:
				store_addr(&header.bcc, cptr1);
				break;
			case 5:
				store_addr(&header.resent_to, cptr1);
				break;
			case 6:
				store_addr(&header.resent_cc, cptr1);
				break;
			case 7:
				store_addr(&header.resent_bcc, cptr1);
				break;
			case 8:
				header.date = cptr1;
				break;
			case 9:
				header.bogosity = cptr1;
				break;
			case 10: /*- X-Mailer */
				header.mailer = cptr1;
				break;
			case 11:
				header.subject = cptr1;
				break;
			case 12:
				header.messageid = cptr1;
				break;
			case 13:
				header.importance = cptr1;
				break;
			case 14: /*- User-Agent */
				header.mailer = cptr1;
				break;
			} /*- switch (pos) */
		} /*- if (!strncasecmp(str, *ptr, len)) */
	} /*- for(pos = 0, ptr = headertab;*ptr;ptr++,pos++) */
	return;
}

int
popen_inject(char *sender)
{
	char           *(args[6]);
	stralloc        bin = {0};
	int             fds[2];

	if (pipe(fds) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	inject_pid = fork();
	switch (inject_pid)
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
		break;
	case 0:
		if (chdir("/"))
			strerr_die2sys(111, FATAL, "unable to chdir to root: ");
		if (!stralloc_copys(&bin, auto_prefix) ||
				!stralloc_catb(&bin, "/bin/qmail-inject", 17) ||
				!stralloc_0(&bin))
			strerr_die2x(111, FATAL, "out of memory");
		args[0] = bin.s;
		args[1] = "-a";
		args[2] = "-f";
		args[3] = "";
		args[4] = sender;
		args[5] = 0;
		close(fds[1]);
		if (!env_unset("QMAILNAME"))
			strerr_die2x(111, FATAL, "out of memory");
		if (!env_unset("QMAILUSER="))
			strerr_die2x(111, FATAL, "out of memory");
		if (!env_unset("QMAILHOST="))
			strerr_die2x(111, FATAL, "out of memory");
		close(0);
		dup2(fds[0], 0);
		if (fds[0])
			close(fds[0]);
		execv(*args, args);
		strerr_die4sys(111, FATAL, "unable to exec ", *args, ": ");
		break;
	}
	close(fds[0]);
	return fds[1];
}

void
pclose_inject(int fdout)
{
	int             i, werr, status;

	close(fdout);
	for (;;) {
		if (!(i = waitpid(inject_pid, &status, 0)))
			break;
		else
		if (i == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			strerr_die2sys(111, FATAL, "waitpid: ");
		}
		if (i != inject_pid)
			strerr_die2x(111, FATAL, "waitpid surprise");
		if (!(i = wait_handler(status, &werr)) && werr)
			continue;
		else
		if (werr == -1)
			strerr_die2x(111, FATAL, "internal wait handler error");
		else
		if (werr) {
			strnum[fmt_int(strnum, werr)] = 0;
			strerr_die3x(111, FATAL, "qmail-inject killed by signal ", strnum);
		} else
		if (i) {
			strnum[fmt_int(strnum, i)] = 0;
			strerr_die3x(111, FATAL, "qmail-inject exit code ", strnum);
		}
		break;
	}
}

void
parseMessage_file(stralloc *line, struct datetime *dt, char *ldbuf)
{
	static int      saw_percent = 0;
	ssize_t         todo, incr;
	unsigned long   len;
	char           *next, *buf;

	buf = line->s;
	len = line->len;
	while (len > 0) {
		if (!(next = str_chrn(buf, '%', len))) {
			if (substdio_put(&ssout, buf, len))
				strerr_die2sys(111, FATAL, "unable to write: ");
			return;
		}
		todo = next - buf;
		incr = todo + 1;
		saw_percent = 1;
		if (substdio_put(&ssout, buf, todo))
			strerr_die2sys(111, FATAL, "unable to write: ");
		len -= incr;
		if (!len)
			return;
		buf += incr;
		if (saw_percent) {
			saw_percent = 0;
			switch (*buf)
			{
			case 'R': /*- Return-Path: */
				if (header.returnpath && substdio_puts(&ssout, header.returnpath))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'F': /*- From: */
				if (header.from && substdio_puts(&ssout, header.from))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'T': /*- To: Envelope Recipient */
				if (header.to.len && substdio_puts(&ssout, header.to.s))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 't': /*- Recipient */
				if (substdio_puts(&ssout, recipient))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'C': /*- Cc: Envelope Recipient */
				if (header.cc.len && substdio_puts(&ssout, header.cc.s))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'B': /*- Bcc: Envelope Recipient */
				if (header.bcc.len && substdio_puts(&ssout, header.bcc.s))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'd': /*- current date */
				if (substdio_puts(&ssout, daytab[dt->wday]))
					strerr_die2sys(111, FATAL, "unable to write: ");
				if (substdio_put(&ssout, ", ", 2))
					strerr_die2sys(111, FATAL, "unable to write: ");
				if (substdio_puts(&ssout, ldbuf))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				continue;
			case 'D': /*- Date: */
				if (header.date) {
					if (substdio_puts(&ssout, header.date))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
				++buf;
				--len;
				continue;
			case 'r': /*- X-Bogosity: */
				if (header.bogosity) {
					if (substdio_puts(&ssout, header.bogosity))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
				++buf;
				--len;
				continue;
			case 'M': /*- X-Mailer: */
				if (header.mailer) {
					if (substdio_puts(&ssout, header.mailer))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
				++buf;
				--len;
				continue;
				break;
			case 'S': /*- Subject */
				if (header.subject) {
					if (substdio_puts(&ssout, header.subject))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
				++buf;
				--len;
				continue;
			case 'm': /*- MessageID: */
				if (header.messageid) {
					if (substdio_puts(&ssout, header.messageid))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
				++buf;
				--len;
				continue;
				break;
			case '%':
				if (substdio_puts(&ssout, "%"))
					strerr_die2sys(111, FATAL, "unable to write: ");
				++buf;
				--len;
				break;
			default:
				if (substdio_puts(&ssout, "%"))
					strerr_die2sys(111, FATAL, "unable to write: ");
				break;
			}
		}
	}
	return;
}

int
count_history(char *sender, unsigned max)
{
	DIR            *dir = opendir(".");
	struct dirent  *entry;
	char           *ptr, *end, *filename, *sender_copy, *last_filename = 0;
	int             fd;
	unsigned        message_pid, count = 0;
	pid_t           pid;
	size_t          sender_len, i;
	time_t          message_time;

	/*
	 * Translate all '/' to ':', to avoid fake paths in email addresses
	 */
	sender_len = str_len(sender);
	sender_copy = alloc(sender_len + 1);
	for (i = 0; i < sender_len; i++)
		sender_copy[i] = (sender[i] == '/') ? ':' : sender[i];
	sender_copy[i] = 0;
	/*
	 * create the filename, format "PID.TIME.SENDER"
	 * The PID is added to avoid collisions.
	 */
	filename = (char *) alloc(fmt_ulong(0, (pid = getpid())) + fmt_ulong(0, when) + 2 + sender_len + 1);
	ptr = filename;
	i = fmt_ulong(ptr, pid);
	ptr += i;
	i = fmt_str(ptr, ".");
	ptr += i;
	i = fmt_ulong(ptr, when);
	ptr += i;
	i = fmt_str(ptr, ".");
	ptr += i;
	i = fmt_str(ptr, sender_copy);
	ptr += i;
	*ptr = 0;
	/*
	 * check if there are too many responses in the logs
	 */
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		message_pid = strtoul(entry->d_name, &ptr, 10);
		if (message_pid == 0 || *ptr != '.')
			continue;
		message_time = strtoul(ptr + 1, &end, 10);
		if (!end || *end != '.')
			continue;
		if (when - message_time > opt_timelimit) /*- too old..ignore errors on unlink */
			unlink(entry->d_name);
		else {
			/*
			 * If the user's count is already over the max,
			 * don't record any more.
			 */
			if (str_equal(end + 1, sender_copy) && ++count >= max)
				return 1;
			last_filename = entry->d_name;
		}
	}
	/*
	 * Conserve inodes -- create links when possible
	 */
	if (last_filename && !opt_nolinks && !link(last_filename, filename))
		*filename = 0;
	/*- Otherwise, create a new 0-byte file now */
	if (*filename) {
		if ((fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0444)) == -1) {
			alloc_free(filename);
			strerr_die4sys(111, FATAL, "unable to create file ", filename, ": ");
		}
		close(fd);
	}
	alloc_free(filename);
	return 0;
}

/*
 * qmail-autoresponder -c -N -n 3 /tmp/test.msg /tmp/abcd < ../scripts/mail.msg |less
 */
int
main(int argc, char *argv[])
{
	int             fdout, match, len, do_srs = 0, at;
	char           *sender, *dtrecip, *host, *ptr;
	char            dbuf[DATE822FMT];
#ifdef MIME
	char            num[FMT_ULONG];
	struct tai      datetai;
	stralloc        boundary = { 0 };
#endif
	stralloc        mailfrom = { 0 };
	stralloc        line = { 0 };
	stralloc        bouncefrom = { 0 };
	stralloc        bouncehost = { 0 };
	stralloc        doublebounceto = { 0 };
	stralloc        doublebouncehost = { 0 };
#ifdef QUOTE
	stralloc        quoted = { 0 };
#endif
	struct datetime dt;

	if (chdir(auto_sysconfdir))
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_sysconfdir, ": ");
	if (control_init() == -1)
		strerr_die2sys(111, FATAL, "unable to read init controls: ");
	if (control_rldef(&bouncefrom, "bouncefrom", 0, "MAILER-DAEMON") != 1)
		strerr_die2sys(111, FATAL, "unable to read bouncefrom controls: ");
	if (control_rldef(&bouncehost, "bouncehost", 1, "bouncehost") != 1)
		strerr_die2sys(111, FATAL, "unable to read bouncehost controls: ");
	if (control_rldef(&doublebouncehost, "doublebouncehost", 1, "doublebouncehost") != 1)
		strerr_die2sys(111, FATAL, "unable to read doublebouncehost controls: ");
	if (control_rldef(&doublebounceto, "doublebounceto", 0, "postmaster") != 1)
		strerr_die2sys(111, FATAL, "unable to read doublebounceto controls: ");
	if ((liphostok = control_rldef(&liphost, "localiphost", 1, (char *) 0)) == -1)
		strerr_die2sys(111, FATAL, "unable to read localiphost controls: ");
	if (control_readline(&srs_domain, (ptr = env_get("SRS_DOMAIN")) && *ptr ? ptr : (ptr = "srs_domain")) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", ptr, " controls: ");
	if (srs_domain.len && !stralloc_0(&srs_domain))
		strerr_die2x(111, FATAL, "out of memory");
	srs_domain.len--; /*- substract length due to stralloc_0 */
	if (mktempfile(0))
		strerr_die2sys(111, FATAL, "mktempfile: ");
	get_arguments(argc, argv);
	/*- Fail if SENDER or DTLINE are not set */
	if (!(sender = env_get("SENDER")))
		usage("SENDER is not set, must be run from .qmail");
	if (!(dtrecip = dtline = env_get("DTLINE")))
		usage("DTLINE is not set; must be run from .qmail");
	dtline_len = str_len(dtline);
#ifdef HAVESRS
	if (srs_domain.len && (case_starts(sender, "SRS0=") || case_starts(sender, "SRS1="))) {
		len = str_len(sender);
		if ((at = byte_rchr(sender, len, '@')) < len - 2) {
			if (!str_diffn(srs_domain.s, sender + at + 1,
						srs_domain.len > (len - at - 1) ? srs_domain.len + 1 : len - at))
				do_srs = 1;
		}
	}
	if (do_srs) {
		switch (srsreverse(sender))
		{
		case -3:
			strerr_die2x(100, FATAL, srs_error.s);
			break;
		case -2:
			strerr_die2x(111, FATAL, "out of memory");
			break;
		case -1:
			strerr_die2x(111, FATAL, "unable to read controls");
			break;
		case 0:
			strerr_die2x(100, FATAL, "unable to rewrite envelope");
			break;
		}
		if (!str_diffn(srs_result.s, "#@[]", 4))
			strerr_warn2(WARN, "triple bounce: discarding message", 0);
		else
		if (!srs_result.len && *doublebounceto.s == '@')
			strerr_warn2(WARN, "double bounce: discarding message", 0);
		sender = srs_result.s;
	}
#endif

	if (!(recipient = env_get("RECIPIENT")))
		usage("RECIPIENT is not set; must be run from .qmail");
	if (!(host = env_get("HOST")))
		usage("HOST is not set; must be run from .qmail");
	recipient += (match = str_len(host)) + 1; /*- example.com-mbhangui@example.com */
	dtrecip += match + 15; /*- Delivered-To: example.com-mbhangui@example.com */
	if (!str_diffn(recipient, dtrecip, str_len(recipient)))
		opt_laxmode = 1;

	if (!stralloc_copys(&mailfrom, sender) ||
			!stralloc_0(&mailfrom))
		strerr_die2x(111, FATAL, "out of memory");
	case_lowers(mailfrom.s);
	check_sender(&mailfrom);
	/*- Read and parse header */
	if (lseek(0, 0, SEEK_SET) == -1)
		strerr_die2sys(111, FATAL, "unable to lseek: ");
	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read headers: ");
		if (!mess822_ok(&line))
			break;
		if (!match && line.len == 0)
			break;
		parse_header(line.s, line.len);
	}
	if (!opt_laxmode && !match_addr(&header.to, recipient) && !match_addr(&header.cc, recipient)
		&& !match_addr(&header.bcc, recipient) && !match_addr(&header.resent_to, recipient)
		&& !match_addr(&header.resent_cc, recipient) && !match_addr(&header.resent_bcc, recipient))
		ignore("Message header does not contain recipient");
	if (header.bogosity && !strncasecmp(header.bogosity, "Yes,", 4))
		ignore("Message is spam"); /*- RFC 3834 */
	if (opt_nosend)
		fdout = 1;
	else {
		/*- Check rate that SENDER has sent */
		if (count_history(mailfrom.s, opt_maxmsgs))
			ignore("SENDER has sent too many messages");
		fdout = popen_inject(sender);
	}
	substdio_fdbuf(&ssout, write, fdout, ssoutbuf, sizeof(ssoutbuf));

	/*- Auto-Submitted Field - RFC 3834 */
	if (substdio_put(&ssout, "Auto-Submitted: auto-replied\n", 29))
		strerr_die2sys(111, FATAL, "unable to write: ");

	/*- Date Field */
	datetime_tai(&dt, when);
	dbuf[date822fmt(dbuf, &dt)] = 0;
	if (substdio_put(&ssout, "Date: ", 6))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_puts(&ssout, dbuf))
		strerr_die2sys(111, FATAL, "unable to write: ");

	/* The From: Address */
	if ((ptr = env_get("SERVICE_RESPONDER"))) {
		if (substdio_puts(&ssout, "From: ") == -1)
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, *ptr ? ptr : "AutoResponse") == -1)
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, " <") == -1)
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, bouncefrom.s, bouncefrom.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, "@"))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, bouncehost.s, bouncehost.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, ">\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
	} else {
		if (substdio_puts(&ssout, "From: ") == -1)
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (!from_addr)
			from_addr = recipient;
#ifdef QUOTE
		if (!quote2(&quoted, from_addr))
			strerr_die2x(111, FATAL, "out of memory");
		if (substdio_put(&ssout, quoted.s, quoted.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
#else
		if (substdio_puts(&ssout, from_addr))
			strerr_die2sys(111, FATAL, "unable to write: ");
#endif
		if (substdio_puts(&ssout, "\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
	}

	/*- To Field */
	if (substdio_put(&ssout, "To: ", 4))
		strerr_die2sys(111, FATAL, "unable to write: ");
#ifdef QUOTE
	if (!quote2(&quoted, sender))
		strerr_die2x(111, FATAL, "out of memory");
	if (substdio_put(&ssout, quoted.s, quoted.len))
		strerr_die2sys(111, FATAL, "unable to write: ");
#else
	if (substdio_puts(&ssout, sender))
		strerr_die2sys(111, FATAL, "unable to write: ");
#endif
	if (substdio_put(&ssout, "\n", 1))
		strerr_die2sys(111, FATAL, "unable to write: ");

	if (header.messageid) /*- RFC 3834 */ {
		/*- In-Reply-To Field */
		if (substdio_put(&ssout, "In-Reply-To: ", 13))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, header.messageid))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, "\n", 1))
			strerr_die2sys(111, FATAL, "unable to write: ");

		/*- References Field */
		if (substdio_put(&ssout, "References: ", 12))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, header.messageid))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, "\n", 1))
			strerr_die2sys(111, FATAL, "unable to write: ");
	}

#ifdef MIME
	if (opt_copyinput) {
		tai_now(&datetai);
		if (substdio_puts(&ssout, "Mime-Version: 1.0\n"
			"Content-Type: multipart/mixed;\n"
			"\tboundary=\""))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (!stralloc_copyb(&boundary, num, fmt_ulong(num, datetai.x)) ||
				!stralloc_cats(&boundary, ".qp_") ||
				!stralloc_catb(&boundary, num, fmt_ulong(num, getpid())) ||
				!stralloc_cats(&boundary, ".KUI@") ||
				!stralloc_catb(&boundary, bouncehost.s, bouncehost.len))
			strerr_die2x(111, FATAL, "out of memory");
		if (substdio_put(&ssout, boundary.s, boundary.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, "\"\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
	} else
	if (content_type.len) {
		if (substdio_put(&ssout, "Mime-Version: 1.0\n", 18))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, content_type.s, content_type.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, "\n", 1))
			strerr_die2sys(111, FATAL, "unable to write: ");
	}
	if (substdio_puts(&ssout, "Subject: "))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (opt_subject_prefix && substdio_puts(&ssout, opt_subject_prefix))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_puts(&ssout, header.subject)) /*- original subject */
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_puts(&ssout, "\n\n"))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (opt_copyinput) {
		if (substdio_puts(&ssout, "This is a multi-part message in MIME format.\n\n--"))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, boundary.s, boundary.len))	/* def type is text/plain */
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, "\n\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
	}
#else
	if (substdio_puts(&ssout, "Subject: "))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (opt_subject_prefix && substdio_puts(&ssout, opt_subject_prefix))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_puts(&ssout, header.subject)) /*- original subject */
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_puts(&ssout, "\n\n"))
		strerr_die2sys(111, FATAL, "unable to write: ");
#endif
	/*- Copy the autoresponse text */
	substdio_fdbuf(&ssin, read, msgfilefd, ssinbuf, sizeof(ssinbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read autoresponse text: ");
		if (!match && line.len == 0)
			break;
		parseMessage_file(&line, &dt, dbuf);
	}
	close(msgfilefd);
	if (opt_copyinput) {
#ifdef MIME
		if (substdio_puts(&ssout, "\n--- Enclosed below is a copy of the message.\n\n--"))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, boundary.s,boundary.len))	/* enclosure boundary */
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, "\nContent-Type: message/rfc822\n\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
#else
		if (substdio_puts(&ssout, "\n--- Below this line is a copy of the message.\n\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
#endif
		/*- Insert the original message */
		if (lseek(0, 0, SEEK_SET) == -1)
			strerr_die2sys(111, FATAL, "unable to lseek: ");
		substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
		switch (substdio_copy(&ssout, &ssin))
		{
		case -2: /*- read error */
			strerr_die2sys(111, FATAL, "read error: ");
		case -3: /*- write error */
			strerr_die2sys(111, FATAL, "write error: ");
		}
#ifdef MIME
		if (substdio_puts(&ssout, "\n--")) /* end boundary */
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_put(&ssout, boundary.s, boundary.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (substdio_puts(&ssout, "--\n\n"))
			strerr_die2sys(111, FATAL, "unable to write: ");
#endif
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (!opt_nosend)
		pclose_inject(fdout);
	return 0;
}

void
getversion_qmail_autoresponder_c()
{
	static char    *x = "$Id: autoresponder.c,v 1.41 2024-01-23 01:20:28+05:30 Cprogrammer Exp mbhangui $";

	x++;
	x = sccsidmktempfileh;
	x++;
}

/*
 * $Log: autoresponder.c,v $
 * Revision 1.41  2024-01-23 01:20:28+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.40  2023-10-25 13:25:55+05:30  Cprogrammer
 * rewind descriptor 0 regardless of MAKE_SEEKABLE setting
 *
 * Revision 1.39  2023-10-02 17:40:58+05:30  Cprogrammer
 * decode srs return path address
 *
 * Revision 1.38  2023-03-26 01:52:43+05:30  Cprogrammer
 * fixed code using wait_handler.
 *
 * Revision 1.37  2022-12-18 12:31:42+05:30  Cprogrammer
 * handle wait status with details
 *
 * Revision 1.36  2022-01-30 08:28:42+05:30  Cprogrammer
 * removed chdir auto_qmail
 *
 * Revision 1.35  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.34  2021-07-19 07:57:58+05:30  Cprogrammer
 * fixed compiler warning on _XOPEN_SOURCE redefinition
 *
 * Revision 1.33  2021-06-14 00:37:37+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.32  2017-03-09 14:35:38+05:30  Cprogrammer
 * ignore responses to senders listed in badrcptto
 *
 * Revision 1.31  2015-12-12 15:01:41+05:30  Cprogrammer
 * fix for deliver to email alias
 *
 * Revision 1.30  2015-08-24 19:04:55+05:30  Cprogrammer
 * changed ip_scanbracket to ip4_scanbracket
 *
 * Revision 1.29  2014-05-22 14:45:38+05:30  Cprogrammer
 * fixed issue with Date field
 * fixed SIGPIPE resulting in duplicate emails
 * use mktime to use local time
 *
 * Revision 1.28  2014-01-29 13:57:54+05:30  Cprogrammer
 * changed data type for opt_maxmsgs to unsigned long
 *
 * Revision 1.27  2012-08-16 07:58:56+05:30  Cprogrammer
 * added description for -T option
 *
 * Revision 1.26  2011-12-05 12:13:11+05:30  Cprogrammer
 * set Content-Type if -T argument is given
 *
 * Revision 1.25  2011-12-01 21:21:50+05:30  Cprogrammer
 * added option to add Content-Type header
 *
 * Revision 1.24  2011-11-06 22:54:53+05:30  Cprogrammer
 * fixed checking of strtoul function
 *
 * Revision 1.23  2009-01-06 20:47:48+05:30  Cprogrammer
 * corrected addrparse() function
 *
 * Revision 1.22  2008-08-03 18:25:57+05:30  Cprogrammer
 * use proper prot for strptime()
 *
 * Revision 1.21  2005-06-17 21:49:16+05:30  Cprogrammer
 * replaced struct_ipaddress with shorter typedef
 *
 * Revision 1.20  2005-03-03 14:36:24+05:30  Cprogrammer
 * added service responder
 *
 * Revision 1.19  2005-03-03 00:45:36+05:30  Cprogrammer
 * provide Auto-Submitted, In-Reply-To, References fields (RFC 3834)
 * fixed parsing of RECIPIENT environment variable
 *
 * Revision 1.18  2004-11-03 23:41:28+05:30  Cprogrammer
 * added option %d to add the current date
 * added User-Agent header
 *
 * Revision 1.17  2004-11-01 23:13:32+05:30  Cprogrammer
 * added start date and end date options
 *
 * Revision 1.16  2004-10-29 00:10:50+05:30  Cprogrammer
 * rfc 3834 implementation
 *
 * Revision 1.15  2004-10-24 21:29:28+05:30  Cprogrammer
 * removed fail_temp(), die_nomem()
 * corrected length of filename
 *
 * Revision 1.14  2004-10-24 20:09:41+05:30  Cprogrammer
 * djbfication of code
 *
 * Revision 1.13  2004-10-22 20:28:06+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.12  2004-07-15 23:35:15+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.11  2004-05-06 22:26:58+05:30  Cprogrammer
 * FROM field set incorrectly if domain contained '-' character
 *
 * Revision 1.10  2004-05-03 22:09:16+05:30  Cprogrammer
 * header was not delineated properly
 *
 * Revision 1.9  2003-12-07 13:01:20+05:30  Cprogrammer
 * added -S option to add a fixed subject
 *
 * TODO
 *
 * 1. Include more List- headers
 */
