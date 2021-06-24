/*
 * $Log: qmail-showctl.c,v $
 * Revision 1.75  2021-06-24 12:17:02+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.74  2021-05-16 00:49:19+05:30  Cprogrammer
 * use configurable conf_split instead of auto_split variable
 *
 * Revision 1.73  2021-05-13 14:44:16+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.72  2021-05-12 15:50:52+05:30  Cprogrammer
 * set env variables from $HOME/.defaultqueue
 * set conf_split from CONFSPLIT env variable
 *
 * Revision 1.71  2021-02-07 23:14:18+05:30  Cprogrammer
 * declare verbose variable removed from indimail_stub
 *
 * Revision 1.70  2021-02-07 21:13:07+05:30  Cprogrammer
 * removed non-existent control file inquery
 *
 * Revision 1.69  2020-11-24 13:47:30+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.68  2020-05-11 10:59:57+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.67  2019-05-27 20:31:30+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.66  2019-05-27 12:39:52+05:30  Cprogrammer
 * set libfn with full path of libindimail control file
 *
 * Revision 1.65  2019-05-26 12:32:25+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.64  2019-04-20 19:52:13+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.63  2018-08-26 03:46:06+05:30  Cprogrammer
 * better indentation
 *
 * Revision 1.62  2018-07-01 11:49:55+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.61  2018-01-09 11:53:53+05:30  Cprogrammer
 * use loadLibrary() to load indimail code
 *
 * Revision 1.60  2017-08-26 09:17:42+05:30  Cprogrammer
 * fixed SIGSEGV
 *
 * Revision 1.59  2017-08-23 13:20:24+05:30  Cprogrammer
 * use tlsclientmethod, tlsservermethod control files only for openssl version < 1.0.1
 *
 * Revision 1.58  2017-03-10 11:30:22+05:30  Cprogrammer
 * added control files maxdeliveredto, tlsclientmethod, tlsservermethod
 *
 * Revision 1.57  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.56  2014-01-22 20:38:48+05:30  Cprogrammer
 * added hassrs.h
 *
 * Revision 1.55  2013-11-22 11:31:37+05:30  Cprogrammer
 * added concurrencyr, concurrencyl and domainqueue control files
 *
 * Revision 1.54  2013-08-25 18:38:09+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.53  2011-07-29 09:29:51+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.52  2011-07-08 13:47:54+05:30  Cprogrammer
 * added dnsbllist control file
 *
 * Revision 1.51  2010-04-24 20:13:53+05:30  Cprogrammer
 * added badip, qmtproutes control files
 *
 * Revision 1.50  2010-02-10 08:59:05+05:30  Cprogrammer
 * removed dependency on indimail
 *
 * Revision 1.49  2009-12-09 23:57:31+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.48  2009-12-05 11:25:39+05:30  Cprogrammer
 * added control files badhelo, badhost, originipfield
 *
 * Revision 1.47  2009-09-01 23:19:56+05:30  Cprogrammer
 * changes for batv
 *
 * Revision 1.46  2009-09-01 22:03:31+05:30  Cprogrammer
 * added control files for BATV
 *
 * Revision 1.45  2009-04-29 11:54:08+05:30  Cprogrammer
 * added blackholedrcpt, blackholedrcptpatterns, goodrcpt, goodrcptpatterns
 *
 * Revision 1.44  2009-03-29 01:08:55+05:30  Cprogrammer
 * added signaturedomains
 *
 * Revision 1.43  2009-03-08 10:19:31+05:30  Cprogrammer
 * removed filterargs, spamfilterargs
 *
 * Revision 1.42  2009-01-14 15:24:10+05:30  Cprogrammer
 * display files/directories in error messages
 *
 * Revision 1.41  2008-06-25 20:39:01+05:30  Cprogrammer
 * added queue_base control file
 *
 * Revision 1.40  2008-06-01 15:41:29+05:30  Cprogrammer
 * removed redundant control files
 *
 * Revision 1.39  2008-05-28 22:25:16+05:30  Cprogrammer
 * removed redundant control files. added mcdinfo
 *
 * Revision 1.38  2008-02-05 15:32:24+05:30  Cprogrammer
 * added control file domainbindings
 *
 * Revision 1.37  2007-12-20 13:50:40+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.36  2005-12-29 14:01:18+05:30  Cprogrammer
 * added separate queuelifetime for bounce messages
 *
 * Revision 1.35  2004-10-22 20:29:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.34  2004-10-11 14:27:15+05:30  Cprogrammer
 * added missing argment to control_readfile
 *
 * Revision 1.33  2004-09-23 22:55:49+05:30  Cprogrammer
 * added badext, badextpatterns
 *
 * Revision 1.32  2004-08-14 02:22:05+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.31  2004-07-13 22:52:05+05:30  Cprogrammer
 * added control files signatures, bodycheck, chkusrdomains, chkrcptdomains
 *
 * Revision 1.30  2004-07-13 22:44:58+05:30  Cprogrammer
 * added control file maxcmdlen
 *
 * Revision 1.29  2003-12-26 14:06:07+05:30  Cprogrammer
 * added authdomains control file
 * added switch -v for listing out uknown files in control directory
 *
 * Revision 1.28  2003-12-22 18:37:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include "substdio.h"
#include "subfd.h"
#include "fmt.h"
#include "str.h"
#include "control.h"
#include "constmap.h"
#include "stralloc.h"
#include "direntry.h"
#include "pathexec.h"
#include "envdir.h"
#include "auto_uids.h"
#include "auto_qmail.h"
#include "auto_break.h"
#include "auto_patrn.h"
#include "auto_spawn.h"
#include "auto_split.h"
#include "auto_control.h"
#include "env.h"
#include "getEnvConfig.h"
#include "variables.h"
#include "hassrs.h"
#include "indimail_stub.h"
#ifdef USE_SPF
#include "spf.h"
#endif
#include "set_environment.h"

#define FATAL "qmail-showctl: fatal: "
#define WARN  "qmail-showctl: warn: "

stralloc        me = { 0 };
int             meok, conf_split;
stralloc        line = { 0 };
stralloc        libfn = { 0 };
char            num[FMT_ULONG];

void
safeput(buf, len)
	char           *buf;
	unsigned int    len;
{
	char            ch;

	while (len > 0) {
		ch = *buf;
		if ((ch < 32) || (ch > 126))
			ch = '?';
		substdio_put(subfdout, &ch, 1);
		++buf;
		--len;
	}
}

void
do_int(fn, def, pre, post)
	char           *fn;
	char           *def;
	char           *pre;
	char           *post;
{
	int             i;
	substdio_puts(subfdout, "\n");
	substdio_puts(subfdout, fn);
	substdio_puts(subfdout, ": ");
	switch (control_readint(&i, fn)) {
	case 0:
		substdio_puts(subfdout, "(Default) ");
		substdio_puts(subfdout, pre);
		substdio_puts(subfdout, def);
		substdio_puts(subfdout, post);
		substdio_puts(subfdout, ".\n");
		break;
	case 1:
		if (i < 0)
			i = 0;
		substdio_puts(subfdout, pre);
		substdio_put(subfdout, num, fmt_uint(num, i));
		substdio_puts(subfdout, post);
		substdio_puts(subfdout, ".\n");
		break;
	default:
		substdio_puts(subfdout, "Oops! Trouble reading this file [");
		substdio_puts(subfdout, fn);
		substdio_puts(subfdout, "].\n");
		break;
	}
}

void
do_str(fn, flagme, def, pre)
	char           *fn;
	int             flagme;
	char           *def;
	char           *pre;
{
	substdio_puts(subfdout, "\n");
	substdio_puts(subfdout, fn);
	substdio_puts(subfdout, ": ");
	switch (control_readline(&line, fn)) {
	case 0:
		substdio_puts(subfdout, "(Default) ");
		if (!stralloc_copys(&line, def)) {
			substdio_puts(subfdout, "Oops! Out of memory.\n");
			break;
		}
		if (flagme && meok)
			if (!stralloc_copy(&line, &me)) {
				substdio_puts(subfdout, "Oops! Out of memory.\n");
				break;
			}
	case 1:
		substdio_puts(subfdout, pre);
		safeput(line.s, line.len);
		substdio_puts(subfdout, ".\n");
		break;
	default:
		substdio_puts(subfdout, "Oops! Trouble reading this file [");
		substdio_puts(subfdout, fn);
		substdio_puts(subfdout, "] .\n");
		break;
	}
}

int
do_lst(fn, def, pre, post)
	char           *fn;
	char           *def;
	char           *pre;
	char           *post;
{
	int             i;
	int             j;

	substdio_puts(subfdout, "\n");
	substdio_puts(subfdout, fn);
	substdio_puts(subfdout, ": ");
	switch (control_readfile(&line, fn, 0)) {
	case 0:
		substdio_puts(subfdout, "(Default) ");
		substdio_puts(subfdout, def);
		substdio_puts(subfdout, "\n");
		return 0;
	case 1:
		substdio_puts(subfdout, "\n");
		i = 0;
		for (j = 0; j < line.len; ++j)
			if (!line.s[j]) {
				substdio_puts(subfdout, pre);
				safeput(line.s + i, j - i);
				substdio_puts(subfdout, post);
				substdio_puts(subfdout, "\n");
				i = j + 1;
			}
		return 1;
	default:
		substdio_puts(subfdout, "Oops! Trouble reading this file [");
		substdio_puts(subfdout, fn);
		substdio_puts(subfdout, "].\n");
		return -1;
	}
}

void
print_concurrency()
{
	static char     d = 0;

	if (!d++) {
		do_int("concurrencylocal", "10", "Local  concurrency is ", "");
		do_int("concurrencyremote", "20", "Remote concurrency is ", "");
		do_int("concurrencyincoming", "10", "SMTP   concurrency is ", "");
	}
}

void
die_chdir(char *dir)
{
	substdio_puts(subfdout, "Oops! Unable to chdir to ");
	substdio_puts(subfdout, dir);
	substdio_puts(subfdout, ".\n");
	substdio_flush(subfdout);
	_exit(111);
}

int
main(int argc, char **argv)
{
	DIR            *dir;
	direntry       *d;
	void           *handle = (void *) 0;
	char           *ptr, *local_ip, *qbase, *local_id, *errstr;
	int             i, verbose;
	struct stat     stmrh, stmrhcdb;
	char           *(*get_local_ip) (void);
	char           *(*get_local_hostid) (void);

	if (argc == 2 && str_equal(argv[1], "-v"))
		verbose = 1;
	else
		verbose = 0;
	set_environment(WARN, FATAL);
	substdio_puts(subfdout, "qmail home directory: ");
	substdio_puts(subfdout, auto_qmail);
	substdio_puts(subfdout, ".\n");

	substdio_puts(subfdout, "user-ext delimiter: ");
	substdio_puts(subfdout, auto_break);
	substdio_puts(subfdout, ".\n");

	substdio_puts(subfdout, "paternalism (in decimal): ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_patrn));
	substdio_puts(subfdout, ".\n");

	substdio_puts(subfdout, "silent concurrency limit: ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_spawn));
	substdio_puts(subfdout, ".\n");

	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	substdio_puts(subfdout, "subdirectory split: ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) conf_split));
	substdio_puts(subfdout, ".\n");

	substdio_put(subfdout, "user ids\n", 9);
	if (uidinit(1)) {
		substdio_puts(subfdout, "Oops! Unable to get uids/gids.\n");
		substdio_flush(subfdout);
		_exit(111);
	}
	substdio_put(subfdout, " alias: ", 8);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uida));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmaild: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidd));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmaill: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidl));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailp: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidp));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailq: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidq));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailr: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidr));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmails: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uids));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " indimail: ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidv));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qscand: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidc));
	substdio_put(subfdout, "\n\n", 2);
	substdio_put(subfdout, "group ids\n", 10);
	substdio_put(subfdout, " nofiles: ", 10);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidn));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmail: ", 8);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidq));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " indimail: ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidv));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qscand: ", 9);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidc));
	substdio_put(subfdout, "\n", 1);

	if (chdir(auto_qmail) == -1)
		die_chdir(auto_qmail);
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (access(controldir, F_OK))
		die_chdir(controldir);

	if (!(dir = opendir(controldir))) {
		substdio_puts(subfdout, "Oops! Unable to open current directory [");
		substdio_puts(subfdout, controldir);
		substdio_puts(subfdout, "].\n");
		substdio_flush(subfdout);
		_exit(111);
	}

	if ((meok = control_readline(&me, "me") == -1)) {
		substdio_puts(subfdout, "Oops! Trouble reading ");
		substdio_puts(subfdout, controldir);
		substdio_puts(subfdout, "/me.");
		substdio_flush(subfdout);
		_exit(111);
	}
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&libfn, controldir)) {
			substdio_puts(subfdout, "Oops! Out of memory.\n");
			_exit(111);
		}
		if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/")) {
			substdio_puts(subfdout, "Oops! Out of memory.\n");
			_exit(111);
		}
		if (!stralloc_catb(&libfn, "libindimail", 11) ||
				!stralloc_0(&libfn)) {
			substdio_puts(subfdout, "Oops! Out of memory.\n");
			_exit(111);
		}
		ptr = libfn.s;
	} else
		ptr = "VIRTUAL_PKG_LIB";
	loadLibrary(&handle, ptr, &i, &errstr);
	if (i) {
		substdio_puts(subfderr, "error loading shared library: ");
		substdio_puts(subfderr, errstr);
		substdio_puts(subfderr, "\n");
		substdio_flush(subfderr);
		_exit(111);
	}
	if (handle) {
		if (!(get_local_ip = getlibObject(ptr, &handle, "get_local_ip", &errstr))) {
			substdio_puts(subfderr, "getlibObject: get_local_ip: ");
			substdio_puts(subfderr, errstr);
			substdio_puts(subfderr, "\n");
			substdio_flush(subfderr);
			_exit(111);
		} else
			local_ip = (*get_local_ip) ();
		if (!(get_local_hostid = getlibObject(ptr, &handle, "get_local_hostid", &errstr))) {
			substdio_puts(subfderr, "getlibObject: get_local_hostid: ");
			substdio_puts(subfderr, errstr);
			substdio_puts(subfderr, "\n");
			substdio_flush(subfderr);
			_exit(111);
		} else
			local_id = (*get_local_hostid) ();
		closeLibrary(&handle);
	} else {
		local_id = "0x00";
		local_ip = "127.0.0.1";
	}
	do_str("me", 0, "undefined! Uh-oh", "My name is ");
	if (do_lst
		("rcpthosts", "SMTP clients may send messages to any recipient.", "SMTP clients may send messages to recipients at ", "."))
		do_lst("morercpthosts", "No effect", "SMTP clients may send messages to recipients at ", ".");
	else
		do_lst("morercpthosts", "No rcpthosts; morercpthosts is irrelevant.",
			   "No rcpthosts; doesn't matter that morercpthosts has ", ".");
	/*- XXX: check morercpthosts.cdb contents */
	substdio_puts(subfdout, "\nmorercpthosts.cdb: ");
	if (stat("morercpthosts", &stmrh) == -1)
		if (stat("morercpthosts.cdb", &stmrhcdb) == -1)
			substdio_puts(subfdout, "(Defaul.) No effect\n");
		else
			substdio_puts(subfdout, "Oops! morercpthosts.cdb exists but morercpthosts doesn't.\n");
	else if (stat("morercpthosts.cdb", &stmrhcdb) == -1)
		substdio_puts(subfdout, "Oops! morercpthosts exists but morercpthosts.cdb doesn't.\n");
	else if (stmrh.st_mtime > stmrhcdb.st_mtime)
		substdio_puts(subfdout, "Oops! morercpthosts.cdb is older than morercpthosts.\n");
	else
		substdio_puts(subfdout, "Modified recently enough; hopefully up to date.\n");
	do_lst("locals", "Messages for me are delivered locally.", "Messages for ", " are delivered locally.");
	do_str("defaultdomain", 1, "defaultdomain", "Default domain name is ");
	do_str("defaulthost", 1, "defaulthost", "Default host name is ");
	do_str("defaultdelivery", 0, "undefined! Uh-oh", "Default Delivery is: ");
	do_str("smtpgreeting", 1, "smtpgreeting", "SMTP greeting: 220 ");
	do_lst("smtproutes", "No artificial SMTP routes.", "SMTP route: ", "");
	do_lst("signaturedomains", "No DKIM signature domains.", "DKIM signature domain(s): ", "");
	do_lst("virtualdomains", "No virtual domains.", "Virtual domain: ", "");

	do_lst("blackholedsender", "Any SMTP connection is allowed.", "", " is immediately dropped for MAIL FROM.");
	do_lst("blackholedpatterns", "Any SMTP connection is allowed.", "",
		   " is immediately dropped for MAIL FROM. (Not if line starts with !).");
	do_lst("blackholedrcpt", "Any SMTP connection is allowed.", "", " is immediately dropped for RCPT TO.");
	do_lst("blackholedrcptpatterns", "Any SMTP connection is allowed.", "",
		   " is immediately dropped for RCPT TO. (Not if line starts with !).");
	do_lst("badhelo", "Any HELO/EHLO greeting is allowed.", "", " not accepted in HELO/EHLO.");
	do_lst("badhost", "Any external remote hosts are allowed.", "", " pattern will be rejected for external host.");
	do_lst("badip", "Any external IP are allowed.", "", " pattern will be rejected for external IP.");
	do_lst("badmailfrom", "Any MAIL FROM is allowed.", "", " not accepted in MAIL FROM.");
	do_lst("badmailpatterns", "Any MAIL FROM is allowed.", "", " not accepted in MAIL FROM (Not if line starts with !).");
	do_lst("badrcptto", "Any RCPT TO is allowed.", "", " not accepted in RCPT TO.");
	do_lst("badrcptpatterns", "Any RCPT TO is allowed.", "", " not accepted in RCPT TO (Not if line starts with !).");
	do_lst("badext", "All attachments are allowed.", "", " not accepted in attachments.");
	do_lst("badextpatterns", "All attachments are allowed.", "", " not accepted in attachments. (Not if line starts with !).");
	do_lst("spamignore", "Any MAIL FROM exceeding spam count will be blocked by chowkidar", "",
		   " will not be treated as spammers.");
	do_lst("spamignorepatterns", "Any MAIL FROM exceeding spam count will be blocked by chowkidar", "",
		   " will not be treated as spammers.");
	do_lst("spamredirect", "No spam redirection", "", " spam redirection.");
	do_lst("bouncemessage", "No bouncemessage defined", "", "");
	do_lst("doublebouncemessage", "No doublebouncemessage defined", "", "");
	do_lst("bouncemaxbytes", "No bouncemaxbytes defined.", "Actual Bouncemaxbytes: ", "");
	do_lst("dnsbllist", "No dnsbl list configured.", "List at ", " configured for dnsbl check.");
	do_lst("bindroutes", "No binding routes.", "Binding route: ", "");
	do_lst("domainbindings", "No sender domain based local ip bindings.", "Sender domain local IP binding: ", "");
	do_lst("percenthack", "The percent hack is not allowed.", "The percent hack is allowed for user%host@", ".");
	do_lst("qmqpservers", "No QMQP servers.", "QMQP server: ", ".");

	do_str("globalspamredirect", 0, "none", "spam redirected to ");
	do_str("bouncefrom", 0, "MAILER-DAEMON", "Bounce user name is ");
	do_str("bouncehost", 1, "bouncehost", "Bounce host name is ");
	do_str("bouncesubject", 0, "\"failure notice\"", "Bounce Subject is ");
	do_str("doublebouncesubject", 0, "\"failure notice\"", "Double Bounce Subject is ");
	do_str("doublebouncehost", 1, "doublebouncehost", "2B recipient host: ");
	do_str("doublebounceto", 0, "postmaster", "2B recipient user: ");
	do_str("envnoathost", 1, "envnoathost", "Presumed domain name is ");
	do_str("helohost", 1, "helohost", "SMTP client HELO host name is ");
	do_str("idhost", 1, "idhost", "Message-ID host name is ");
	do_str("localiphost", 0, local_ip ? local_ip : "0.0.0.0", "Local IP address becomes ");
	do_str("outgoingip", 0, local_ip ? local_ip : "0.0.0.0", "Outgoingip host name is ");
	do_str("plusdomain", 1, "plusdomain", "Plus domain name is ");

	do_int("maxhops", "100", "MAX Hops is ", " hops");
	do_str("queue_base", 0, (qbase = env_get("QUEUE_BASE")) ? qbase : auto_qmail, "Base queue directory: ");
	do_lst("extraqueue", "No extra recipient.", "Extra recipient for queued message: '", "'.");
	do_lst("domainqueue", "No Domain Queues defined.", "Domain Queues: ", "");
	do_int("queuelifetime", "604800", "Message lifetime in the queue is ", " seconds");
	do_int("maxdeliveredto", "0", "Maximum Delivered-To lines: ", "");
	do_int("bouncelifetime", "604800", "Bounce message lifetime in the queue is ", " seconds (or max of queuelifetime)");
	do_int("holdremote", "0", "Hold Remote is ", "");
	do_int("holdlocal", "0", "Hold Local is ", "");
	do_int("originipfield", "0", "X-Originating-IP header is set to ", "");

