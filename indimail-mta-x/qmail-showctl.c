/*
 * $Log: qmail-showctl.c,v $
 * Revision 1.14  2023-05-31 16:31:02+05:30  Cprogrammer
 * restore HOME env variable after env_clear
 *
 * Revision 1.13  2023-03-05 01:35:12+05:30  Cprogrammer
 * added missing control files
 *
 * Revision 1.12  2023-02-14 09:15:23+05:30  Cprogrammer
 * renamed auto_uidv to auto_uidi, auto_uidc to auto_uidv, auto_gidv to auto_gidi
 * added auto_uidc for qcerts id
 *
 * Revision 1.11  2023-01-22 13:17:57+05:30  Cprogrammer
 * fixed do_int
 *
 * Revision 1.10  2023-01-18 00:02:13+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.9  2022-11-23 15:07:55+05:30  Cprogrammer
 * rename mysql_lib to libmysql on upgrade
 *
 * Revision 1.8  2022-11-03 12:35:40+05:30  Cprogrammer
 * added remote_auth, recipients control file
 *
 * Revision 1.7  2022-10-13 21:51:48+05:30  Cprogrammer
 * renamed batv control files (use batv prefix)
 *
 * Revision 1.6  2022-08-17 02:05:41+05:30  Cprogrammer
 * added qregex, tlsclients to control file list
 *
 * Revision 1.5  2022-03-20 16:28:45+05:30  Cprogrammer
 * -E option for displaying client env variables
 *
 * Revision 1.4  2022-01-30 09:14:04+05:30  Cprogrammer
 * made big-todo configurable
 * replaced execvp with execv
 *
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.2  2021-07-05 21:28:04+05:30  Cprogrammer
 * allow processing $HOME/.defaultqueue for root
 *
 * Revision 1.1  2021-07-04 14:37:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#include <str.h>
#include <constmap.h>
#include <stralloc.h>
#include <direntry.h>
#include <pathexec.h>
#include <envdir.h>
#include <env.h>
#include <getEnvConfig.h>
#include <sgetopt.h>
#include <getln.h>
#include <open.h>
#include <error.h>
#include <strerr.h>
#include <qprintf.h>
#include <noreturn.h>
#include <wait.h>
#include "control.h"
#include "auto_uids.h"
#include "auto_qmail.h"
#include "auto_assign.h"
#include "auto_break.h"
#include "auto_patrn.h"
#include "auto_spawn.h"
#include "auto_split.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"
#include "auto_prefix.h"
#include "variables.h"
#include "hassrs.h"
#include "indimail_stub.h"
#ifdef USE_SPF
#include "spf.h"
#endif
#include "set_environment.h"

#define FATAL "qmail-showctl: fatal: "
#define WARN  "qmail-showctl: warn: "

static stralloc me = { 0 };
static stralloc line = { 0 };
static stralloc libfn = { 0 };
static int      meok, conf_split;
static char     num[FMT_ULONG];

no_return void
die_nomem()
{
	substdio_puts(subfderr, "Out of memory\n");
	_exit(111);
}

void
safeput(char *buf, unsigned int len)
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
do_int(char *fn, char *def, char *pre, char *post)
{
	int             i;

	subprintf(subfdout, "%-22s: ", fn);
	switch (control_readint(&i, fn))
	{
	case -1:
		strerr_die3sys(111, "unable to read ", fn, ": ");
	case 0:
		subprintf(subfdout, "%s%s%s (Default)\n\n", pre, def, post);
		break;
	case 1:
		if (i < 0)
			i = 0;
		subprintf(subfdout, "%s%d%s\n\n", pre, i, post);
		break;
	}
}

void
do_str(char *fn, int flagme, char *def, char *pre)
{
	int             i = 0;

	subprintf(subfdout, "%-22s: ", fn);
	switch (control_readline(&line, fn))
	{
	case -1:
		strerr_die3sys(111, "unable to read ", fn, ": ");
	case 0:
		i = 1;
		if (!stralloc_copys(&line, def))
			die_nomem();
		if (flagme && meok)
			if (!stralloc_copy(&line, &me))
				die_nomem();
	case 1:
		substdio_puts(subfdout, pre);
		safeput(line.s, line.len);
		if (i)
			substdio_puts(subfdout, " (Default) ");
		substdio_puts(subfdout, "\n\n");
		break;
	}
}

int
do_lst(char *fn, char *def, char *pre, char *post)
{
	int             i, j;

	subprintf(subfdout, "%-22s: ", fn);
	switch (control_readfile(&line, fn, 0))
	{
	case -1:
		strerr_die3sys(111, "unable to read ", fn, ": ");
	case 0:
		subprintf(subfdout, "%s (Default)\n\n", def);
		return 0;
	case 1:
		i = 0;
		for (j = 0; j < line.len; ++j)
			if (!line.s[j]) {
				if (i)
					subprintf(subfdout, "%24s", " ");
				substdio_puts(subfdout, pre);
				safeput(line.s + i, j - i);
				subprintf(subfdout, "%s\n", post);
				i = j + 1;
			}
		substdio_puts(subfdout, "\n");
		return 1;
	}
	return 0;
}

void
print_concurrency()
{
	static char     d = 0;

	if (!d++) {
		do_int("concurrencylocal",  "10", "Global Local  concurrency is ", "");
		do_int("concurrencyremote", "20", "Global Remote concurrency is ", "");
	}
}

no_return void
die_chdir(char *dir)
{
	substdio_puts(subfdout, "Oops! Unable to chdir to ");
	substdio_puts(subfdout, dir);
	substdio_puts(subfdout, ".\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
display_control()
{
	char           *ptr, *local_ip, *qbase, *local_id, *errstr;
	void           *handle = (void *) 0;
	struct stat     stmrh, stmrhcdb;
	int             i, load_indimail = 0;
	char           *(*get_local_ip) (void);
	char           *(*get_local_hostid) (void);

	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if ((meok = control_readline(&me, "me") == -1)) {
		substdio_puts(subfdout, "Oops! Trouble reading ");
		substdio_puts(subfdout, controldir);
		substdio_puts(subfdout, "/me.");
		substdio_flush(subfdout);
		_exit(111);
	}
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!stralloc_copys(&libfn, controldir))
			die_nomem();
		if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/"))
			die_nomem();
		if (!stralloc_catb(&libfn, "libindimail", 11) ||
				!stralloc_0(&libfn))
			die_nomem();
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
		load_indimail = 1;
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
	subprintf(subfdout, "%-22s: ", "morercpthosts.cdb");
	if (stat("morercpthosts", &stmrh) == -1)
		if (stat("morercpthosts.cdb", &stmrhcdb) == -1)
			substdio_puts(subfdout, "(Default.) No effect\n");
		else
			substdio_puts(subfdout, "Oops! morercpthosts.cdb exists but morercpthosts doesn't.\n");
	else
	if (stat("morercpthosts.cdb", &stmrhcdb) == -1)
		substdio_puts(subfdout, "Oops! morercpthosts exists but morercpthosts.cdb doesn't.\n");
	else
	if (stmrh.st_mtime > stmrhcdb.st_mtime)
		substdio_puts(subfdout, "Oops! morercpthosts.cdb is older than morercpthosts.\n");
	else
		substdio_puts(subfdout, "Modified recently enough; hopefully up to date.\n");
	substdio_puts(subfdout, "\n");
	do_lst("locals", "Messages for me are delivered locally.", "Messages for ", " are delivered locally.");
	do_str("defaultdomain", 1, "defaultdomain", "Default domain name is ");
	do_str("defaulthost", 1, "defaulthost", "Default host name is ");
	do_str("defaultdelivery", 0, "undefined! Uh-oh", "Default Delivery is: ");
	do_str("smtpgreeting", 1, "smtpgreeting", "SMTP greeting: 220 ");
	do_lst("smtproutes", "No artificial SMTP routes.", "SMTP route: ", "");
	do_lst("remote_auth", "No users for remote authenticated SMTP", "Remote SMTP Auth Users: ", "");
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
	do_lst("chkrcptdomains", "All Recipient Domains will be checked for User Status",
			"Recipient Domains checked for User Status: ", "");
	do_lst("recipients", "No recipients extension defined", "recipients extension rules: ", "");
	do_lst("authdomains", "Auth not reqd for mails to local domains ", "SMTP/IMAP/POP3 auth required for: ", "");
	do_lst("relayclients", "No Relayclients defined.", "Allowed Relayclients: ", "");
	do_lst("relaydomains", "No Relaydomains defined.", "Allowed Relaydomains: ", "");
	do_lst("relaymailfrom", "No Relaymailfrom defined.", "Allowed <Mail from: Relayaddresses>: ", "");
	do_lst("tarpitcount", "No Tarpitcount defined.", "Actual Tarpitcount: ", "");
	do_lst("maxrecipients", "No limit on number of Recipients defined.", "Actual Maxrecipients: ", "");
	do_lst("tarpitdelay", "No Tarpitdelay defined.", "Actual Tarpitdelay: ", "");
	if (load_indimail) {
		do_str("host.master", 0, MASTER_HOST, "Master Host: ");
		do_str("host.cntrl", 0, CNTRL_HOST, "Control Host: ");
		do_str("host.mysql", 0, MYSQL_HOST, "Mysql Host: ");
		do_str("hostip", 0, local_ip ? local_ip : "127.0.0.1", "Host Local IP: ");
		do_str("hostid", 0, local_id ? local_id : "?", "Host Local ID: ");
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
	do_str("batvkey", 0, "No BATV sign key", "BATV sign key: ");
	do_int("batvkeystale", "7", "BATV sign key stale is set to ", "");
	do_lst("batvnosignremote", "Nothing defined.", "", " will be excluded for BATV signing (remote hosts).");
	do_lst("batvnosignlocals", "Nothing defined.", "", " will be excluded for BATV signing (local domain).");
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	do_str("tlsclientmethod", 0, "TLSv1", "TLS client method is ");
	do_str("tlsservermethod", 0, "TLSv1", "TLS server method is ");
#endif
	do_lst("nodnscheck", "Any MAIL FROM is checked for existing Domains.", "", " excluded for DNS checks in MAIL FROM.");
}

int
valid_control_files(char *fn)
{
	int             len, fd, match;
	struct substdio ssin;
	char            inbuf[2048];
	char          **ptr;
	char           *control_fn_list[] = {"controlfiles.q", "controlfiles.i", 0};
	char           *control_files[] = {
		"bouncefrom", "bouncehost", "bouncesubject", "bouncemaxbytes", "bouncemessage",
		"doublebouncesubject", "doublebouncemessage", "badhost", "badmailpatterns",
		"badrcptpatterns", "spamignorepatterns", "filterargs", "spamfilter", "bouncefrom",
		"bouncehost", "concurrencylocal", "concurrencyremote", "maxdeliveredto",
		"domainqueue", "from.envrules",
		"rcpt.envrules", "databytes", "maxhops", "defaultdomain", "defaulthost",
		"dnsbllist", "doublebouncehost", "doublebounceto", "envnoathost", "extraqueue",
		"helohost", "idhost", "localiphost", "locals", "me", "morercpthosts", "signatures",
		"bodycheck", "morercpthosts.cdb", "nodnscheck", "percenthack", "plusdomain",
		"qmqpservers", "originipfield", "queuelifetime", "bouncelifetime", "rcpthosts",
		"smtpgreeting", "smtproutes", "qmtproutes", "queue_base", "timeoutconnect",
		"timeoutremote", "timeoutsmtpd", "timeoutread", "timeoutwrite", "virtualdomains",
		"etrnhosts", "relayclients", "relaydomains", "tarpitcount", "tarpitdelay",
		"maxrecipients", "maxcmdlen", "defaultdelivery", "blackholedpatterns",
		"blackholedrcptpatterns", "goodrcptpatterns", "outgoingip", "domainbindings",
		"bindroutes", "badextpatterns", "holdremote", "holdlocal",
		"msgqueuelen", "msgqueuesize", "global_vars", "qfilters", ".qmail_control",
		"qregex", "recipients", "remote_auth", "remote_auth.cdb",
		".indimail_control",
#ifdef HAVESRS
		"srs_domain", "srs_secrets", "srs_maxage", "srs_hashlength", "srs_hashmin",
#endif
#ifdef HASDKIM
		"dkimkey", "signaturedomains", "nosignaturedomains", "domainkeys",
#endif
#ifdef USE_SPF
		"spfbehavior", "spfexp", "spfguess", "spfrules",
#endif
#ifdef BATV
		"batvkey", "batvkeystale", "batvnosignremote", "batvnosignlocals",
#endif
#ifdef TLS
		"tlsclientmethod", "tlsservermethod", "tlsclients", "servercert.pem", "tlsserverciphers",
		"tlsclientciphers", "clientcert.pem",
#endif
		"conf-syncdir", "conf-fsync", "conf-fdatasync", "servicedir.conf",
		"level2-tlds", "level3-tlds", "libmysql", 0};
	char          *cdb_sql_files[] = {
		"authdomains",  "badhelo",  "badext",  "badmailfrom", "badrcptto", "blackholedsender",
		"blackholedrcpt", "chkrcptdomains", "goodrcptto", "relaymailfrom", "spamignore",
		"greylist.white", "tlsa.white", "tlsadomains", "badip", 0};

	for (ptr = control_files; *ptr; ptr++) {
		if (!str_diff(*ptr, fn))
			return 1;
	}
	for (ptr = cdb_sql_files; *ptr; ptr++) {
		if (!str_diff(*ptr, fn))
			return 1;
		if (!stralloc_copys(&line, *ptr))
			die_nomem();
		len = line.len;
		if (!stralloc_catb(&line, ".sql", 4) ||
				!stralloc_0(&line))
			die_nomem();
		if (!str_diff(line.s, fn))
			return 1;
		line.len = len;
		if (!stralloc_catb(&line, ".cdb", 4) ||
				!stralloc_0(&line))
			die_nomem();
		if (!str_diff(line.s, fn))
			return 1;
	}
	if (chdir(auto_sysconfdir) == -1)
		die_chdir(auto_sysconfdir);
	for (ptr = control_fn_list; *ptr; ptr++) {
		if ((fd = open_read(*ptr)) == -1) {
			if (errno == error_noent)
				continue;
			strerr_die4sys(111, FATAL, "unable to open ", *ptr, ": ");
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				strerr_die4sys(111, FATAL, "unable to read ", *ptr, ": ");
			if (line.len == 0)
				break;
			line.s[line.len - 1] = 0;
			if (!str_diff(line.s, fn)) {
				close(fd);
				return 1;
			}
		}
		close(fd);
	}
	return 0;
}

void
show_internals(char *home)
{
	substdio_puts(subfdout, "indimail-mta  home dir: ");
	substdio_puts(subfdout, auto_qmail);
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "user        assign dir: ");
	substdio_puts(subfdout, auto_assign);
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "tcp         access dir: ");
	substdio_puts(subfdout, auto_sysconfdir);
	substdio_puts(subfdout, "/tcp\n");

	substdio_puts(subfdout, "certificates       dir: ");
	substdio_puts(subfdout, auto_sysconfdir);
	substdio_puts(subfdout, "/certs\n");

	substdio_puts(subfdout, "control      irect dir: ");
	substdio_puts(subfdout, controldir);
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "global environment dir: ");
	substdio_puts(subfdout, controldir);
	substdio_puts(subfdout, "/defaultqueue\n");

	if (home && !access(".defaultqueue", F_OK) && !chdir(".defaultqueue")) {
		substdio_puts(subfdout, "user   environment dir: ");
		substdio_puts(subfdout, home);
		substdio_puts(subfdout, "/.defaultqueue\n");
	}

	substdio_puts(subfdout, "surbl        cache dir: ");
	substdio_puts(subfdout, controldir);
	substdio_puts(subfdout, "/cache\n");

	substdio_puts(subfdout, "domainkeys         dir: ");
	substdio_puts(subfdout, controldir);
	substdio_puts(subfdout, "/domainkeys\n");

	substdio_puts(subfdout, "\n");
	substdio_puts(subfdout, "user-ext delimiter      : ");
	substdio_puts(subfdout, auto_break);
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "paternalism (in decimal): ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_patrn));
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "silent concurrency limit: ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_spawn));
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "\n");
	substdio_put(subfdout, "user  ids\n", 10);
	if (uidinit(1, 1))
		strerr_die2sys(111, FATAL, "unable to initialize uids/gids: ");
	substdio_put(subfdout, " alias   : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uida));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmaild  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidd));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmaill  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidl));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailp  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidp));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailq  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidq));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmailr  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidr));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmails  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uids));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " indimail: ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidi));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qscand  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_uidv));
	substdio_put(subfdout, "\n\n", 2);
	substdio_put(subfdout, "group ids\n", 10);
	substdio_put(subfdout, " nofiles : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidn));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qmail   : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidq));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " indimail: ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidi));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qscand  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidv));
	substdio_put(subfdout, "\n", 1);
	substdio_put(subfdout, " qcerts  : ", 11);
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) auto_gidc));
	substdio_put(subfdout, "\n", 1);
}

void
show_concurrency(DIR *dir)
{
	direntry       *d;

	while ((d = readdir(dir))) {
		if (str_equal(d->d_name, ".") || str_equal(d->d_name, ".."))
			continue;
		if (str_start(d->d_name, "concurrencyr.")) {
			print_concurrency();
			do_int(d->d_name, "20", "Remote  Queue concurrency is ", "");
			continue;
		}
		if (str_start(d->d_name, "concurrencyl.")) {
			print_concurrency();
			do_int(d->d_name, "10", "Local   Queue concurrency is ", "");
			continue;
		}
	}
	print_concurrency();
}

void
show_errors(DIR *dir)
{
	direntry       *d;

	while ((d = readdir(dir))) {
		if (str_equal(d->d_name, ".") || str_equal(d->d_name, ".."))
			continue;
		if (str_equal(d->d_name, "libindimail") ||
				str_equal(d->d_name, "ratelimit") ||
				str_equal(d->d_name, "cache") ||
				str_equal(d->d_name, "defaultqueue"))
			continue;
		if (valid_control_files(d->d_name))
			continue;
		if (str_start(d->d_name, "concurrencyl."))
			continue;
		if (str_start(d->d_name, "concurrencyr."))
			continue;
		substdio_puts(subfderr, d->d_name);
		substdio_puts(subfderr, ": I have no idea what this file does\n");
	}
}

stralloc        qdir = { 0 };
void
show_queues()
{
	char           *qbase;
	int             save, i, j, k, l, bigtodo;
	char            strnum[FMT_ULONG];
	char         **ptr;
	char           *extra_queue[] = {"slowq", "nqueue", "qmta", 0};

	getEnvConfigInt(&bigtodo, "BIGTODO", 1);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	substdio_puts(subfdout, "subdirectory split      : ");
	substdio_put(subfdout, num, fmt_ulong(num, (unsigned long) conf_split));
	substdio_puts(subfdout, "\n");

	substdio_puts(subfdout, "big todo / big intd     : ");
	substdio_puts(subfdout, bigtodo ? "Yes\n" : "No\n");

	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&qdir, "queue_base", 0))
		{
		case -1:
			strerr_die2sys(111, FATAL, "Unable to read control file queue_base: ");
			break;
		case 0:
			if (!stralloc_copys(&qdir, auto_qmail) ||
					!stralloc_catb(&qdir, "/queue", 6))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 1:
			qdir.len--; /*- remove NULL put by control_readfile() */
			break;
		}
	} else {
		if (!stralloc_copys(&qdir, qbase))
			strerr_die2x(111, FATAL, "out of memory");
	}
	save = qdir.len;
	if (!stralloc_0(&qdir))
		strerr_die2x(111, FATAL, "out of memory");
	substdio_puts(subfdout, "indimail-mta  queue base: ");
	substdio_puts(subfdout, qdir.s);
	substdio_puts(subfdout, "\n\n");
	qdir.len--;
	if (chdir(qdir.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", qdir.s, ": ");
	for (i = 1, k = 0;; i++) {
		qdir.len = save;
		strnum[j = fmt_int(strnum, i)] = 0;
		if (!stralloc_catb(&qdir, "/queue", 6) ||
				!stralloc_catb(&qdir, strnum, j) ||
				!stralloc_catb(&qdir, "/lock/sendmutex", 15) ||
				!stralloc_0(&qdir))
			strerr_die2x(111, FATAL, "out of memory");
		if (!access(qdir.s, F_OK)) {
			if (i == 1) {
				k = 1;
				substdio_puts(subfdout, "main  queue directories\n");
			}
			qdir.len = save + 6 + j;
			if (!stralloc_0(&qdir))
				strerr_die2x(111, FATAL, "out of memory");
			subprintf(subfdout, "queue %03d %s\n", i, qdir.s);
		} else
			break;
	}
	for (i = 1, ptr = extra_queue; *ptr; ptr++) {
		qdir.len = save;
		if (!stralloc_append(&qdir, "/") ||
				!stralloc_cats(&qdir, *ptr))
			strerr_die2x(111, FATAL, "out of memory");
		l = qdir.len;
		if (!stralloc_catb(&qdir, "/lock/sendmutex", 15) ||
				!stralloc_0(&qdir))
			strerr_die2x(111, FATAL, "out of memory");
		if (!access(qdir.s, F_OK)) {
			if (i++ == 1) {
				if (k)
					substdio_puts(subfdout, "\n");
				substdio_puts(subfdout, "extra queue directories\n");
			}
			qdir.len = l;
			if (!stralloc_0(&qdir))
				strerr_die2x(111, FATAL, "out of memory");
			subprintf(subfdout, "%-6s %s\n", *ptr, qdir.s);
		}
	}
}

int
main(int argc, char **argv)
{
	DIR            *dir;
	char           *ptr;
	int             opt, do_control = 0, do_internals = 0, do_concurrency = 0,
					do_queue = 0, do_errors = 0, do_env = 0, qstat;
	pid_t           pid;
	char           *svctool[] = { "svctool", "--dumpconfig", 0};
	stralloc        bin = {0};

	while ((opt = getopt(argc, argv, "acCiqeEs")) != opteof) {
		switch(opt)
		{
		case 'a':
			do_control = do_concurrency = do_internals = do_queue = do_errors = do_env = 1;
			break;
		case 'c':
			do_control = 1;
			break;
		case 'C':
			do_concurrency = 1;
			break;
		case 'i':
			do_internals = 1;
			break;
		case 'q':
			do_queue = 1;
			break;
		case 'e':
			do_errors = 1;
			break;
		case 'E':
			do_env = 1;
			break;
		case 's':
			if (!stralloc_copys(&bin, auto_prefix) ||
					!stralloc_catb(&bin, "/sbin/svctool", 13) ||
					!stralloc_0(&bin))
				strerr_die2x(111, FATAL, "out of memory");
			execv(bin.s, svctool); /*- run svctool */
			strerr_die4sys(111, FATAL, "execv: ", *svctool, ": ");
			break;
		default:
			strerr_die2x(100, FATAL,
					"usage: qmail-showctl [-cCie]\n"
					"      options:\n"
					"        -c show control file information\n"
					"        -C show concurrency limits for deliveries\n"
					"        -i show internal information\n"
					"        -q Display indimail-mta queues\n"
					"        -s Dump entire indimail-mta configuration\n"
					"        -e Display files that shouldn't be there\n"
					"        -E Display default env variables set\n"
					"        -a Do all of the above");
		}
	}
	if (!do_control && !do_concurrency && !do_internals && !do_errors && !do_queue && !do_env)
		do_queue = do_internals = 1;

	ptr = env_get("HOME");
	env_clear();
	if (ptr && !env_put2("HOME", ptr))
		die_nomem();
	set_environment(WARN, FATAL, 1);
	if (do_env) {
		substdio_puts(subfdout, "------------------ begin show env ----------------------------\n");
		substdio_flush(subfdout);
		switch ((pid = fork()))
		{
		case -1:
			strerr_die2sys(111, FATAL, "fork: ");
		case 0:
			execlp("env", "env", (char *) 0);
			strerr_die2sys(111, FATAL, "execv: env: ");
		default:
			if (wait_pid(&qstat, pid) == -1)
				strerr_die2sys(111, FATAL, "wait_pid: ");
			if (wait_crashed(qstat))
				strerr_die2x(111, FATAL, "env program crashed");
		}
		substdio_puts(subfdout, "\n");
	}
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;

	if (do_internals) {
		if ((ptr = env_get("HOME"))) {
			if (chdir(ptr) == -1)
				die_chdir(ptr);
		}
		substdio_puts(subfdout, "------------------ begin show internals ----------------------\n");
		show_internals(ptr);
		substdio_puts(subfdout, "\n");
	}
	if (do_queue) {
		if (chdir(auto_qmail) == -1)
			die_chdir(auto_qmail);
		substdio_puts(subfdout, "--------------------- begin show queues ----------------------\n");
		show_queues();
		substdio_puts(subfdout, "\n");
	}

	if (chdir(controldir) == -1)
		die_chdir(controldir);

	if (!(dir = opendir(controldir)))
		strerr_die4sys(111, FATAL, "unable to open directory ", controldir, ": ");

	if (do_concurrency) {
		substdio_puts(subfdout, "-------------------- concurrency limits ----------------------\n");
		show_concurrency(dir);
	}
	if (do_control) {
		substdio_puts(subfdout, "------------------- begin control files ----------------------\n");
		display_control();
		substdio_puts(subfdout, "\n");
	}
	if (do_errors) {
		rewinddir(dir);
		substdio_puts(subfderr, "------------------- begin unknown files ----------------------\n");
		show_errors(dir);
		substdio_puts(subfderr, "\n");
	}
	closedir(dir);
	substdio_flush(subfderr);
	substdio_flush(subfdout);
	_exit(0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_showctl_c()
{
	static char    *x = "$Id: qmail-showctl.c,v 1.14 2023-05-31 16:31:02+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