#ifdef HAVESRS
	do_str("srs_domain", 0, "", "SRS domain name is ");
	do_lst("srs_secrets", "No secrets", "", "");
	do_int("srs_maxage", "21", "SRS maxage is ", "");
	do_int("srs_hashlength", "4", "SRS hashlength is ", "");
	do_int("srs_hashmin", "4", "SRS hashmin is ", "");
#endif
	do_lst("qmtproutes", "No artificial QMTP routes.", "QMTP route: ", "");
	do_int("databytes", "0", "SMTP DATA limit is ", " bytes");
	do_int("maxcmdlen", "0", "Max SMTP Command Length is ", "");
	do_int("timeoutconnect", "60", "SMTP client connection timeout is ", " seconds");
	do_int("timeoutremote", "1200", "SMTP client data timeout is ", " seconds");
	do_int("timeoutsmtpd", "1200", "SMTP server data timeout is ", " seconds");
	do_int("timeoutread", "4", "InServer Read data timeout is ", " seconds");
	do_int("timeoutwrite", "4", "InServer Write data timeout is ", " seconds");
	do_lst("etrnhosts", "No ETRN/ATRN domains.", "ETRN/ATRN domain(s): ", "");
	do_lst("chkrcptdomains", "All Recipient Domains will be checked for User Status", "Recipient Domains checked for User Status: ",
		   "");
	do_lst("authdomains", "Auth not reqd for mails to local domains ", "SMTP/IMAP/POP3 auth required for: ", "");
	do_lst("relayclients", "No Relayclients defined.", "Allowed Relayclients: ", "");
	do_lst("relaydomains", "No Relaydomains defined.", "Allowed Relaydomains: ", "");
	do_lst("relaymailfrom", "No Relaymailfrom defined.", "Allowed <Mail from: Relayaddresses>: ", "");
	do_lst("tarpitcount", "No Tarpitcount defined.", "Actual Tarpitcount: ", "");
	do_lst("maxrecipients", "No limit on number of Recipients defined.", "Actual Maxrecipients: ", "");
	do_lst("tarpitdelay", "No Tarpitdelay defined.", "Actual Tarpitdelay: ", "");
	if (handle) {
		do_str("host.master", 0, MASTER_HOST, "Master Host: ");
		do_str("host.cntrl", 0, CNTRL_HOST, "Control Host: ");
		do_str("host.mysql", 0, MYSQL_HOST, "Mysql Host: ");
		do_str("hostip", 0, local_ip ? local_ip : "127.0.0.1", "Host Local Ip: ");
		do_str("hostid", 0, local_id ? local_id : "?", "Host Local Id: ");
	}
	do_lst("signatures", "No virus signatures defined.", "virus signatures: ", "");
	do_lst("bodycheck", "No Content-filters.", "Content-filters: ", "");
	do_lst("from.envrules", "No Sender Envrules defined.", "Sender Envrules: ", "");
	do_lst("rcpt.envrules", "No Recipient Envrules defined.", "Recipient Envrules: ", "");
#ifdef USE_SPF
	do_int("spfbehavior", "0", "The SPF behavior is ", "");
	do_str("spfexp", 0, SPF_DEFEXP, "The SPF default explanation is: 550 ");
	do_str("spfguess", 0, "", "The guess SPF rules are: ");
	do_str("spfrules", 0, "", "The local SPF rules are: ");
#endif
#ifdef BATV
	do_str("signkey", 0, "No BATV sign key", "BATV sign key: ");
	do_int("signkeystale", "7", "BATV sign key stale is set to ", "");
	do_lst("nosignhosts", "Nothing defined.", "", " will be excluded for BATV signing (remote hosts).");
	do_lst("nosignmydoms", "Nothing defined.", "", " will be excluded for BATV signing (local domain).");
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	do_str("tlsclientmethod", 0, "TLSv1", "TLS client method is ");
	do_str("tlsservermethod", 0, "TLSv1", "TLS server method is ");
#endif
	do_lst("nodnscheck", "Any MAIL FROM is checked for existing Domains.", "", " excluded for DNS checks in MAIL FROM.");

	while ((d = readdir(dir))) {
		if ((ptr = (char *) strrchr(d->d_name, '.'))) {
			ptr++;
			if (str_equal(ptr, "lock"))
				continue;
		}
		if (str_equal(d->d_name, "."))
			continue;
		if (str_equal(d->d_name, ".."))
			continue;
		if (str_equal(d->d_name, "bouncefrom"))
			continue;
		if (str_equal(d->d_name, "bouncehost"))
			continue;
		if (str_equal(d->d_name, "bouncesubject"))
			continue;
		if (str_equal(d->d_name, "bouncemaxbytes"))
			continue;
		if (str_equal(d->d_name, "bouncemessage"))
			continue;
		if (str_equal(d->d_name, "doublebouncesubject"))
			continue;
		if (str_equal(d->d_name, "doublebouncemessage"))
			continue;
		if (str_equal(d->d_name, "badrcptto"))
			continue;
		if (str_equal(d->d_name, "badhelo"))
			continue;
		if (str_equal(d->d_name, "badhost"))
			continue;
		if (str_equal(d->d_name, "badip"))
			continue;
		if (str_equal(d->d_name, "badmailpatterns"))
			continue;
		if (str_equal(d->d_name, "badrcptpatterns"))
			continue;
		if (str_equal(d->d_name, "badmailfrom"))
			continue;
		if (str_equal(d->d_name, "spamignore"))
			continue;
		if (str_equal(d->d_name, "spamignorepatterns"))
			continue;
		if (str_equal(d->d_name, "filterargs"))
			continue;
		if (str_equal(d->d_name, "spamfilter"))
			continue;
		if (str_equal(d->d_name, "bouncefrom"))
			continue;
		if (str_equal(d->d_name, "bouncehost"))
			continue;
		if (str_equal(d->d_name, "concurrencylocal"))
			continue;
		if (str_equal(d->d_name, "concurrencyremote"))
			continue;
		if (str_equal(d->d_name, "maxdeliveredto"))
			continue;
		if (str_equal(d->d_name, "tlsclientmethod"))
			continue;
		if (str_equal(d->d_name, "tlsservermethod"))
			continue;
		if (str_start(d->d_name, "concurrencyr")) {
			print_concurrency();
			do_int(d->d_name, "20", "Remote Queue concurrency is ", "");
			continue;
		}
		if (str_start(d->d_name, "concurrencyl")) {
			print_concurrency();
			do_int(d->d_name, "10", "Local Queue concurrency is ", "");
			continue;
		}
		if (str_equal(d->d_name, "concurrencyincoming")) {
			print_concurrency();
			continue;
		}
		if (str_equal(d->d_name, "domainqueue"))
			continue;
		if (str_equal(d->d_name, "from.envrules"))
			continue;
		if (str_equal(d->d_name, "rcpt.envrules"))
			continue;
		if (str_equal(d->d_name, "databytes"))
			continue;
		if (str_equal(d->d_name, "maxhops"))
			continue;
		if (str_equal(d->d_name, "defaultdomain"))
			continue;
		if (str_equal(d->d_name, "defaulthost"))
			continue;
		if (str_equal(d->d_name, "dnsbllist"))
			continue;
		if (str_equal(d->d_name, "doublebouncehost"))
			continue;
		if (str_equal(d->d_name, "doublebounceto"))
			continue;
		if (str_equal(d->d_name, "envnoathost"))
			continue;
		if (str_equal(d->d_name, "extraqueue"))
			continue;
		if (str_equal(d->d_name, "helohost"))
			continue;
		if (str_equal(d->d_name, "idhost"))
			continue;
		if (str_equal(d->d_name, "localiphost"))
			continue;
		if (str_equal(d->d_name, "locals"))
			continue;
		if (str_equal(d->d_name, "me"))
			continue;
		if (str_equal(d->d_name, "morercpthosts"))
			continue;
		if (str_equal(d->d_name, "signatures"))
			continue;
		if (str_equal(d->d_name, "bodycheck"))
			continue;
		if (str_equal(d->d_name, "morercpthosts.cdb"))
			continue;
		if (str_equal(d->d_name, "nodnscheck"))
			continue;
		if (str_equal(d->d_name, "percenthack"))
			continue;
		if (str_equal(d->d_name, "plusdomain"))
			continue;
		if (str_equal(d->d_name, "qmqpservers"))
			continue;
		if (str_equal(d->d_name, "originipfield"))
			continue;
		if (str_equal(d->d_name, "queuelifetime"))
			continue;
		if (str_equal(d->d_name, "bouncelifetime"))
			continue;
		if (str_equal(d->d_name, "rcpthosts"))
			continue;
		if (str_equal(d->d_name, "smtpgreeting"))
			continue;
		if (str_equal(d->d_name, "smtproutes"))
			continue;
#ifdef HAVESRS
		if (str_equal(d->d_name, "srs_domain"))
			continue;
		if (str_equal(d->d_name, "srs_secrets"))
			continue;
		if (str_equal(d->d_name, "srs_maxage"))
			continue;
		if (str_equal(d->d_name, "srs_hashlength"))
			continue;
		if (str_equal(d->d_name, "srs_hashmin"))
			continue;
#endif
		if (str_equal(d->d_name, "qmtproutes"))
			continue;
		if (str_equal(d->d_name, "queue_base"))
			continue;
#ifdef USE_SPF
		if (str_equal(d->d_name, "spfbehavior"))
			continue;
		if (str_equal(d->d_name, "spfexp"))
			continue;
		if (str_equal(d->d_name, "spfguess"))
			continue;
		if (str_equal(d->d_name, "spfrules"))
			continue;
#endif
		if (str_equal(d->d_name, "timeoutconnect"))
			continue;
		if (str_equal(d->d_name, "timeoutremote"))
			continue;
		if (str_equal(d->d_name, "timeoutsmtpd"))
			continue;
		if (str_equal(d->d_name, "timeoutread"))
			continue;
		if (str_equal(d->d_name, "timeoutwrite"))
			continue;
		if (str_equal(d->d_name, "virtualdomains"))
			continue;
		if (str_equal(d->d_name, "etrnhosts"))
			continue;
		if (str_equal(d->d_name, "chkrcptdomains"))
			continue;
		if (str_equal(d->d_name, "authdomains"))
			continue;
		if (str_equal(d->d_name, "relayclients"))
			continue;
		if (str_equal(d->d_name, "relaydomains"))
			continue;
		if (str_equal(d->d_name, "relaymailfrom"))
			continue;
		if (str_equal(d->d_name, "tarpitcount"))
			continue;
		if (str_equal(d->d_name, "tarpitdelay"))
			continue;
		if (str_equal(d->d_name, "maxrecipients"))
			continue;
		if (str_equal(d->d_name, "maxcmdlen"))
			continue;
		if (str_equal(d->d_name, "defaultdelivery"))
			continue;
		if (str_equal(d->d_name, "blackholedsender"))
			continue;
		if (str_equal(d->d_name, "blackholedpatterns"))
			continue;
		if (str_equal(d->d_name, "blackholedrcpt"))
			continue;
		if (str_equal(d->d_name, "blackholedrcptpatterns"))
			continue;
		if (str_equal(d->d_name, "goodrcptto"))
			continue;
		if (str_equal(d->d_name, "goodrcptpatterns"))
			continue;
		if (str_equal(d->d_name, "outgoingip"))
			continue;
		if (str_equal(d->d_name, "domainbindings"))
			continue;
		if (str_equal(d->d_name, "bindroutes"))
			continue;
		if (str_equal(d->d_name, "mcdinfo"))
			continue;
		if (str_equal(d->d_name, "host.mysql"))
			continue;
		if (str_equal(d->d_name, "host.master"))
			continue;
		if (str_equal(d->d_name, "host.cntrl"))
			continue;
		if (str_equal(d->d_name, "hostip"))
			continue;
		if (str_equal(d->d_name, "hostid"))
			continue;
		if (str_equal(d->d_name, "mailalert.cfg"))
			continue;
		if (str_equal(d->d_name, "badext"))
			continue;
		if (str_equal(d->d_name, "badextpatterns"))
			continue;
		if (str_equal(d->d_name, "holdremote"))
			continue;
		if (str_equal(d->d_name, "holdlocal"))
			continue;
		if (str_equal(d->d_name, "signaturedomains"))
			continue;
#ifdef BATV
		if (str_equal(d->d_name, "signkey"))
			continue;
		if (str_equal(d->d_name, "signkeystale"))
			continue;
		if (str_equal(d->d_name, "nosignhosts"))
			continue;
		if (str_equal(d->d_name, "nosignmydoms"))
			continue;
#endif
		if (verbose) {
			substdio_puts(subfdout, "\n");
			substdio_puts(subfdout, d->d_name);
			substdio_puts(subfdout, ": I have no idea what this file does.\n");
		}
	}
	print_concurrency();
	substdio_flush(subfdout);
	_exit(0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_showctl_c()
{
	static char    *x = "$Id: qmail-showctl.c,v 1.75 2021-06-24 12:17:02+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
