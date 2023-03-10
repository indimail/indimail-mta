/*-
 * RCS log at bottom
 * $Id: qmail-remote.c,v 1.165 2023-03-10 13:12:05+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cdb.h>
#include <open.h>
#include <ctype.h>
#include <sig.h>
#include <env.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <scan.h>
#include <case.h>
#include <error.h>
#include <alloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <str.h>
#include <now.h>
#include <constmap.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <fmt.h>
#include <base64.h>
#include <wait.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <hmac.h>
#include <authmethods.h>
#include <noreturn.h>

#include "hastlsa.h"
#if defined(TLS) && defined(HASTLSA)
#include "tlsarralloc.h"
#include "fn_handler.h"
#include <netdb.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#else
#warning "not compiled with -DTLS -DHASTLSA"
#endif /*- #if defined(TLS) && defined(HASTLSA) */

#ifdef TLS
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <tls.h>
#ifdef HASTLSA
#include "tlsacheck.h"
#endif
#endif /*- #ifdef TLS */
#include "dossl.h"

#include "haslibgsasl.h"
#if defined(HASLIBGSASL) && defined(TLS)
#include <gsasl.h>
#include <openssl/ssl.h>
#endif

/* email address internationalization EAI */
#include "hassmtputf8.h"
#ifdef SMTPUTF8
#include <idn2.h>
#endif

#include "cdb_match.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"
#include "control.h"
#include "dns.h"
#include "quote.h"
#include "ip.h"
#include "ipalloc.h"
#include "ipme.h"
#include "tcpto.h"
#include "timeoutconn.h"
#include "variables.h"
#include "socket.h"
#include "qr_digest_md5.h"
#include "parse_env.h"
#include "varargs.h"

#define EHLO 1
#define HUGESMTPTEXT  5000
#define MIN_PENALTY   3600
#define MAX_TOLERANCE 120

#define PORT_SMTP     25 /*- silly rabbit, /etc/services is for users */
#define PORT_QMTP     209

GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)

typedef struct constmap CONSTMAP;
typedef unsigned long my_ulong;
typedef struct ip_mx IPMX;
typedef union v46addr V46ADDR;

static my_ulong port = PORT_SMTP;
static stralloc sauninit = { 0 };
static stralloc helohost = { 0 };
static stralloc senderbind = { 0 };
static stralloc localips = { 0 };
static stralloc helohosts = { 0 };
static stralloc outgoingip = { 0 };

static stralloc smtproutes = { 0 };
static stralloc qmtproutes = { 0 };

static CONSTMAP mapsmtproutes;
static CONSTMAP mapqmtproutes;
static stralloc bounce = { 0 };

static int      protocol_t = 0;	/*- defines smtps, smtp, qmtp */
#ifdef MXPS
static int      mxps = 0;
#endif
static CONSTMAP maplocalips;
static CONSTMAP maphelohosts;
static stralloc host = { 0 };
static stralloc rhost = { 0 }; /*- host to which qmail-remote ultimately connects */
static stralloc qmtp_sender = { 0 };
static stralloc smtp_sender = { 0 };
static stralloc qqeh = { 0 };
static stralloc user = { 0 };
static stralloc pass = { 0 };
static stralloc slop = { 0 };
static stralloc chal = { 0 };
static saa      qmtp_reciplist = { 0 };
static saa      smtp_reciplist = { 0 };

static IPMX     partner;
static V46ADDR  outip;
static int      inside_greeting = 0;
static char    *msgsize, *use_auth_smtp;
static char   **my_argv;
static int      my_argc;
#ifdef TLS
static char    *ssl_err_str = NULL;
static SSL     *ssl;
static int      notls = 0;
static stralloc notlshosts = { 0 } ;
static CONSTMAP mapnotlshosts;
static int      smtps;
#endif

static int      fdmoreroutes = -1;
static int      flagtcpto = 1;
static int      min_penalty = MIN_PENALTY;
static my_ulong max_tolerance = MAX_TOLERANCE;
static stralloc smtptext = { 0 };
static stralloc smtpenv = { 0 };
static stralloc helo_str = { 0 };

/*- http://mipassoc.org/pipermail/batv-tech/2007q4/000032.html */
#ifdef BATV
#include "batv.h"
#include <openssl/ssl.h>
#include "byte.h"
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/evp.h>
#else
#include <openssl/md5.h>
#endif
static char     batvok = 0;
static stralloc batvkey = { 0 };
static stralloc nosignremote = { 0 };
static stralloc nosignlocals = { 0 };

static CONSTMAP mapnosignremote;
static CONSTMAP mapnosignlocals;
#endif

#if defined(TLS) && defined(HASTLSA)
static char    *do_tlsa = NULL, *tlsadomainsfn = NULL;
static int      use_daned = 0;
static tlsarralloc ta = { 0 };
static stralloc hexstring = { 0 };
static stralloc tlsadomains = { 0 };
static CONSTMAP maptlsadomains;
#endif

#ifdef SMTPUTF8
static stralloc header = { 0 };
static stralloc idnhost = { 0 };
static int      smtputf8 = 0;       /*- if remote has SMTPUTF8 capability */
static char    *enable_utf8 = NULL; /*- enable utf8 using SMTPUTF8 env variable */
static int      flagutf8;           /*- sender, recipient or received header has UTF8 */
#endif

#if defined(HASLIBGSASL) && defined(TLS)
static stralloc gsasl_str = { 0 };
typedef enum {
	tls_unique,
	tls_exporter,
} cb_type;
#endif

void            temp_nomem();

void
out(char *s)
{
	if (substdio_puts(subfdoutsmall, s) == -1)
		_exit(0);
}

void
my_error(char *s1, char *s2, char *s3)
{
	if (substdio_puts(subfderr, s1) == -1)
		_exit(0);
	if (s2) {
		if (substdio_put(subfderr, ": ", 2) == -1 ||
				substdio_puts(subfderr, s2) == -1)
			_exit(0);
	}
	if (s3) {
		if (substdio_put(subfderr, ": ", 2) == -1 ||
				substdio_puts(subfderr, s3) == -1)
			_exit(0);
	}
	if (substdio_put(subfderr, ": ", 2) == -1 ||
			substdio_puts(subfderr, error_str(errno)) == -1 ||
			substdio_put(subfderr, "\n", 1) == -1 ||
			substdio_flush(subfderr) == -1)
		_exit(0);
}

/*-
 * run script defined by ONSUCCESS_REMOTE, ONFAILURE_REMOTE, ONTEMPORARY_REMOTE
 * succ  1 - success
 * succ  0 - perm failure
 * succ -1 - temp failure
 */
int
run_script(char code, int succ)
{
	char           *prog, *str;
	char            remote_code[2] = "\0\0";
	char          **args;
	int             child, wstat, i;

	switch (succ)
	{
	case 1:/*- success */
		str = "ONSUCCESS_REMOTE";
		break;
	case 0:/*- failure */
		str = "ONFAILURE_REMOTE";
		break;
	case -1:/*- transient error */
		str = "ONTEMPORARY_REMOTE";
		break;
	default:
		return 0;
	}
	if (!(prog = env_get(str)))
		return (0);
	remote_code[0] = code;
	if (!env_put2("SMTPSTATUS", remote_code)) {
		my_error("alert: Out of memory", 0, 0);
		_exit(1);
	}
	if (!(args = (char **) alloc(my_argc + 1))) {
		my_error("alert: Out of memory", 0, 0);
		_exit(1);
	}
	switch (child = fork())
	{
	case -1:
		my_error("alert: fork failed", 0, 0);
		_exit(0);
		return (1);
	case 0:
		switch (succ)
		{
		case 1:/*- success */
			if (!env_unset("ONFAILURE_REMOTE")) {
				my_error("alert: out of memory", 0, 0);
				_exit(1);
			}
			if (!env_unset("ONTEMPORARY_REMOTE")) {
				my_error("alert: out of memory", 0, 0);
				_exit(1);
			}
			break;
		case 0:/*- failure */
			if (!env_unset("ONSUCCESS_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit(1);
			}
			if (!env_unset("ONTEMPORARY_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit(1);
			}
			break;
		case -1:
			if (!env_unset("ONSUCCESS_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit(1);
			}
			if (!env_unset("ONFAILURE_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit(1);
			}
			break;
		}
		/*- copy all arguments */
		for (i = 0; i < my_argc; i++)
			args[i] = my_argv[i];
		args[i] = NULL;
		execv(prog, args);
		my_error("alert: Unable to run", prog, 0);
		_exit(1);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat)) {
		my_error("alert", prog, "crashed");
		_exit(1);
	}
	return (wait_exitcode(wstat));
}

void
zero()
{
	if (substdio_put(subfdoutsmall, "\0", 1) == -1)
		_exit(0);
}

void
outsafe(stralloc *sa)
{
	int             i;
	char            ch;

	for (i = 0; i < sa->len; ++i) {
		ch = sa->s[i];
		if (ch < 33)
			ch = '?';
		if (ch > 126)
			ch = '?';
		if (substdio_put(subfdoutsmall, &ch, 1) == -1)
			_exit(0);
	}
}

extern void _exit (int __status) __attribute__ ((__noreturn__));
/*-
 * RETURN VALUES:
 * succ  1 - success
 * succ  0 - perm failure
 * succ -1 - temp failure
 * execute user definded script/executable using run_script()
 */
no_return void
zerodie(char *s1, int succ)
{
#ifdef TLS
	if (ssl) {
		while (SSL_shutdown(ssl) == 0)
			usleep(100);
		SSL_free(ssl);
		ssl = NULL;
	}
#endif
	zero();
	substdio_flush(subfdoutsmall);
	_exit(run_script(s1 ? s1[0] : 'Z', succ));
}

/*
 * set smtpenv variable which gets used for SMTPTEXT environment variable
 * return 1 on NOMEM
 * smtptenv      = Text obtained from remote host during conversation
 * smtptenv null = Conversation with remote host did not happen at all
 * smtptext      = Text set internally indicating error with processing email
 * code xxx      = Actual smtp status code (250, 454, 553, etc)
 * code   0      = Temporary failure. No conversation with remote host took place
 */
int
setsmtptext(int code, int type)
{
	if (env_get("ONFAILURE_REMOTE") || env_get("ONSUCCESS_REMOTE")
		|| env_get("ONTEMPORARY_REMOTE")) {
		char            strnum[FMT_ULONG];

		/*- copy smtptext to smtpenv */
		if (!smtpenv.len && smtptext.len && !stralloc_copy(&smtpenv, &smtptext))
			return (1);
		if (smtpenv.len) {
			if (smtpenv.s[smtpenv.len - 1] == '\n')
				smtpenv.s[smtpenv.len - 1] = 0;
			else
			if (!stralloc_0(&smtpenv))
				return (1);
			smtpenv.len--;
			if (!env_put2((type == 'q' || mxps == 1) ? "QMTPTEXT" : "SMTPTEXT", smtpenv.s)) {
				return (1);
			}
		}
		if (code >= 0) {
			strnum[fmt_ulong(strnum, code)] = '\0';
			if (!env_put2("SMTPCODE", strnum))
				return (1);
		} else /*- shouldn't happen - no additional code available from remote host. BUG in code */
		if (!env_put2((type == 'q' || mxps == 1) ? "QMTPCODE" : "SMTPCODE", "-1"))
			temp_nomem();
	}
	return (0);
}

no_return void
temp_nomem()
{
	out("ZOut of memory. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Out of memory. (#4.3.0)"))
		zerodie("Z", -1);
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_noip()
{
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	out("Zinvalid ipaddr in ");
	out(controldir);
	out("/outgoingip (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "invalid ipaddr in ") ||
			!stralloc_cats(&smtptext, controldir) ||
			!stralloc_cats(&smtptext, "/outgoingip (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_oserr()
{
	out("ZSystem resources temporarily unavailable. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "System resources temporarily unavailable. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_write()
{
	out("ZUnable to write message. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to write message. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_read()
{
	out("ZUnable to read message. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to read message. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_dnscanon()
{
	out("ZCNAME lookup failed temporarily. (#4.4.3)\n");
	if (!stralloc_copys(&smtptext, "CNAME lookup failed temporarily. (#4.4.3)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_dns()
{
	out("ZSorry, I couldn't find any host by that name. (#4.1.2)\n");
	if (!stralloc_copys(&smtptext, "Sorry, I couldn't find any host by that name. (#4.1.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_dns_rr()
{
	out("ZSorry, I couldn't find TLSA RR for this host. (#4.1.2)\n");
	if (!stralloc_copys(&smtptext, "Sorry, I couldn't find any TLSA RR for this host. (#4.1.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_chdir()
{
	out("ZUnable to switch to home directory. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to switch to home directory. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_control(char *arg1, char *arg2)
{
	out("Z");
	out(arg1);
	out(". (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, arg1))
		temp_nomem();
	if (arg2) {
		if (!stralloc_catb(&smtptext, " [", 2) ||
				!stralloc_cats(&smtptext, arg2) ||
				!stralloc_append(&smtptext, "]"))
			temp_nomem();
	}
	if (!stralloc_catb(&smtptext, " (#4.3.0)", 8))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_cdb(char *arg)
{
	out("Z");
	out(arg);
	out(". (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, arg))
		temp_nomem();
	if (!stralloc_catb(&smtptext, " (#4.3.0)", 8))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
perm_partialline()
{
	char           *r = "DSMTP cannot transfer messages with partial final lines. (#5.6.2)\n";

	out(r);
	if (!stralloc_copys(&smtptext, r + 1))
		temp_nomem();
	smtptext.len--;
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

no_return void
perm_usage()
{
	char           *r = "DI (qmail-remote) was invoked improperly. (#5.3.5)\n";

	out(r);
	if (!stralloc_copys(&smtptext, r + 3))
		temp_nomem();
	smtptext.len--;
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

no_return void
perm_dns()
{
	char           *r = "DSorry, I couldn't find any host named ";

	out(r);
	outsafe(&host);
	out(". (#5.1.2)\n");
	if (!stralloc_copys(&smtptext, r + 1) ||
			!stralloc_cat(&smtptext, &host) ||
			!stralloc_cats(&smtptext, ". (#5.1.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

no_return void
perm_nomx()
{
	char           *r = "DSorry, I couldn't find a mail exchanger or IP address. (#5.4.4)\n";

	out(r);
	if (!stralloc_copys(&smtptext, r + 1))
		temp_nomem();
	smtptext.len--;
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

no_return void
perm_ambigmx()
{
	char           *r = "DSorry. Although I'm listed as a best-preference MX or A for that host,\nit isn't in my ";

	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	out(r);
	out(controldir);
	out("/locals file, so I don't treat it as local. (#5.4.6)\n");
	if (!stralloc_copys(&smtptext, r + 1) ||
			!stralloc_cats(&smtptext, controldir) ||
			!stralloc_cats(&smtptext, "/locals file, so I don't treat it as local. (#5.4.6)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

no_return void
perm_tlsclientmethod()
{
	char           *r = "DInvalid TLS method configured. (#5.3.5)\n";

	out(r);
	if (!stralloc_copys(&smtptext, r + 3))
		temp_nomem();
	smtptext.len--;
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

void
outhost()
{
	char            x[IPFMT];
	unsigned int    len;

	switch (partner.af)
	{
#ifdef IPV6
	case AF_INET6:
		len = ip6_fmt(x, &partner.addr.ip6);
		break;
#endif
	case AF_INET:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	default:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	}
	if (!stralloc_copyb(&rhost, x, len) ||
			!stralloc_0(&rhost))
		temp_nomem();
	if (substdio_put(subfdoutsmall, x, len) == -1)
		_exit(0);
	if (!env_put2("SMTPHOST", rhost.s))
		temp_nomem();
}

int             flagcritical = 0;

no_return void
dropped()
{
	char            strnum[FMT_ULONG];

	out("ZConnected to ");
	outhost();
	out(" but connection died. ");
	if (!stralloc_copys(&smtptext, "Connected to ") ||
			!stralloc_copyb(&smtptext, rhost.s, rhost.len - 1) ||
			!stralloc_cats(&smtptext, " but connection died. "))
		temp_nomem();
	if (flagcritical) {
		out("Possible duplicate! ");
		if (!stralloc_cats(&smtptext, "Possible duplicate! "))
			temp_nomem();
	} else
	if (inside_greeting) { /*- RFC 2821 - client should treat connection failures as temporary error */
		out("Will try alternate MX. ");
		if (!stralloc_cats(&smtptext, "Will try alternate MX. "))
			temp_nomem();
		if (flagtcpto) {
			out("tcpto interval ");
			strnum[fmt_ulong(strnum, max_tolerance)] = '\0';
			out(strnum);
			out(" ");
			if (!stralloc_cats(&smtptext, "tcpto interval ") ||
					!stralloc_cats(&smtptext, strnum) ||
					!stralloc_catb(&smtptext, " ", 1))
				temp_nomem();
			tcpto_err(&partner, 1, max_tolerance);
		}
	}
#ifdef TLS
	if (ssl_err_str) {
		out(ssl_err_str);
		out(" ");
		if (!stralloc_cats(&smtptext, ssl_err_str) ||
				!stralloc_catb(&smtptext, " ", 1))
			temp_nomem();
	}
#endif
	out("(#4.4.2)\n");
	if (!stralloc_cats(&smtptext, "(#4.4.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_noconn_out(char *s)
{
	out(s);
	if (!stralloc_cats(&smtptext, smtptext.len ? s : s + 1))
		temp_nomem();
	return;
}

no_return void
temp_noconn(stralloc *h, char *ip, int port_num)
{
	char            strnum[FMT_ULONG];

	if (!stralloc_copys(&smtptext, ""))
		temp_nomem();
	temp_noconn_out("ZSorry, I wasn't able to establish a ");
	temp_noconn_out((protocol_t == 'q' || mxps == 1) ? "QMTP connection for " : "SMTP connection for ");
	if (substdio_put(subfdoutsmall, h->s, h->len) == -1)
		_exit(0);
	if (!stralloc_catb(&smtptext, h->s, h->len))
		temp_nomem();
	if (ip) {
		temp_noconn_out(" to ");
		temp_noconn_out(ip);
		temp_noconn_out(" port ");
		strnum[fmt_ulong(strnum, port_num)] = '\0';
		temp_noconn_out(strnum);
		temp_noconn_out(" bind IP [");
		if (substdio_put(subfdoutsmall, outgoingip.s, outgoingip.len) == -1)
			_exit(0);
		temp_noconn_out("]");
		if (!stralloc_catb(&smtptext, outgoingip.s, outgoingip.len) ||
				!stralloc_copys(&rhost, ip) ||
				!stralloc_0(&rhost))
			temp_nomem();
		if (!env_put2("SMTPHOST", rhost.s))
			temp_nomem();
		alloc_free(ip);
	}
	temp_noconn_out(". (#4.4.1)\n");
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
temp_qmtp_noconn(stralloc *h, char *ip, int port_num)
{
	char            strnum[FMT_ULONG];

	temp_noconn_out("ZSorry, I wasn't able to establish a QMTP connection for ");
	if (substdio_put(subfdoutsmall, h->s, h->len) == -1)
		_exit(0);
	if (ip) {
		temp_noconn_out(" to ");
		temp_noconn_out(ip);
		temp_noconn_out(" port ");
		strnum[fmt_ulong(strnum, port_num)] = '\0';
		temp_noconn_out(strnum);
		if (!stralloc_copys(&rhost, ip) ||
				!stralloc_0(&rhost))
			temp_nomem();
		if (!env_put2("SMTPHOST", rhost.s))
			temp_nomem();
		alloc_free(ip);
	}
	temp_noconn_out(". (#4.4.1)\n");
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
err_authprot()
{
	out("Kno supported AUTH method found, continuing without authentication.\n");
	zero();
	substdio_flush(subfdoutsmall);
}

int             smtpfd;
int             timeoutconn = 60;
int             timeoutdata = 1200;

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

#ifdef TLS
	if (ssl) {
		if ((r = ssl_timeoutread(timeoutdata, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = myssl_error_str();
	} else
		r = timeoutread(timeoutdata, smtpfd, buf, len);
#else
	r = timeoutread(timeoutdata, smtpfd, buf, len);
#endif
	if (r <= 0)
		dropped();
	return r;
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

#ifdef TLS
	if (ssl) {
		if ((r = ssl_timeoutwrite(timeoutdata, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = myssl_error_str();
	} else
		r = timeoutwrite(timeoutdata, smtpfd, buf, len);
#else
	r = timeoutwrite(timeoutdata, smtpfd, buf, len);
#endif
	if (r <= 0)
		dropped();
	return r;
}

char            inbuf[1500];
char            smtptobuf[1500];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        smtpto = SUBSTDIO_FDBUF(safewrite, -1, smtptobuf, sizeof smtptobuf);
char            smtpfrombuf[128];
substdio        smtpfrom = SUBSTDIO_FDBUF(saferead, -1, smtpfrombuf, sizeof smtpfrombuf);

void
get1(char *ch)
{
	/*-
	 * substdio_get(&smtpfrom, ...) can either return here with exactly 1 byte
	 * or
	 * via calling dropped() in saferead()
	 */
	substdio_get(&smtpfrom, ch, 1);
	if (*ch != '\r' && smtptext.len < HUGESMTPTEXT &&
			!stralloc_append(&smtptext, ch))
		temp_nomem();
}

unsigned long
get3()
{
	char            str[4];
	int             i;
	unsigned long   code;

	/*-
	 * substdio_get(&smtpfrom, ...) can either return here with exactly 1 byte
	 * or
	 * via calling dropped() in saferead()
	 * calling substdio_get(&smtpfrom xxx, n) where n != 1 is unsafe unless
	 * you check the return value for bytes returned
	 */
	for (i = 0; i < 3; i++)
		get1(str + i);
	str[i] = '\0';
	scan_ulong(str, &code);
	return code;
}

unsigned long
smtpcode()
{
	unsigned char   ch;
	unsigned long   code;
	int             err = 0;

	if (!stralloc_copys(&smtptext, ""))
		temp_nomem();
	if ((code = get3()) < 200)
		err = 1;
	for (;;) {
		get1((char *) &ch);
		if (ch != ' ' && ch != '-')
			err = 1;
		if (ch != '-')
			break;
		while (ch != '\n')
			get1((char *) &ch);
		get3();
	}
	while (ch != '\n')
		get1((char *) &ch);
	return err ? 400 : code;
}

saa             ehlokw = { 0 };	/*- list of EHLO keywords and parameters */
int             maxehlokwlen = 0;

unsigned long
ehlo()
{
	stralloc       *saptr;
	char           *s, *e, *p;
	unsigned long   code;

	if (ehlokw.len > maxehlokwlen)
		maxehlokwlen = ehlokw.len;
	ehlokw.len = 0;
	if (protocol_t == 'q')		/*- QMTP */
		return 0;
	if (substdio_put(&smtpto, "EHLO ", 5) == -1 ||
			substdio_put(&smtpto, helohost.s, helohost.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 250)
		return code;
	s = smtptext.s;
	while (*s++ != '\n'); /*- skip the first line: contains the domain */
	e = smtptext.s + smtptext.len - 6;	/*- 250-?\n */
	while (s <= e) {
		int             wasspace = 0;

		if (!saa_readyplus(&ehlokw, 1))
			temp_nomem();
		saptr = ehlokw.sa + ehlokw.len++;
		if (ehlokw.len > maxehlokwlen)
			*saptr = sauninit;
		else
			saptr->len = 0;

		/*- smtptext is known to end in a '\n' */
		for (p = (s += 4);; ++p) {
			if (*p == '\n' || *p == ' ' || *p == '\t') {
				if (!wasspace)
					if (!stralloc_catb(saptr, s, p - s) || !stralloc_0(saptr))
						temp_nomem();
				if (*p == '\n')
					break;
				wasspace = 1;
			} else
			if (wasspace == 1) {
				wasspace = 0;
				s = p;
			}
		}
		s = ++p;

		/*
		 * keyword should consist of alpha-num and '-'
		 * broken AUTH might use '=' instead of space
		 */
		for (p = saptr->s; *p; ++p) {
			if (*p == '=') {
				*p = '\0';
				break;
			}
		}
	}
	return 250;
}

void
outsmtptext()
{
	int             i;

	if (smtptext.s && smtptext.len) {
		out("Remote host said: ");
		for (i = 0; i < smtptext.len; ++i) {
			if (!smtptext.s[i])
				smtptext.s[i] = '?';
		}
		if (substdio_put(subfdoutsmall, smtptext.s, smtptext.len) == -1)
			_exit(0);
		/*- copy smtptext to smtpenv since we are going to truncate smtptext */
		if (!stralloc_copy(&smtpenv, &smtptext))
			smtpenv.len = 0;
		smtptext.len = 0;
	}
}

/*
 * die =   0 - success
 * die =  -1 - failure system error
 * die =   1 - failure non-system error
 *
 * code =  xxx - smtp code
 * code =    0 - temporary failure
 */

no_return void
#ifdef  HAVE_STDARG_H
quit(int code, int die, char *prepend, ...)
#else
quit(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *str;
#ifndef HAVE_STDARG_H
	int             code, die;
	char           *prepend;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, prepend);
#else
	va_start(ap);
	code = va_arg(ap, int);
	die = va_arg(ap, int);
	prepend = va_arg(ap, char *);
#endif
	if (substdio_putflush(&smtpto, "QUIT\r\n", 6) == -1)
		temp_write();
	out(prepend);
	outhost();

	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		out(str);
	}
	out(".\n");
	va_end(ap);
	setsmtptext(code, 0);
	outsmtptext();
	zerodie(prepend, die == -1 ? -1 : !die);
}

void
blast()
{
	int             r, i, j, sol;
	char            in[4096], out[4096 * 2 + 1];

	for (r = 0; r < qqeh.len; r++) {
		if (qqeh.s[r] == '\n' && substdio_put(&smtpto, "\r", 1) == -1)
				temp_write();
		if (substdio_put(&smtpto, qqeh.s + r, 1) == -1)
			temp_write();
	}
	for (sol = 1;;) {
		if (!(r = substdio_get(&ssin, in, sizeof(in))))
			break;
		if (r == -1)
			temp_read();
		for (i = j = 0; i < r; ) {
			if (sol && in[i] == '.') {
				out[j++] = '.';
				out[j++] = in[i++];
			}
			sol = 0;
			while (i < r) {
				if (in[i] == '\n') {
					i++;
					sol = 1;
					out[j++] = '\r';
					out[j++] = '\n';
					break;
				}
				out[j++] = in[i++];
			} /*- while (i < r) */
		} /*- for (i = o = 0; i < r; ) */
		if (substdio_put(&smtpto, out, j) == -1)
			temp_write();
	}
	if (!sol)
		perm_partialline();
	flagcritical = 1;
	if (substdio_put(&smtpto, ".\r\n", 3) == -1)
		temp_write();
	substdio_flush(&smtpto);
}

#ifdef SMTPUTF8
int
containsutf8(char *p, int l)
{
	int             i = 0;

	while (i < l)
		if (p[i++] > 127)
			return 1;
	return 0;
}

void
checkutf8message()
{
	int             pos, i, r, state;
	char            ch;

	if (containsutf8(smtp_sender.s, smtp_sender.len)) {
		flagutf8 = 1;
		return;
	}
	for (i = 0; i < smtp_reciplist.len; ++i) {
		if (containsutf8(smtp_reciplist.sa[i].s, smtp_reciplist.sa[i].len)) {
			flagutf8 = 1;
			return;
		}
	}
	state = 0;
	pos = 0;
	/*
	 * "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)"
	 * "  by silverton.berkeley.edu with UTF8SMTP; 29 Nov 2020 04:46:54 -0000\n"
	 */
	for (;;) {
		if (!(r = substdio_get(&ssin, &ch, 1)))
			break;
		else
		if (r == -1)
			temp_read();
		if (ch == '\n' && !stralloc_cats(&header, "\r"))
			temp_nomem();
		if (!stralloc_append(&header, &ch))
			temp_nomem();
		if (ch == '\r')
			continue;
		if (ch == '\t')
			ch = ' ';
		if (ch == '\n' && flagutf8)
			return;
		switch (state)
		{
		case 6: /* in Received, at LF but before WITH clause */
			if (ch == ' ') {
				state = 3;
				pos = 1;
				continue;
			}
			state = 0;
			/*- FALL THROUGH */
		case 0: /* start of header field */
			if (ch == '\n')
				return;
			state = 1;
			pos = 0;
			/*- FALL THROUGH */
		case 1: /* partway through "Received:" */
			if (ch != "RECEIVED:"[pos] && ch != "received:"[pos]) {
				state = 2;
				continue;
			}
			if (++pos == 9) {
				state = 3;
				pos = 0;
			}
			continue;
		case 2: /* other header field */
			if (ch == '\n')
				state = 0;
			continue;
		case 3: /* in Received, before WITH clause or partway though " with " */
			if (ch == '\n') {
				state = 6;
				continue;
			}
			if (ch != " WITH "[pos] && ch != " with "[pos]) {
				pos = 0;
				continue;
			}
			if (++pos == 6) {
				state = 4;
				pos = 0;
			}
			continue;
		case 4: /* in Received, having seen with, before the argument */
			if (pos == 0 && ch == ' ')
				continue;
			if (ch != "UTF8"[pos] && ch != "utf8"[pos]) {
				state = 5;
				continue;
			}
			if (++pos == 4) {
				flagutf8 = 1;
				state = 5;
				continue;
			}
			continue;
		case 5: /* after the RECEIVED WITH argument */
			/*- blast() assumes that it copies whole lines */
			if (ch == '\n')
				return;
			state = 1;
			pos = 0;
			continue;
		}
	}
}
#endif

char           *partner_fqdn = NULL;
#ifdef TLS
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define SSL_ST_BEFORE 0x4000
#endif
no_return void
tls_quit(const char *s1, char *s2, char *s3, char *s4, char *s5, stralloc *saptr)
{
	char            ch;
	int             i, state;

	out((char *) s1);
	out((char *) s2);
	if (s3)
		out((char *) s3);
	if (s4)
		out((char *) s4);
	if (s5)
		out((char *) s5);
	if (saptr && saptr->len) {
		for (i = 0; i < saptr->len; ++i) {
			ch = saptr->s[i];
			if (ch < 33)
				ch = '?';
			if (ch > 126)
				ch = '?';
			if (substdio_put(subfdoutsmall, &ch, 1) == -1)
				_exit(0);
		}
	}

	/*-
	 * shouldn't talk to the client unless in an appropriate state
	 * https://mta.openssl.org/pipermail/openssl-commits/2015-October/002060.html
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	state = ssl ? SSL_get_state(ssl) : SSL_ST_BEFORE;
	if ((state & TLS_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		if (substdio_putflush(&smtpto, "QUIT\r\n", 6) == -1)
			temp_write();
#else
	state = ssl ? ssl->state : SSL_ST_BEFORE;
	if ((state & SSL_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		if (substdio_putflush(&smtpto, "QUIT\r\n", 6) == -1)
			temp_write();
#endif
	out(ssl ? "; connected to " : "; connecting to ");
	outhost();
	out(".\n");
	setsmtptext(-1, 0);
	outsmtptext();
	if (env_get("DEBUG") && ssl) {
		X509           *peercert;

		out("STARTTLS proto=");
		out((char *) SSL_get_version(ssl));
		out("; cipher=");
		out((char *) SSL_get_cipher(ssl));

		/*- we want certificate details */
		if ((peercert = SSL_get_peer_certificate(ssl))) {
			char           *str;

			str = X509_NAME_oneline(X509_get_subject_name(peercert), NULL, 0);
			out("; subject=");
			out(str);
			OPENSSL_free(str);

			str = X509_NAME_oneline(X509_get_issuer_name(peercert), NULL, 0);
			out("; issuer=");
			out(str);
			OPENSSL_free(str);

			X509_free(peercert);
		}
		out(";\n");
	}
	zerodie((char *) s1, -1);
}

#ifdef HASTLSA
no_return void
tlsa_error(char *str)
{
	out("Z");
	out(str);
	out(": Unable to fetch TLSA RR. (#4.5.0)\n");
	if (!stralloc_copys(&smtptext, str) ||
			!stralloc_catb(&smtptext, ": Unable to fetch TLSA RR. (#4.5.0)", 35))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

static stralloc temphost = { 0 };
static stralloc sa = { 0 };
int
get_tlsa_rr(char *mxhost, int port_num)
{
	char            strnum[FMT_ULONG];
	int             r;

	if (temphost.len && !str_diffn(temphost.s, mxhost, temphost.len))
		return (DNS_MEM);
	else
	if (!stralloc_copys(&temphost, mxhost) || !stralloc_0(&temphost))
		return (DNS_MEM);
	if (!stralloc_copyb(&sa, "_", 1) ||
			!stralloc_catb(&sa, strnum, fmt_uint(strnum, port_num)) ||
			!stralloc_catb(&sa, "._tcp.", 6) ||
			!stralloc_cats(&sa, mxhost))
		return (DNS_MEM);
	return ((r = dns_tlsarr(&ta, &sa)));
}
#endif /*- #ifdef HASTLSA */
#endif /*- #ifdef TLS */

static stralloc xuser = { 0 };
static stralloc auth = { 0 };
static stralloc plain = { 0 };

int
xtext(stralloc *ptr, char *s, int len)
{
	int             i;

	if (!stralloc_copys(ptr, ""))
		temp_nomem();
	for (i = 0; i < len; i++) {
		if (s[i] == '+') {
			if (!stralloc_cats(ptr, "+2B"))
				temp_nomem();
		} else
		if (s[i] == '=') {
			if (!stralloc_cats(ptr, "+3D"))
				temp_nomem();
		} else
		if ((int) s[i] < 33 || (int) s[i] > 126) {
			if (!stralloc_cats(ptr, "+3F"))
				temp_nomem();	/*- ok. not correct */
		} else
		if (!stralloc_catb(ptr, s + i, 1))
			temp_nomem();
	}
	return (ptr->len ? ptr->len : 1);
}

void
mailfrom(int use_size)
{
	if (substdio_put(&smtpto, "MAIL FROM:<", 11) == -1 ||
			substdio_put(&smtpto, smtp_sender.s, smtp_sender.len) == -1)
		temp_write();
	if (use_size) {
		if (substdio_put(&smtpto, "> SIZE=", 7) == -1 ||
				substdio_puts(&smtpto, msgsize) == -1)
			temp_write();
	} else
	if (substdio_put(&smtpto, ">", 1) == -1)
		temp_write();
#ifdef SMTPUTF8
	if (enable_utf8 && smtputf8 && flagutf8 &&
			substdio_put(&smtpto, " SMTPUTF8", 9) == -1)
		temp_write();
#endif
	if (substdio_put(&smtpto, "\r\n", 2) == -1)
		temp_write();
}

void
mailfrom_xtext(int use_size)
{
	if (!xtext(&xuser, user.s, user.len))
		temp_nomem();
	if (substdio_put(&smtpto, "MAIL FROM:<", 11) == -1 ||
			substdio_put(&smtpto, smtp_sender.s, smtp_sender.len) == -1)
		temp_write();
	if (use_size) {
		if (substdio_put(&smtpto, "> SIZE=", 7) == -1 ||
				substdio_puts(&smtpto, msgsize) == -1 ||
				substdio_put(&smtpto, " AUTH=<", 7) == -1)
			temp_write();
	} else
	if (substdio_put(&smtpto, "> AUTH=<", 8) == -1)
		temp_write();
	if (substdio_put(&smtpto, xuser.s, xuser.len) == -1 ||
			substdio_put(&smtpto, ">", 1) == -1)
		temp_write();
#ifdef SMTPUTF8
	if (enable_utf8 && smtputf8 && flagutf8 &&
			substdio_put(&smtpto, " SMTPUTF8", 9) == -1)
		temp_write();
#endif
	if (substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
}

static char     hextab[] = "0123456789abcdef";
static stralloc realm = { 0 }, nonce = { 0 }, digesturi = { 0 };

/*- parse digest response */
unsigned int
scan_response(stralloc *dst, stralloc *src, const char *search)
{
	char           *x = src->s;
	int             i, len;
	unsigned int    slen;

	slen = str_len((char *) search);
	if (!stralloc_copys(dst, ""))
		temp_nomem();
	for (i = 0; src->len > i + slen; i += str_chr(x + i, ',') + 1) {
		char           *s = x + i;
		if (case_diffb(s, slen, (char *) search) == 0) {
			s += slen;			/*- skip name */
			if (*s++ != '=')
				return 0;		/*- has to be here! */
			if (*s == '"') {	/*- var="value" */
				s++;
				len = str_chr(s, '"');
				if (!len)
					return 0;
				if (!stralloc_catb(dst, s, len))
					temp_nomem();
			} else {			/*- var=value */
				len = str_chr(s, ',');
				if (!len)
					str_len(s);	/*- should be the end */
				if (!stralloc_catb(dst, s, len))
					temp_nomem();
			}
			return dst->len;
		}
	}
	return 0;
}

/*
 * reference: https://www.rfc-editor.org/rfc/rfc4954
 */
no_return static void
decode_smtpauth_err(int code, char *s1, char *s2)
{
	switch(code)
	{
	case 432:
		quit(code, 1, "ZConnected to ", " but a password transition is needed ", "(", s1, s2, ") (#4.7.12).", 0);
		break;
	case 454:
		quit(code, 1, "ZConnected to ", " but got temporary authentication failure ", s1, s2, ") (#4.7.0).", 0);
		break;
	case 534:
		quit(code, 1, "DConnected to ", " but authentication mechanism is too weak ", s1, s2, ") (#5.7.9).", 0);
		break;
	case 535:
		quit(code, 1, "DConnected to ", " but authentication credentials invalid ", s1, s2, ") (#5.7.8).", 0);
		break;
	case 500:
		quit(code, 1, "DConnected to ", " but authentication Exchange line is too long ", s1, s2, ") (#5.5.6).", 0);
		break;
	case 530:
		quit(code, 1, "DConnected to ", " but authentication required ", s1, s2, ") (#5.7.0).", 0);
		break;
	case 538:
		quit(code, 1, "DConnected to ", " but encryption required for requested authentication mechanism ", s1, s2, ") (#5.7.11).", 0);
		break;
	default:
		quit(code, 1, "ZConnected to ", " but authentication was rejected ", s1, s2, ") (#4.7.0)", 0);
		break;
	}
}

void
auth_digest_md5(int use_size)
{
	unsigned char   unique[FMT_ULONG + FMT_ULONG + 3];
	unsigned char   digest[20], cnonce[41];
	unsigned char  *s, *x = cnonce;
	unsigned long   i, len = 0;
	int             code;
	char            z[IPFMT];

	if (substdio_put(&smtpto, "AUTH DIGEST-MD5\r\n", 17) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334)
		quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH DIGEST-MD5)", 0);
	if ((i = str_chr(smtptext.s + 5, '\n')) > 0) {	/*- Challenge */
		slop.len = 0;
		if (!stralloc_copyb(&slop, smtptext.s + 4, smtptext.len - 5))
			temp_nomem();
		if (b64decode((unsigned char *) slop.s, slop.len, &chal))
			quit(-1, -1, "ZConnected to ", " but unable to base64decode challenge", 0);
	} else
		quit(-1, 1, "ZConnected to ", " but got no challenge", 0);

	if (scan_response(&nonce, &chal, "nonce") == 0)
		quit(-1, 1, "ZConnected to ", " but got no response= in challenge");
	s = unique;
	s += (i = fmt_uint((char *) s, getpid()));
	len += i;
	s += (i = fmt_str((char *) s, "."));
	len += i;
	s += (i = fmt_ulong((char *) s, (unsigned long) now()));
	len += i;
	s += (i = fmt_str((char *) s, "@"));
	len += i;
	*s++ = '\0';
	hmac_sha1(unique, len, unique + 3, len - 3, digest);
	for (i = 0; i < 20; i++) {
		*x = hextab[digest[i] / 16];
		++x;
		*x = hextab[digest[i] % 16];
		++x;
	}
	*x = '\0';
	if (!stralloc_copyb(&slop, "cnonce=\"", 8) ||
			!stralloc_catb(&slop, (char *) cnonce, 32) ||
			!stralloc_catb(&slop, "\",digest-uri=\"", 14) ||
			!stralloc_copyb(&digesturi, "smtp/", 5))
		temp_nomem();
	switch (partner.af)
	{
#ifdef IPV6
	case AF_INET6:
		len = ip6_fmt(z, &partner.addr.ip6);
		break;
#endif
	case AF_INET:
		len = ip4_fmt(z, &partner.addr.ip);
		break;
	default:
		len = ip4_fmt(z, &partner.addr.ip);
		break;
	}
	if (!stralloc_catb(&digesturi, z, len) ||
			!stralloc_cat(&slop, &digesturi) ||
			!stralloc_catb(&slop, "\",nc=00000001,nonce=\"", 21) ||
			!stralloc_cat(&slop, &nonce) ||
			!stralloc_catb(&slop, "\",qop=\"auth\",realm=\"", 20))
		temp_nomem();
	if (control_readfile(&realm, "realm", 1) == -1)
		temp_control("Unable to read control file", "realm");
	realm.len--;
	if (!stralloc_cat(&slop, &realm) ||
			!stralloc_catb(&slop, "\",response=", 11))
		temp_nomem();
	if (!
		(s =
		 (unsigned char *) qr_digest_md5(user.s, user.len, realm.s, realm.len, pass.s, pass.len, 0, nonce.s, nonce.len, digesturi.s,
									  digesturi.len, (char *) cnonce, "00000001", "auth")))
		quit(-1, -1, "ZConnected to ", " but unable to generate response", 0);
	if (!stralloc_cats(&slop, (char *) s) ||
			!stralloc_catb(&slop, ",username=\"", 11) ||
			!stralloc_cat(&slop, &user) ||
			!stralloc_catb(&slop, "\"", 1))
		temp_nomem();
	if (b64encode(&slop, &auth))
		quit(-1, -1, "ZConnected to ", " but unable to base64encode username+digest", 0);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334)
		quit(code, 1, "ZConnected to ", " but authentication was rejected (username+digest)", 0);
	if (substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 235)
		decode_smtpauth_err(code, "AUTH ", "DIGEST-MD5");
	mailfrom_xtext(use_size);
}

void
auth_cram(int type, int use_size)
{
	int             j, code, iter = 16;
	unsigned char   digest[129], encrypted[65];
	unsigned char  *e;

	switch (type)
	{
	case AUTH_CRAM_MD5:
		iter = MD5_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-MD5\r\n", 15) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-MD5)", 0);
		break;
	case AUTH_CRAM_RIPEMD:
		iter = RIPEMD160_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-RIPEMD\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-RIPEMD)", 0);
		break;
	case AUTH_CRAM_SHA1:
		iter = SHA_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA1\r\n", 16) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA1)", 0);
		break;
	case AUTH_CRAM_SHA224:
		iter = SHA224_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA224\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA224)", 0);
		break;
	case AUTH_CRAM_SHA256:
		iter = SHA256_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA256\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA256)", 0);
		break;
	case AUTH_CRAM_SHA384:
		iter = SHA384_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA384\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA384)", 0);
		break;
	case AUTH_CRAM_SHA512:
		iter = SHA512_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA512\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA512)", 0);
		break;
	}
	if ((j = str_chr(smtptext.s + 5, '\n')) > 0) {	/*- Challenge */
		if (!stralloc_copys(&slop, "") ||
				!stralloc_copyb(&slop, smtptext.s + 4, smtptext.len - 5))
			temp_nomem();
		if (b64decode((unsigned char *) slop.s, slop.len, &chal))
			quit(-1, -1, "ZConnected to ", " but unable to base64decode challenge", 0);
	} else
		quit(-1, 1, "ZConnected to ", " but got no challenge", 0);
	switch (type)
	{
	case AUTH_CRAM_MD5:
		hmac_md5((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA1:
		hmac_sha1((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA224:
		hmac_sha224((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA256:
		hmac_sha256((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA384:
		hmac_sha384((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA512:
		hmac_sha512((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_RIPEMD:
		hmac_ripemd((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	}

	for (j = 0, e = encrypted; j < iter; j++) {	/*- HEX => ASCII */
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = '\0';

	/*- copy user-id and digest */
	if (!stralloc_copy(&slop, &user) ||
			!stralloc_catb(&slop, " ", 1) ||
			!stralloc_cats(&slop, (char *) encrypted))
		temp_nomem();

	if (b64encode(&slop, &auth))
		quit(-1, -1, "ZConnected to ", " but unable to base64encode username+digest", 0);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1)
		temp_write();
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		decode_smtpauth_err(code, "AUTH ", get_authmethod(type));
	mailfrom_xtext(use_size);
}

void
auth_plain(int use_size)
{
	int             code;

	if (substdio_put(&smtpto, "AUTH PLAIN\r\n", 12) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334)
		quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH PLAIN)", 0);
	if (!stralloc_copy(&plain, &smtp_sender) || /*- Mail From: <auth-id> */
			!stralloc_0(&plain) ||
			!stralloc_cat(&plain, &user) || /*- userid */
			!stralloc_0(&plain) ||
			!stralloc_cat(&plain, &pass)) /*- password */
		temp_nomem();
	if (b64encode(&plain, &auth))
		quit(-1, -1, "ZConnected to ", " but unable to base64encode (plain)", 0);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 235)
		decode_smtpauth_err(code, "AUTH ", "PLAIN");
	mailfrom_xtext(use_size);
}

void
auth_login(int use_size)
{
	int             code;

	if (substdio_put(&smtpto, "AUTH LOGIN\r\n", 12) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334)
		quit(code, 1, "ZConnected to ", " but authentication was rejected (AUTH LOGIN)", 0);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&user, &auth))
		quit(-1, -1, "ZConnected to ", " but unable to base64encode user", 0);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 334)
		quit(code, 1, "ZConnected to ", " but authentication was rejected (username)", 0);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&pass, &auth))
		quit(-1, -1, "ZConnected to ", " but unable to base64encode pass", 0);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 235)
		decode_smtpauth_err(code, "AUTH ", "LOGIN");
	mailfrom_xtext(use_size);
}

#if defined(HASLIBGSASL) && defined(TLS)
static void
remove_newline()
{
	int             i;

	if (!stralloc_copyb(&gsasl_str, smtptext.s + 4, smtptext.len - 4))
		temp_nomem();
	i = str_rchr(gsasl_str.s, '\r');
	if (gsasl_str.s[i])
		gsasl_str.s[i] = '\0';
	else {
		i = str_rchr(gsasl_str.s, '\n');
		if (gsasl_str.s[i])
			gsasl_str.s[i] = '\0';
	}
	return;
}

static void
gsasl_authenticate(Gsasl_session *session, char *mech)
{
	char           *p;
	int             rc, code;

	if (substdio_put(&smtpto, "AUTH ", 5) == -1 ||
			substdio_puts(&smtpto, mech) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334) {
		if (!stralloc_copyb(&gsasl_str, " but authentication was rejected (AUTH ", 39) ||
				!stralloc_cats(&gsasl_str, mech) ||
				!stralloc_append(&gsasl_str, ")") ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(code, 1, "ZConnected to ", gsasl_str.s, 0);
	}

	if (!stralloc_0(&gsasl_str))
		temp_nomem();
	/*- This loop mimics a protocol where the client send data first. */
	do {
		/* -
		 * skip smtp code in gsasl_str.s (3 bytes + 1 sp)
		 * Generate client output.
		 */
		rc = gsasl_step64(session, gsasl_str.s, &p);
		if (rc == GSASL_NEEDS_MORE || rc == GSASL_OK) {
			if (substdio_puts(&smtpto, p) == -1 ||
					substdio_put(&smtpto, "\r\n", 2) == -1 ||
					substdio_flush(&smtpto) == -1)
				temp_write();
			gsasl_free(p);
		}
		if (rc == GSASL_NEEDS_MORE) {
			/*
			 * If the client need more data from server, get it here.
			 */
			if ((code = smtpcode()) != 334) {
				if (!stralloc_copyb(&gsasl_str, " but authentication was rejected (AUTH ", 39) ||
						!stralloc_cats(&gsasl_str, mech) ||
						!stralloc_append(&gsasl_str, ")") ||
						!stralloc_0(&gsasl_str))
					temp_nomem();
				quit(code, 1, "ZConnected to ", gsasl_str.s, 0);
			}
			remove_newline();
		}
	} while (rc == GSASL_NEEDS_MORE);
	if (rc != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " but authentication failed: ", 28) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, 1, "ZConnected to ", gsasl_str.s, 0);
	}
	/*
	 * The client is done.  Here you would typically check if the server
	 * let the client in.  If not, you could try again.
	 */
	if ((code = smtpcode()) != 235) {
		if (!stralloc_copyb(&gsasl_str, " but authentication was rejected (AUTH ", 39) ||
				!stralloc_cats(&gsasl_str, mech) ||
				!stralloc_append(&gsasl_str, ")") ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(code, 1, "ZConnected to ", gsasl_str.s, 0);
	}
}

#ifdef TLS
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1
char           *
get_finish_message(cb_type type)
{
	char            tls_finish_buf[EVP_MAX_MD_SIZE];
	static stralloc in = {0}, res = {0};
	int             tls_finish_len;
#if GSASL_VERSION_NUMBER >= 0x020002
	int             i;
#endif

	if (!ssl) /*- we should never be here */
		return ((char *) NULL);
	switch (type)
	{
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1
	case tls_unique: /*- RFC 5929 */
		/*
		 * Save the TLS finish message expected to be found, useful for
		 * authentication checks related to channel binding.
		 * SSL_get_peer_finished() does not offer a way to know the exact length
		 * of a TLS finish message beforehand, so attempt first with a fixed-length
		 * buffer, and try again if the message does not fit.
		 */
		tls_finish_len = SSL_get_finished(ssl, tls_finish_buf, EVP_MAX_MD_SIZE);
		if (tls_finish_len > EVP_MAX_MD_SIZE)
			return ((char *) NULL);
		break;
#endif
#if GSASL_VERSION_NUMBER >= 0x020002
	case tls_exporter: /*- RFC 9266 tls-exporter length = 32 */
		tls_finish_len = 32;
		if ((i = SSL_export_keying_material(ssl, (unsigned char *) tls_finish_buf,
						tls_finish_len, "EXPORTER-Channel-Binding", 24, 0, 0, 1)) != 1) {
			return ((char *) NULL);
		}
		break;
#endif
	default:
		return ((char *) NULL);
	}
	if (!stralloc_copyb(&in, tls_finish_buf, tls_finish_len) ||
			b64encode(&in, &res) != 0 || !stralloc_0(&res))
		temp_nomem();
	return res.s;
}
#endif /*- #if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1 */

void
do_channel_binding(Gsasl_session *sctx)
{
	char           *p = (char *) NULL;
	int             i;

	switch ((i = SSL_version(ssl)))
	{
	case TLS1_2_VERSION:
		if (!(p = get_finish_message(tls_unique)))
			quit(-1, -1, "ZConnected to ", " but unable to set channel binding for GSASL_CB_TLS_UNIQUE", 0);
#if GSASL_VERSION_MAJOR > 1
		if (gsasl_property_set(sctx, GSASL_CB_TLS_UNIQUE, p) != GSASL_OK)
			quit(-1, -1, "ZConnected to ", " but unable to set channel binding for GSASL_CB_TLS_UNIQUE", 0);
#else
		gsasl_property_set(sctx, GSASL_CB_TLS_UNIQUE, p);
#endif
		break;
#if GSASL_VERSION_NUMBER >= 0x020002
#if defined(TLS1_3_VERSION)
	case TLS1_3_VERSION:
		if(!(p = get_finish_message(tls_exporter)))
			quit(-1, -1, "ZConnected to ", " but unable to get finish message for GSASL_CB_TLS_EXPORTER", 0);
		if (gsasl_property_set(sctx, GSASL_CB_TLS_EXPORTER, p) != GSASL_OK)
			quit(-1, -1, "ZConnected to ", " but unable to set channel binding for GSASL_CB_TLS_EXPORTER", 0);
		break;
#endif
#endif
	default:
		quit(-1, -1, "ZConnected to ", " but got unknown channel binding", 0);
	}
}

static void
gsasl_client(Gsasl *gsasl_ctx, int method)
{
	Gsasl_session  *session;
	const char     *mech;
	int             i, rc;
	char            strnum[FMT_ULONG];

	switch (method)
	{
	case AUTH_SCRAM_SHA1:
		mech = "SCRAM-SHA-1";
		break;
	case AUTH_SCRAM_SHA256:
		mech = "SCRAM-SHA-256";
		break;
	case AUTH_SCRAM_SHA1_PLUS:
		mech = "SCRAM-SHA-1-PLUS";
		break;
	case AUTH_SCRAM_SHA256_PLUS:
		mech = "SCRAM-SHA-256-PLUS";
		break;
	default:
		strnum[i = fmt_int(strnum, method)] = '\0';
		if (!stralloc_copyb(&gsasl_str, " but got unknown method=", 24) ||
				!stralloc_catb(&gsasl_str, strnum, i) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, 1, "ZConnected to ", gsasl_str.s, 0);
		break;
	}
	/* Create new authentication session.  */
	if ((rc = gsasl_client_start(gsasl_ctx, mech, &session)) != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to initialize client: ", 30) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, -1, "ZConnected to ", gsasl_str.s, 0);
	}

	if (method == AUTH_SCRAM_SHA1_PLUS || method == AUTH_SCRAM_SHA256_PLUS)
		do_channel_binding(session);
	/*
	 * Set username and password in session handle. This info will be
	 * lost when this session is deallocated below.
	 */
#if GSASL_VERSION_MAJOR > 1
	rc = gsasl_property_set(session, GSASL_AUTHID, user.s);
	if (rc != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to set username: ", 25) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, -1, "ZConnected to ", gsasl_str.s, 0);
		return;
	}
	rc = gsasl_property_set(session, env_get("SALTED_PASSWORD") ? GSASL_SCRAM_SALTED_PASSWORD : GSASL_PASSWORD, pass.s);
	if (rc != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to set password: ", 25) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, -1, "ZConnected to ", gsasl_str.s, 0);
		return;
	}
#else
	gsasl_property_set(session, GSASL_AUTHID, user.s);
	gsasl_property_set(session, env_get("SALTED_PASSWORD") ? GSASL_SCRAM_SALTED_PASSWORD : GSASL_PASSWORD, pass.s);
#endif
	gsasl_authenticate(session, mech);
	/* Cleanup.  */
	gsasl_finish(session);
}

void
auth_scram(int method, int use_size)
{
	Gsasl          *gsasl_ctx = NULL;
	int             rc;

	/* Initialize library. */
	if ((rc = gsasl_init(&gsasl_ctx)) != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to initialize libgsasl: ", 32) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit(-1, -1, "ZConnected to ", gsasl_str.s, 0);
	}
	/* Do it.  */
	gsasl_client(gsasl_ctx, method);
	/* Cleanup.  */
	gsasl_done(gsasl_ctx);
	mailfrom_xtext(use_size);
	return;
}
#endif /*- #ifdef HASLIBGSASL */
#endif /*- #ifdef TLS */

void
smtp_auth(char *type, int use_size)
{
	int             i = 0, login_supp = 0, plain_supp = 0, cram_md5_supp = 0, cram_sha1_supp = 0,
					cram_sha224_supp, cram_sha256_supp = 0, cram_sha384_supp, cram_sha512_supp = 0,
					cram_rmd_supp = 0, digest_md5_supp = 0;
	char           *ptr, *no_auth_login, *no_auth_plain, *no_cram_md5, *no_cram_sha1, *no_cram_sha224,
				   *no_cram_sha256, *no_cram_sha384, *no_cram_sha512, *no_cram_ripemd, *no_digest_md5;
#ifdef TLS
	int             secure_auth;
#endif
#if defined(HASLIBGSASL) && defined(TLS)
	int             scram_sha1_supp = 0, scram_sha256_supp = 0, scram_sha1_plus_supp = 0,
					scram_sha256_plus_supp = 0;
	char           *no_scram_sha1, *no_scram_sha256, *no_scram_sha1_plus, *no_scram_sha256_plus;
#endif

	/*-
	 * cycle through all lines got as response to EHLO
	 * break when you find 2XX-AUTH or 2XX AUTH
	 */
	while ((i += str_chr(smtptext.s + i, '\n') + 1) && (i + 8 < smtptext.len)
		   && str_diffn(smtptext.s + i + 4, "AUTH ", 5));

	for (ptr = smtptext.s + i + 4 + 5; *ptr != '\n' && ptr < smtptext.s + smtptext.len; ptr++) {
		if (*ptr == '-') {
			if (case_starts(ptr - 6, "DIGEST-MD5"))
				digest_md5_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-RIPEMD"))
				cram_rmd_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA512"))
				cram_sha512_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA384"))
				cram_sha384_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA256"))
				cram_sha256_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA224"))
				cram_sha224_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA1"))
				cram_sha1_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-MD5"))
				cram_md5_supp = 1;
#if defined(HASLIBGSASL) && defined(TLS)
			else
			if (ssl && case_starts(ptr - 5, "SCRAM-SHA-256-PLUS"))
				scram_sha256_plus_supp = 1;
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-256"))
				scram_sha256_supp = 1;
			else
			if (ssl && case_starts(ptr - 5, "SCRAM-SHA-1-PLUS"))
				scram_sha1_plus_supp = 1;
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-1"))
				scram_sha1_supp = 1;
#endif
		} else
		if (*ptr == 'L') {
			if (case_starts(ptr, "LOGIN"))
				login_supp = 1;
		} else
		if (*ptr == 'P') {
			if (case_starts(ptr, "PLAIN"))
				plain_supp = 1;
		}
	}
#ifdef TLS
	secure_auth = env_get("SECURE_AUTH") ? 1 : 0;
	no_auth_login = secure_auth && !ssl ? "" : env_get("DISABLE_AUTH_LOGIN");
	no_auth_plain = secure_auth && !ssl ? "" : env_get("DISABLE_AUTH_PLAIN");
#else
	no_auth_login = env_get("DISABLE_AUTH_LOGIN");
	no_auth_plain = env_get("DISABLE_AUTH_PLAIN");
#endif
	no_cram_md5 = env_get("DISABLE_CRAM_MD5");
	no_cram_sha1 = env_get("DISABLE_CRAM_SHA1");
	no_cram_sha224 = env_get("DISABLE_CRAM_SHA224");
	no_cram_sha256 = env_get("DISABLE_CRAM_SHA256");
	no_cram_sha384 = env_get("DISABLE_CRAM_SHA384");
	no_cram_sha512 = env_get("DISABLE_CRAM_SHA512");
	no_cram_ripemd = env_get("DISABLE_CRAM_RIPEMD");
	no_digest_md5 = env_get("DISABLE_DIGEST_MD5");
#if defined(HASLIBGSASL) && defined(TLS)
	no_scram_sha1 = env_get("DISABLE_SCRAM_SHA1");
	no_scram_sha256 = env_get("DISABLE_SCRAM_SHA256");
	no_scram_sha1_plus = env_get("DISABLE_SCRAM_SHA1_PLUS");
	no_scram_sha256_plus = env_get("DISABLE_SCRAM_SHA256_PLUS");
#endif
	if (!*type) {
#if defined(HASLIBGSASL) && defined(TLS)
		if (!no_scram_sha256_plus && scram_sha256_plus_supp) {
			auth_scram(AUTH_SCRAM_SHA256_PLUS, use_size);
			return;
		} else
		if (!no_scram_sha256 && scram_sha256_supp) {
			auth_scram(AUTH_SCRAM_SHA256, use_size);
			return;
		} else
		if (!no_scram_sha1_plus && scram_sha1_plus_supp) {
			auth_scram(AUTH_SCRAM_SHA1_PLUS, use_size);
			return;
		} else
		if (!no_scram_sha1 && scram_sha1_supp) {
			auth_scram(AUTH_SCRAM_SHA1, use_size);
			return;
		}
#endif
		if (!no_digest_md5 && digest_md5_supp) {
			auth_digest_md5(use_size);
			return;
		}
		if (!no_cram_ripemd && cram_rmd_supp) {
			auth_cram(AUTH_CRAM_RIPEMD, use_size);
			return;
		}
		if (!no_cram_sha512 && cram_sha512_supp) {
			auth_cram(AUTH_CRAM_SHA512, use_size);
			return;
		}
		if (!no_cram_sha384 && cram_sha384_supp) {
			auth_cram(AUTH_CRAM_SHA384, use_size);
			return;
		}
		if (!no_cram_sha256 && cram_sha256_supp) {
			auth_cram(AUTH_CRAM_SHA256, use_size);
			return;
		}
		if (!no_cram_sha224 && cram_sha224_supp) {
			auth_cram(AUTH_CRAM_SHA224, use_size);
			return;
		}
		if (!no_cram_sha1 && cram_sha1_supp) {
			auth_cram(AUTH_CRAM_SHA1, use_size);
			return;
		}
		if (!no_cram_md5 && cram_md5_supp) {
			auth_cram(AUTH_CRAM_MD5, use_size);
			return;
		}
		if (!no_auth_plain && plain_supp) {
			auth_plain(use_size);
			return;
		}
		if (!no_auth_login && login_supp) {
			auth_login(use_size);
			return;
		}
	} else
#if defined(HASLIBGSASL) && defined(TLS)
	if (scram_sha256_plus_supp && !case_diffs(type, "SCRAM-SHA-256-PLUS")) {
		auth_scram(AUTH_SCRAM_SHA256_PLUS, use_size);
		return;
	} else
	if (scram_sha1_plus_supp && !case_diffs(type, "SCRAM-SHA-1-PLUS")) {
		auth_scram(AUTH_SCRAM_SHA1_PLUS, use_size);
		return;
	} else
	if (scram_sha256_supp && !case_diffs(type, "SCRAM-SHA-256")) {
		auth_scram(AUTH_SCRAM_SHA256, use_size);
		return;
	} else
	if (scram_sha1_supp && !case_diffs(type, "SCRAM-SHA-1")) {
		auth_scram(AUTH_SCRAM_SHA1, use_size);
		return;
	} else
#endif
	if (login_supp && !case_diffs(type, "LOGIN")) {
		auth_login(use_size);
		return;
	} else
	if (plain_supp && !case_diffs(type, "PLAIN")) {
		auth_plain(use_size);
		return;
	} else
	if (cram_md5_supp && !case_diffs(type, "CRAM-MD5")) {
		auth_cram(AUTH_CRAM_MD5, use_size);
		return;
	} else
	if (cram_sha1_supp && !case_diffs(type, "CRAM-SHA1")) {
		auth_cram(AUTH_CRAM_SHA1, use_size);
		return;
	} else
	if (cram_sha1_supp && !case_diffs(type, "CRAM-SHA224")) {
		auth_cram(AUTH_CRAM_SHA224, use_size);
		return;
	} else
	if (cram_sha256_supp && !case_diffs(type, "CRAM-SHA256")) {
		auth_cram(AUTH_CRAM_SHA256, use_size);
		return;
	} else
	if (cram_sha384_supp && !case_diffs(type, "CRAM-SHA384")) {
		auth_cram(AUTH_CRAM_SHA384, use_size);
		return;
	} else
	if (cram_sha512_supp && !case_diffs(type, "CRAM-SHA512")) {
		auth_cram(AUTH_CRAM_SHA512, use_size);
		return;
	} else
	if (cram_rmd_supp && !case_diffs(type, "CRAM-RIPEMD")) {
		auth_cram(AUTH_CRAM_RIPEMD, use_size);
		return;
	} else
	if (digest_md5_supp && !case_diffs(type, "DIGEST-MD5")) {
		auth_digest_md5(use_size);
		return;
	}
	err_authprot();
	mailfrom(use_size);
	return;
}

no_return void
temp_proto()
{
	out("Zrecipient did not talk proper QMTP (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "recipient did not talk proper QMTP (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

#ifdef MXPS
/*
 * Here is the general rule. Particular MX distances are assigned to protocols as follows:
 *
 *  - 12801, 12817, 12833, 12849, 12865, ..., 13041: QMTP. If the client supports MXPS and
 *  QMTP, it tries a QMTP connection to port 209. If the client does not support MXPS or QMTP,
 *  or if the QMTP connection attempt fails, the client tries a SMTP connection to port 25 as
 *  usual. The client does not try SMTP if the QMTP connection attempt succeeds but mail
 *  delivery through that connection fails.
 */
int
qmtp_priority(int pref)
{
	if (pref < 12800)
		return 0;
	if (pref > 13055)
		return 0;
	if (pref % 16 == 1)
		return 1;
	return 0;
}
#endif

no_return void
qmtp(stralloc *h, char *ip, int port_num)
{
	struct stat     st;
	unsigned long   len;
	char           *x;
	int             i, n, flagallok;
	unsigned char   ch;
	char            num[FMT_ULONG];

	if (fstat(0, &st) == -1)
		quit(-1, -1, "Z", "unable to fstat fd 0", 0);
	len = st.st_size;
	/*-
 	 * the following code was substantially taken from serialmail'ss serialqmtp.c
 	 */
	if (substdio_put(&smtpto, num, fmt_ulong(num, len + 1)) == -1 ||
			substdio_put(&smtpto, ":\n", 2) == -1)
		temp_write();
	while (len > 0) {
		if ((n = substdio_feed(&ssin)) <= 0)
			_exit(32);	/*- wise guy again */
		x = substdio_PEEK(&ssin);
		if (substdio_put(&smtpto, x, n) == -1)
			temp_write();
		substdio_SEEK(&ssin, n);
		len -= n;
	}
	len = qmtp_sender.len;
	if (substdio_put(&smtpto, ",", 1) == -1 ||
			substdio_put(&smtpto, num, fmt_ulong(num, len)) == -1 ||
			substdio_put(&smtpto, ":", 1) == -1 ||
			substdio_put(&smtpto, qmtp_sender.s, qmtp_sender.len) == -1 ||
			substdio_put(&smtpto, ",", 1) == -1)
		temp_write();
	len = 0;
	for (i = 0; i < qmtp_reciplist.len; ++i)
		len += fmt_ulong(num, qmtp_reciplist.sa[i].len) + 1 + qmtp_reciplist.sa[i].len + 1;
	if (substdio_put(&smtpto, num, fmt_ulong(num, len)) == -1 ||
			substdio_put(&smtpto, ":", 1) == -1)
		temp_write();
	for (i = 0; i < qmtp_reciplist.len; ++i) {
		if (substdio_put(&smtpto, num, fmt_ulong(num, qmtp_reciplist.sa[i].len)) == -1 ||
				substdio_put(&smtpto, ":", 1) == -1 ||
				substdio_put(&smtpto, qmtp_reciplist.sa[i].s, qmtp_reciplist.sa[i].len) == -1 ||
				substdio_put(&smtpto, ",", 1) == -1)
			temp_write();
	}
	if (substdio_put(&smtpto, ",", 1) == -1)
		temp_write();
	substdio_flush(&smtpto);
	flagallok = 1;
	for (i = 0; i < qmtp_reciplist.len; ++i) {
		len = 0;
		for (;;) {
			get1((char *) &ch);
			if (ch == ':')
				break;
			if (len > 200000000)
				temp_proto();
			if (ch - '0' > 9)
				temp_proto();
			len = 10 * len + (ch - '0');
		}
		if (!len)
			temp_proto();
		get1((char *) &ch);
		--len;
		if ((ch != 'Z') && (ch != 'D') && (ch != 'K'))
			temp_proto();
		if (!stralloc_copyb(&smtptext, (char *) &ch, 1) ||
				!stralloc_cats(&smtptext, "Remote host said: "))
			temp_nomem();
		while (len > 0) {
			get1((char *) &ch);
			--len;
		}
		for (len = 0; len < smtptext.len; ++len) {
			ch = smtptext.s[len];
			if ((ch < 32) || (ch > 126))
				smtptext.s[len] = '?';
		}
		get1((char *) &ch);
		if (ch != ',')
			temp_proto();
		smtptext.s[smtptext.len - 1] = '\n';
		if (smtptext.s[0] == 'K')
			out("r");
		else
		if (smtptext.s[0] == 'D') {
			out("h");
			flagallok = 0;
		} else {				/*- if (smtptext.s[0] == 'Z') */
			out("s");
			flagallok = -1;
		}
		if (substdio_put(subfdoutsmall, smtptext.s + 1, smtptext.len - 1) == -1)
			temp_qmtp_noconn(h, ip, port_num);
		zero();
	} /*- for (i = 0; i < qmtp_reciplist.len; ++i) */
	if (flagallok == 1) {
		out("K");
		outhost();
		out(" accepted message - Protocol QMTP\n");
		zerodie("K", flagallok);
	} else {
		out("DGiving up on ");
		outhost();
		out(" - Protocol QMTP\n");
		zerodie("DGiving up on ", flagallok);
	}
}

#if defined(TLS) && defined(HASTLSA)
no_return void
timeoutfn()
{
	out("Zerror connecting to DANE verification service. (#4.4.2)\n");
	if (!stralloc_copys(&smtptext, "Zerror connecting to DANE verification service. (#4.4.2)\n"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

no_return void
err_tmpfail(char *arg)
{
	out("ZTemporory failure with DANE verification service [");
	out(arg);
	out("]. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "ZTemporory failure with DANE verification service [") ||
			!stralloc_cats(&smtptext, arg) ||
			!stralloc_catb(&smtptext, "]. (#4.3.0)\n", 12))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}
#endif

void
do_smtp(char *fqdn)
{
	unsigned long   code;
	int             flagbother;
	int             i, use_size = 0, is_esmtp = 1, auth_capa = 0;
#if defined(TLS) && defined(HASTLSA)
	char           *err_str = NULL, *servercert = NULL;
	int             tlsa_status, authfullMatch, authsha256, authsha512,
					match0Or512, needtlsauth, usage, j;
	char            hex[2], rbuf[2];
	tlsarr         *rp;
#endif

#ifdef SMTPUTF8
	smtputf8 = 0;
#endif
	inside_greeting = 1;
#ifdef TLS
	if (protocol_t == 'S' || (protocol_t != 'q' && port == 465))
		smtps = 1;
	if (!smtps)
		code = smtpcode();
	else
		code = 220;
#else /*- TLS */
	code = smtpcode();
#endif /*- #ifdef TLS */
	inside_greeting = 0;
	if (code >= 500 && code < 600)
		quit(code, 1, "DConnected to ", " but greeting failed", 0);
	else
	if (code >= 400 && code < 500)
		return;		/*- try next MX, see RFC-2821 */
	else
	if (code != 220)
		quit(code, 1, "ZConnected to ", " but greeting failed", 0);
	/*-
 	 * RFC2487 says we should issue EHLO (even if we might not need
 	 * extensions); at the same time, it does not prohibit a server
 	 * to reject the EHLO and make us fallback to HELO
 	 */
#ifdef TLS
	if (!smtps)
		code = ehlo();
#ifdef HASTLSA
	if (do_tlsa && ta.len) {
		if (notls)
			quit(430, 1, "ZConnected to ", " but host is in notlshosts", 0);
		match0Or512 = authfullMatch = authsha256 = authsha512 = 0;
		if (!do_tls(&ssl, 0, smtps, smtpfd, &needtlsauth, &servercert, partner_fqdn, host.s, host.len, tls_quit,
					temp_nomem, temp_control, temp_write, quit, &smtptext, &ehlokw, 0)) /*- tls is needed for DANE */
			quit(430, -1, "ZConnected to ", " but unable to intiate TLS for DANE", 0);
		for (j = 0, usage = -1; j < ta.len; ++j) {
			rp = &(ta.rr[j]);
			if (!rp->mtype || rp->mtype == 2)
				match0Or512 = 1;
			for (i = hexstring.len = 0; i < rp->data_len; i++) {
				fmt_hexbyte(hex, (rp->data + i)[0]);
				if (!stralloc_catb(&hexstring, hex, 2))
					temp_nomem();
			}
			if (!stralloc_0(&hexstring))
				temp_nomem();
			if (!rp->usage || rp->usage == 2)
				usage = 2;
			if (!(tlsa_status = tlsa_vrfy_records(ssl, hexstring.s, rp->usage, rp->selector,
							rp->mtype, partner_fqdn, tls_quit, temp_nomem, &smtptext, NULL, NULL, &err_str, 0))) {
				switch(rp->mtype)
				{
				case 0:
					authfullMatch = 1;
					break;
				case 1:
					authsha256 = 1;
					break;
				case 2:
					authsha512 = 1;
					break;
				}
			}
			if (match0Or512)
				break;
		} /*- for (j = 0, usage = -1; j < ta.len; ++j) */
		/*-
		 * client SHOULD accept a server public key that
		 * matches either the "3 1 0" record or the "3 1 2" record, but it
		 * SHOULD NOT accept keys that match only the weaker "3 1 1" record.
		 * 9.  Digest Algorithm Agility
		 * https://tools.ietf.org/html/rfc7671
		 */
		if ((!match0Or512 && authsha256) || match0Or512) {
			(void) tlsacheck(do_tlsa, fqdn, UPDATE_SUCCESS, rbuf, timeoutfn, err_tmpfail);
			if (needtlsauth && (!usage || usage == 2))
				do_pkix(ssl, servercert, partner_fqdn, tls_quit, temp_nomem, &smtptext);
			code = ehlo();
		} else { /*- dane validation failed */
			if (use_daned)
				(void) tlsacheck(do_tlsa, fqdn, UPDATE_FAILURE, rbuf, timeoutfn, err_tmpfail);
			quit(534, 1, "DConnected to ", " but recpient failed DANE validation", 0);
		}
	} else /*- no tlsa rr records */
	if (notls) /*- if found in control/notlshosts control file */
		code = ehlo();
	else
	if (do_tls(&ssl, 0, smtps, smtpfd, 0, 0, partner_fqdn, host.s, host.len, tls_quit, temp_nomem, temp_control, temp_write, quit, &smtptext, &ehlokw, 0))
		code = ehlo();
#else
	if (notls) /*- if found in control/notlshosts control file */
		code = ehlo();
	else
	if (do_tls(&ssl, 0, smtps, smtpfd, 0, 0, partner_fqdn, host.s, host.len, tls_quit, temp_nomem, temp_control, temp_write, quit, &smtptext, &ehlokw, 0))
		code = ehlo();
#endif /*- #ifdef HASTLSA */
#else
	code = ehlo();
#endif /*- #ifdef TLS */
	if (!use_auth_smtp) {
		if (code >= 500) {
			is_esmtp = 0;
			if (substdio_put(&smtpto, "HELO ", 5) == -1 ||
					substdio_put(&smtpto, helohost.s, helohost.len) == -1 ||
					substdio_put(&smtpto, "\r\n", 2) == -1 ||
					substdio_flush(&smtpto) == -1)
				temp_write();
			code = smtpcode();
		}
	}
	/*- for broken servers like yahoo */
	if (env_get("TRY_NEXT_MX_HELO_FAIL") && code >= 400 && code < 500)
		return;
	if (code != 250) {
		if (!stralloc_copys(&helo_str, " but my name -->") ||
				!stralloc_cat(&helo_str, &helohost) ||
				!stralloc_cats(&helo_str, "<-- was rejected :(") ||
				!stralloc_0(&helo_str))
			temp_nomem();
		quit(code, 1, code >= 500 ? "DConnected to " : "ZConnected to ", helo_str.s, 0);
	}
	/*-
 	 * go through all lines of the multi line answer until one begins
 	 * with "XXX[ |-]SIZE", XXX[ |-]SMTPUTF8 or we reach the last line
 	 */
	if (is_esmtp) {
		i = 0;
		do {
			i += 5 + str_chr(smtptext.s + i, '\n');
			if (!use_size)
				use_size = !case_diffb(smtptext.s + i, 4, "SIZE");
			if (use_auth_smtp) {/*- check if remote supports AUTH */
				if (!auth_capa)
					auth_capa = !case_diffb(smtptext.s + i, 4, "AUTH");
			}
#ifdef SMTPUTF8
			if (enable_utf8) {
				if (!smtputf8)
					smtputf8 = enable_utf8 ? !case_diffb(smtptext.s + i, 9, "SMTPUTF8") : 0;
			}
			if (use_size && smtputf8 && auth_capa)
				break;
#else
			if (use_size && auth_capa)
				break;
#endif
		} while (smtptext.s[i - 1] == '-');
	}
#ifdef SMTPUTF8
	if (enable_utf8) {
		if (!flagutf8)
			checkutf8message();
		if (flagutf8 && !smtputf8)
			quit(553, 1, "DConnected to ", " but server does not support unicode in email addresses", 0);
	}
#endif
	if (auth_capa)
		smtp_auth(use_auth_smtp, use_size);
	else
		mailfrom(use_size);
	substdio_flush(&smtpto);
	code = smtpcode();
	if (code >= 500)
		quit(code, 1, "DConnected to ", " but sender was rejected", 0);
	if (code >= 400)
		quit(code, 1, "ZConnected to ", " but sender was rejected", 0);
	flagbother = 0;
	for (i = 0; i < smtp_reciplist.len; ++i) {
		if (substdio_put(&smtpto, "RCPT TO:<", 9) == -1 ||
				substdio_put(&smtpto, smtp_reciplist.sa[i].s, smtp_reciplist.sa[i].len) == -1 ||
				substdio_put(&smtpto, ">\r\n", 3) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		code = smtpcode();
		if (code >= 500) {
			out("hFrom: <");
			outsafe(&smtp_sender);
			out("> RCPT: <");
			outsafe(&smtp_reciplist.sa[i]);
			out("> ");
			outhost();
			out(" does not like recipient.\n");
			outsmtptext();
			zero();
		} else
		if (code >= 400) {
			out("sFrom: <");
			outsafe(&smtp_sender);
			out("> RCPT: <");
			outsafe(&smtp_reciplist.sa[i]);
			out("> ");
			outhost();
			out(" does not like recipient.\n");
			outsmtptext();
			zero();
		} else {
			out("rFrom: <");
			outsafe(&smtp_sender);
			out("> RCPT: <");
			outsafe(&smtp_reciplist.sa[i]);
			out("> ");
			zero();
			flagbother = 1;
		}
	}
	if (!flagbother)
		quit(code, 1, "DGiving up on ", "", 0);
	if (substdio_putflush(&smtpto, "DATA\r\n", 6) == -1)
		temp_write();
	code = smtpcode();
	if (code >= 500)
		quit(code, 1, "D", " failed on DATA command", code, 0);
	if (code >= 400)
		quit(code, 1, "Z", " failed on DATA command", code, 0);
#ifdef SMTPUTF8
	if (enable_utf8 && header.len &&
			substdio_put(&smtpto, header.s, header.len) == -1)
		temp_write();
#endif
	blast();
	code = smtpcode();
	flagcritical = 0;
	if (code >= 500)
		quit(code, 1, "D", " failed after I sent the message", 0);
	if (code >= 400)
		quit(code, 1, "Z", " failed after I sent the message", 0);
	quit(code, 0, "K", " accepted message - Protocol SMTP", 0);
}

static stralloc canonhost = { 0 };
static stralloc canonbox = { 0 };

void	 /*- host has to be canonical, box has to be quoted */
addrmangle(stralloc *saout, char *sender, int *flagalias, int flagcname,
		int flagquote)
{
	int             j;

	*flagalias = flagcname;
	j = str_rchr(sender, '@');
	if (!sender[j]) {
		if (!(flagquote ? quote2(saout, sender) : stralloc_copys(saout, sender)))
			temp_nomem();
		return;
	}
	if (!stralloc_copys(&canonbox, sender))
		temp_nomem();
	canonbox.len = j;
	if (!(flagquote ? quote(saout, &canonbox) : stralloc_copy(saout, &canonbox)) ||
			!stralloc_cats(saout, "@") ||
			!stralloc_copys(&canonhost, sender + j + 1))
		temp_nomem();
	if (flagcname) {
		switch (dns_cname(&canonhost))
		{
		case 0:
			*flagalias = 0;
			break;
		case DNS_MEM:
			temp_nomem();
		case DNS_SOFT:
			temp_dnscanon();
		case DNS_HARD:;	/*- alias loop, not our problem */
		}
	}
	if (!stralloc_cat(saout, &canonhost))
		temp_nomem();
}

int
prep_reciplist(saa *list,char **recips,int flagcname,int flagquote)
{
	int             flagallaliases;
	int             flagalias;

	if (!saa_readyplus(list,0))
		temp_nomem();

	flagallaliases = 1;
	while (*recips) {
		if (!saa_readyplus(list,1))
			temp_nomem();
		list->sa[list->len] = sauninit;
		addrmangle(list->sa + list->len, *recips, &flagalias, flagcname, flagquote);
		if (!flagalias)
			flagallaliases = 0;
		++list->len;
		++recips;
	}
	return flagallaliases;
}


#if defined(TLS) && defined(HASTLSA)
static int
dmatch(char *fn, stralloc *domain, stralloc *content,
	CONSTMAP *ptrmap)
{
	int             x, len;
	char           *ptr;

	if (fn) {
		switch ((x = cdb_matchaddr(fn, domain->s, domain->len)))
		{
		case 0:
		case 1:
			return (x);
		case CDB_MEM_ERR:
			temp_nomem();
		case CDB_LSEEK_ERR:
			temp_cdb("unable to lseek tlsadomains.cdb file");
		case CDB_FILE_ERR:
			temp_cdb("unable to open tlsadomains.cdb file");
		}
	} else
	if (ptrmap && constmap(ptrmap, domain->s, domain->len))
		return 1;
	if (!content)
		return (0);
	for (len = 0, ptr = content->s;len < content->len;) {
		if (!str_diff(domain->s, ptr))
			return (1);
		len += (str_len(ptr) + 1);
		ptr = content->s + len;
	}
	return (0);
}

int
is_in_tlsadomains(char *domain)
{
	static stralloc _domain = { 0 };

	if (!stralloc_copys(&_domain, domain) ||
			!stralloc_0(&_domain))
		temp_nomem();
	_domain.len--;
	switch (dmatch(tlsadomainsfn, &_domain, tlsadomains.len ? &tlsadomains : 0,
			tlsadomains.len ? &maptlsadomains : 0))
	{
	case 1:
		return (1);
	case 0:
		return (0);
	default:
		temp_control("Unable to read control file", "tlsadomains");
	}
	/*- Not reached */
	return (0);
}
#endif

void
getcontrols()
{
	int             r;
	char           *senderdomain, *ip, *x;
	static stralloc outgoingipfn;

	if (control_init() == -1)
		temp_control("Unable to initialize control files", 0);
	if (control_readint(&timeoutdata, "timeoutremote") == -1)
		temp_control("Unable to read control file", "timeoutremote");
	if (control_readint(&timeoutconn, "timeoutconnect") == -1)
		temp_control("Unable to read control file", "timeoutconnect");
	if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
		temp_control("Unable to read control file", "helohost");
#ifdef BATV
	if ((batvok = control_readline(&batvkey, (x = env_get("BATVKEY")) ? x : "batvkey")) == -1)
		temp_control("Unable to read control file", x);
	if (batvok) {
		x = (x = env_get("BATVNOSIGNREMOTE")) && *x ? x : "batvnosignremote";
		switch (control_readfile(&nosignremote, x, 0))
		{
		case -1:
			temp_control("Unable to read control file", x);
		case 0:
			if (!constmap_init(&mapnosignremote, "", 0, 1))
				temp_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosignremote, nosignremote.s, nosignremote.len, 0))
				temp_nomem();
			break;
		}
		x = (x = env_get("BATVNOSIGNLOCALS")) && *x ? x : "batvnosignlocals";
		switch (control_readfile(&nosignlocals, x, 0))
		{
		case -1:
			temp_control("Unable to read control file", x);
		case 0:
			if (!constmap_init(&mapnosignlocals, "", 0, 1))
				temp_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosignlocals, nosignlocals.s, nosignlocals.len, 0))
				temp_nomem();
			break;
		}
	}
#endif
	if ((x = env_get("OUTGOINGIP")) && *x) {
		if (!stralloc_copys(&outgoingip, x))
			temp_nomem();
		r = 1;
	} else { /*- per recipient domain outgoingip */
		if (!stralloc_copys(&outgoingipfn, "outgoingip.") ||
				!stralloc_cat(&outgoingipfn, &host) ||
				!stralloc_0(&outgoingipfn))
			temp_nomem();
		if (!(r = control_readrandom(&outgoingip, outgoingipfn.s)))
			r = control_readrandom(&outgoingip, "outgoingip");
		if (r == -1) {
			if (errno == error_nomem)
				temp_nomem();
			temp_control("Unable to read control file", "outgoingip");
		} else
		if (r && !env_put2("OUTGOINGIP", outgoingip.s))
			temp_nomem();
	}
#ifdef IPV6
	if (0 == r && !stralloc_copys(&outgoingip, "::"))
		temp_nomem();
	if (0 == str_diffn(outgoingip.s, "::", 2)) {
		int             i;
		for (i = 0; i < 16; i++)
			outip.ip6.d[i] = 0;
	} else {
		if (!ip6_scan(outgoingip.s, &outip.ip6))
			temp_noip();
		if (ip6_isv4mapped(&outip.ip6.d) && !ip4_scan(outgoingip.s, &outip.ip))
			temp_noip();
	}
#else
	if (0 == r && !stralloc_copys(&outgoingip, "0.0.0.0"))
		temp_nomem();
	if (0 == str_diffn(outgoingip.s, "0.0.0.0", 7)) {
		int             i;
		for (i = 0; i < 4; i++)
			outip.ip.d[i] = 0;
	} else
	if (!ip4_scan(outgoingip.s, &outip.ip))
		temp_noip();
#endif
	/*- domainbinding patch */
	switch (control_readfile(&localips, (x = env_get("DOMAINBINDINGS")) ? x : "domainbindings", 0))
	{
	case -1:
		temp_control("Unable to read control file", x);
	case 0:
		if (!constmap_init(&maplocalips, "", 0, 1))
			temp_nomem();
		break;
	case 1:
		if (!constmap_init(&maplocalips, localips.s, localips.len, 1))
			temp_nomem();
		break;
	}

	senderdomain = 0;
	if (smtp_sender.len) {
		int             i;

		if ((i = str_rchr(smtp_sender.s, '@')))
			senderdomain = smtp_sender.s + i + 1;
		if (!stralloc_copyb(&senderbind, senderdomain, smtp_sender.len - i - 1))
			temp_nomem();
		ip = constmap(&maplocalips, smtp_sender.s, smtp_sender.len);
		if (ip && !*ip)
			ip = 0;
		for (i = 0; !ip && i <= senderbind.len; ++i) {
			if (!i || i == senderbind.len || senderbind.s[i] == '.') {
				if ((ip = constmap(&maplocalips, senderbind.s + i, senderbind.len - i)))
					break;
			}
		}
		if (ip && !*ip)
			ip = 0;
		if (ip) {
			if (!stralloc_copys(&outgoingip, ip))
				temp_nomem();
#ifdef IPV6
			if (!ip6_scan(ip, &outip.ip6))
				temp_noip();
			if (ip6_isv4mapped(&outip.ip6.d) && !ip4_scan(ip, &outip.ip))
				temp_noip();
#else
			if (!ip4_scan(ip, &outip.ip))
				temp_noip();
#endif
			helohost = senderbind;
		}
	}

	/*- ---------------- END DOMAIN BINDING PATCH */
	/*- helohostbyip */
	switch (control_readfile(&helohosts, (x = env_get("HELOHOSTBYIP")) ? x : "helohostbyip", 0))
	{
	case -1:
		temp_control("Unable to read control file", x);
	case 0:
		if (!constmap_init(&maphelohosts, "", 0, 1))
			temp_nomem();
		break;
	case 1:
		if (!constmap_init(&maphelohosts, helohosts.s, helohosts.len, 1))
			temp_nomem();
		break;
	}
	x = constmap(&maphelohosts, outgoingip.s, outgoingip.len);
	if (x && *x && !stralloc_copys(&helohost, x))
		temp_nomem();
#ifdef TLS
	switch (control_readfile(&notlshosts, (x = env_get("NOTLSHOSTS")) ? x : "notlshosts", 0))
	{
	case -1:
		temp_control("Unable to read control file", x);
	case 0:
		if (!constmap_init(&mapnotlshosts, "", 0, 0))
			temp_nomem();
		break;
	case 1:
		if (!constmap_init(&mapnotlshosts, notlshosts.s, notlshosts.len, 0))
			temp_nomem();
		break;
	}
	x = constmap(&mapnotlshosts, host.s, host.len);
	if (x && *x)
		notls = 1;
#ifdef HASTLSA
	/*- tlsadomains */
	if (!(tlsadomainsfn = env_get("TLSADOMAINS")))
		tlsadomainsfn = "tlsadomains";
	switch (control_readfile(&tlsadomains, tlsadomainsfn, 0))
	{
	case -1:
		temp_control("Unable to read control file", x);
	case 1:
		if (!constmap_init(&maptlsadomains, tlsadomains.s, tlsadomains.len, 0))
			temp_nomem();
		break;
	}
#endif
#endif
}

#if BATV
static stralloc newsender = { 0 };
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
no_return void
temp_batv(char *arg)
{
	out("Zerror creating batv signature. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, arg) ||
			!stralloc_catb(&smtptext, ". (#4.3.0)", 10))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}
#endif

void
sign_batv()
{
	int             daynumber = (now() / 86400) % 1000;
	int             i;
	char            kdate[] = "0000";
	static char     hex[] = "0123456789abcdef";
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_MD_CTX     *mdctx;
	const EVP_MD   *md = NULL;
	unsigned char   md5digest[EVP_MAX_MD_SIZE];
	unsigned int    md_len;
#else
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
#endif

	if (stralloc_starts(&smtp_sender, "prvs="))
		return;	/*- already signed */
	if (stralloc_starts(&smtp_sender, "sb*-")) {	/*- don't sign this */
		smtp_sender.len -= 4;
		byte_copy(smtp_sender.s, smtp_sender.len, smtp_sender.s + 4);
		return;
	}
	if (!stralloc_ready(&newsender, smtp_sender.len + (2 * BATVLEN + 10)) ||
			!stralloc_copyb(&newsender, "prvs=", 5))
		temp_nomem();
	/*- only one key so far */
	kdate[1] = '0' + daynumber / 100;
	kdate[2] = '0' + (daynumber / 10) % 10;
	kdate[3] = '0' + daynumber % 10;
	if (!stralloc_catb(&newsender, kdate, 4))
		temp_nomem();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!(md = EVP_MD_fetch(NULL, "md5", NULL)))
		temp_batv("batv: unable to fetch digest implementation for MD5");
	if (!(mdctx = EVP_MD_CTX_new()))
		temp_nomem();
	if (!EVP_DigestInit_ex(mdctx, md, NULL) ||
			!EVP_DigestUpdate(mdctx, kdate, 4) ||/*- date */
			!EVP_DigestUpdate(mdctx, smtp_sender.s, smtp_sender.len) ||
			!EVP_DigestUpdate(mdctx, batvkey.s, batvkey.len) ||
			!EVP_DigestFinal_ex(mdctx, md5digest, &md_len))
		temp_batv("batv: unable to hash md5 message digest");
	EVP_MD_free(md);
#else
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, smtp_sender.s, smtp_sender.len);
	MD5_Update(&md5, batvkey.s, batvkey.len);
	MD5_Final(md5digest, &md5);
#endif
	for (i = 0; i < BATVLEN; i++) {
		char            md5hex[2];

		md5hex[0] = hex[md5digest[i] >> 4];
		md5hex[1] = hex[md5digest[i] & 15];
		if (!stralloc_catb(&newsender, md5hex, 2))
			temp_nomem();
	}
	/*-	separator */
	if (!stralloc_catb(&newsender, "=", 1) ||
			!stralloc_cat(&newsender, &smtp_sender) ||
			!stralloc_copy(&smtp_sender, &newsender))
		temp_nomem();
}
#endif

/*
 * Original by Richard Lyons
 * Case insensitivity by Ted Fines
 * http://www.apecity.com/qmail/moresmtproutes.txt
 */
char           *
moresmtproutes_lookup(char *hst, int len)
{
	static stralloc morerelayhost = { 0 };
	static stralloc h = { 0 };
	uint32          dlen;

	if (!stralloc_copyb(&h, hst, len))
		temp_nomem();
	case_lowerb(h.s, h.len);
	if (fdmoreroutes != -1 && cdb_seek(fdmoreroutes, h.s, h.len, &dlen) == 1) {
		if (!stralloc_ready(&morerelayhost, (unsigned int) (dlen + 1)))
			temp_nomem();
		morerelayhost.len = dlen;
		if (cdb_bread(fdmoreroutes, morerelayhost.s, morerelayhost.len) == -1)
			temp_control("Unable to read control file", "moresmtproutes.cdb");
		if (!stralloc_0(&morerelayhost))
			temp_nomem();
		return morerelayhost.s;
	}
	return 0;
}

int
ipme_is46(struct ip_mx *mxip)
{
	switch (mxip->af)
	{
	case AF_INET:
		return ipme_is(&mxip->addr.ip);
#ifdef IPV6
	case AF_INET6:
		return ipme_is6(&mxip->addr.ip6);
#endif
	}
	return 0;
}

int
timeoutconn46(int fd, struct ip_mx *ix, union v46addr *ip, int port_num, int tmout)
{
	switch (ix->af)
	{
#ifdef IPV6
	case AF_INET6:
		return timeoutconn6(fd, &ix->addr.ip6, ip, port_num, tmout);
		break;
#endif
	case AF_INET:
		return timeoutconn4(fd, &ix->addr.ip, ip, port_num, tmout);
		break;
	default:
		return timeoutconn4(fd, &ix->addr.ip, ip, port_num, tmout);
	}
}

void
password_lookup(char *addr, int addr_len)
{
	int             i, j;
	char           *result, *ptr;

	switch (cdb_match("remote_auth.cdb", addr, addr_len, &result))
	{
	case CDB_FOUND:
		if (result) {
			i = str_chr(result, ' ');
			if (result[i]) {
				result[i] = '\0';
				for (ptr = result + i + 1; isspace(*ptr); ptr++, i++);
				j = str_chr(result + i + 1, ' ');
				if (result[i + j + 1]) /* envstr present */
					result[i + j + 1] = 0;
				if (!stralloc_copys(&user, result) ||
						!stralloc_0(&user) ||
						!stralloc_copys(&pass, result + i + 1) ||
						!stralloc_0(&pass))
					temp_nomem();
				user.len--;
				pass.len--;
				for (ptr = result + i + j + 2; isspace(*ptr); ptr++, j++);
				if (result[i + j + 2])
					parse_env(result + i + j + 2);
			} else
				use_auth_smtp = 0;
		} else
			use_auth_smtp = 0;
		break;
	case CDB_MEM_ERR:
		temp_nomem();
	case CDB_READ_ERR:
		temp_cdb("unable to read remote_auth.cdb file");
	case CDB_LSEEK_ERR:
		temp_cdb("unable to lseek remote_auth.cdb file");
	case CDB_FILE_ERR:
		temp_cdb("unable to open remote_auth.cdb file");
	default:
		use_auth_smtp = 0;
		break;
	}
	return;
}

char           *
get_relayhost(char **recips)
{
	int             i, j, k, cntrl_stat1, cntrl_stat2;
	char           *relayhost, *x, *routes, *smtproutefile,
				   *moresmtproutefile, *qmtproutefile;
	static stralloc controlfile;

	/* QMTP */
	if ((routes = env_get("QMTPROUTE"))) {	/*- MySQL or manually set env variable */
		if (!stralloc_copyb(&qmtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat2 = 2;
	} else {
		qmtproutefile = (qmtproutefile = env_get("QMTPROUTESFILE")) ? qmtproutefile : "qmtproutes";
		cntrl_stat2 = control_readfile(&qmtproutes, qmtproutefile, 0);
	}
	switch (cntrl_stat2)
	{
	case -1:/*- error reading qmtproutes */
		temp_control("Unable to read control file", "qmtproutes");
	case 0:/*- qmtproutes absent */
		if (!constmap_init(&mapqmtproutes, "", 0, 1))
			temp_nomem();
		break;
	case 1:/*- qmtproutes present */
	case 2:/*- QMTPROUTE env variable present */
		if (!constmap_init(&mapqmtproutes, qmtproutes.s, qmtproutes.len, 1))
			temp_nomem();
		break;
	}

	/* SMTP */
	routes = (routes = env_get("SMTPROUTE")) ? routes : env_get("X-SMTPROUTES");
	if (routes) {	/*- MySQL smtp_port, manually set env variable, X-SMTPROUTES from header */
		if (!stralloc_copyb(&smtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat1 = 2;
	} else {
		smtproutefile = (smtproutefile = env_get("SMTPROUTESFILE")) ? smtproutefile : "smtproutes";
		moresmtproutefile = (moresmtproutefile = env_get("MORESMTPROUTESCDB")) ? moresmtproutefile : "moresmtproutes.cdb";
		cntrl_stat1 = control_readfile(&smtproutes, smtproutefile, 0);

		/*- open moresmtproutes.cdb */
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				!stralloc_cats(&controlfile, moresmtproutefile) ||
				!stralloc_0(&controlfile))
			temp_nomem();
		else
		if ((fdmoreroutes = open_read(controlfile.s)) == -1) {
			if (errno != error_noent)
				cntrl_stat1 = -1;
		}
	}
	switch (cntrl_stat1)
	{
	case -1:/*- error reading smtproutes */
		temp_control("Unable to read control file", "smtproutes");
	case 0:/*- smtproutes absent */
		if (!constmap_init(&mapsmtproutes, "", 0, 1))
			temp_nomem();
		break;
	case 1:/*- smtproutes present */
	case 2:/*- SMTPROUTE env variable present */
		if (!constmap_init(&mapsmtproutes, smtproutes.s, smtproutes.len, 1))
			temp_nomem();
		break;
	}

	/*- Per user SMTPROUTE functionality using moresmtproutes.cdb */
	relayhost = moresmtproutes_lookup(*recips, str_len(*recips));
	if (smtp_sender.len == 0) { /*- bounce routes */
		if (!stralloc_copys(&bounce, "!@"))
			temp_nomem();
		if ((relayhost = constmap(&mapqmtproutes, bounce.s, bounce.len))) {
			protocol_t = 'q';
			port = PORT_QMTP;
		} else {
			if (!(relayhost = constmap(&mapsmtproutes, bounce.s, bounce.len)))
				relayhost = moresmtproutes_lookup("!@", 2);
			if (relayhost) {
				protocol_t = 's';
				port = PORT_SMTP;
			}
		}
	}
	if (relayhost && !*relayhost)
		relayhost = NULL;
	if (cntrl_stat1 || cntrl_stat2) {
		/*- Look at qmtproutes/smtproutes */
		for (i = 0; !relayhost && i <= host.len; ++i) {
			if ((i == 0) || (i == host.len) || (host.s[i] == '.')) {
				/*- default qmtproutes */
				if (cntrl_stat2 == 2 && (relayhost = constmap(&mapqmtproutes, host.s + i, host.len - i))) {
					protocol_t = 'q';
					port = PORT_QMTP;
					break;
				} else
				if (cntrl_stat1 == 2 && (relayhost = constmap(&mapsmtproutes, host.s + i, host.len - i))) {
					port = PORT_SMTP;
					break;
				} else
				if (cntrl_stat2 && (relayhost = constmap(&mapqmtproutes, host.s + i, host.len - i))) {
					protocol_t = 'q';
					port = PORT_QMTP;
					break;
				} else
				if (cntrl_stat1 && (relayhost = constmap(&mapsmtproutes, host.s + i, host.len - i))) {
					port = PORT_SMTP;
					break;
				} else
				if ((relayhost = moresmtproutes_lookup(host.s + i, host.len - i)))
					break;
			}
		}
		if (relayhost && !*relayhost)
			relayhost = NULL;
	}

	if (relayhost) {
		if (use_auth_smtp) {
			/*-
			 * adapted from a patch by Jay Soffian
			 * domain:relay username password envstr
			 *  or
			 * domain:relay:port username password envstr
			 *  or
			 * domain:relay:port:penalty:max_tolerance username password envstr
			 */
			i = str_chr(relayhost, ' ');
			if (relayhost[i]) {
				if (relayhost[i + 1] == '/') { /*- get password from remote_auth.cdb */
					if (!str_diff(relayhost + i + 1, "/s")) /*- match on sender */
						password_lookup(smtp_sender.s, smtp_sender.len);
					else
					if (!str_diff(relayhost + i + 1, "/r")) /*- match on recipient */
						password_lookup(*recips, str_len(*recips));
					else /* match on address specifed in smtproute */
						password_lookup(relayhost + i + 2, str_len(relayhost + i + 2));
				} else {
					relayhost[i] = '\0';
					j = str_chr(relayhost + i + 1, ' ');
					if (relayhost[i + j + 1]) { /*- if password is present */
						relayhost[i + j + 1] = '\0';
						if (relayhost[i + 1] && relayhost[i + j + 2]) {	/*- both user and password are present */
							k = str_chr(relayhost + i + j + 2, ' ');
							if (relayhost[i + j + 2 + k]) {
								relayhost[i + j + 2 + k] = '\0';
								if (relayhost[i + j + k + 3]) /*- envstr */
									parse_env(relayhost + i + j + k + 3);
							}
							if (!stralloc_copys(&user, relayhost + i + 1) ||
									!stralloc_0(&user) ||
									!stralloc_copys(&pass, relayhost + i + j + 2) ||
									!stralloc_0(&pass))
								temp_nomem();
							user.len--;
							pass.len--;
						}
					} else
						use_auth_smtp = 0;
				}
			} else
				use_auth_smtp = 0;
		} /*- if (use_auth_smtp) */
		/*-
		 * domain:relay
		 *  or
		 * domain:relay:port
		 *  or
		 * domain:relay:port:penalty:max_tolerance
		 */
		i = str_chr(relayhost, ':');
		if (relayhost[i]) {
			if (relayhost[i + 1]) {
				if (relayhost[i + 1] != ':') /*- port is present */
					scan_ulong(relayhost + i + 1, &port);
				relayhost[i] = '\0';
				x = relayhost + i + 1; /*- port */
				i = str_chr(x, ':');   /*- : before min_penalty */
				if (x[i]) {
					if (x[i + 1] != ':') /*- if penalty figure is present */
						scan_int(x + i + 1, &min_penalty);
					x = relayhost + i + 1; /*- min_penalty */
					i = str_chr(x, ':');
					if (x[i]) {
						if (x[i + 1] != ':') /*- if tolerance figure is present */
							scan_ulong(x + i + 1, &max_tolerance);
					}
					if (!min_penalty)
						flagtcpto = 0;
				}
			} else
				relayhost[i] = '\0';
		}
		switch (port)
		{
		case 587:
			protocol_t = 's';
			break;
		case 465:
			protocol_t = 'S';
			break;
		case 6209:
			protocol_t = 'Q';
			break;
		}
		if (!stralloc_copys(&host, relayhost))
			temp_nomem();
	} else {
#ifdef SMTPUTF8
		char           *asciihost;
#endif

		use_auth_smtp = 0;
#ifdef SMTPUTF8
		if (smtputf8) {
			if (!stralloc_0(&host))
				temp_nomem();
			host.len--;
			switch (idn2_lookup_u8((const uint8_t *) host.s, (uint8_t **) &asciihost, IDN2_NFC_INPUT))
			{
			case IDN2_OK:
				break;
			case IDN2_MALLOC:
				temp_nomem();
			default:
				perm_dns();
			}
			if (!stralloc_copys(&idnhost, asciihost) || !stralloc_0(&idnhost))
				temp_nomem();
			idnhost.len--;
		}
#endif
	}
	return relayhost;
}

/*
 * argv[0] - qmail-remote
 * argv[1] - Recipient SMTP Domain/host
 * argv[2] - Sender
 * argv[3] - qqeh
 * argv[4] - Size
 * argv + 5 - Recipient list
 */
int
main(int argc, char **argv)
{
	static ipalloc  ip = { 0 };
	int             i, j, errors, flagallaliases, flagalias, smtp_error;
	unsigned long   random, prefme;
	char          **recips;
	char           *relayhost, *x;
#if defined(TLS) && defined(HASTLSA)
	int             e;
	char            rbuf[2];
#endif

	sig_pipeignore();
	if (argc < 6)
		perm_usage();
	my_argc = argc;
	my_argv = argv;
	if (!stralloc_copys(&host, argv[1]))	 /*- host required by getcontrols below */
		temp_nomem();
	addrmangle(&qmtp_sender, argv[2], &flagalias, 0, 0);
	addrmangle(&smtp_sender, argv[2], &flagalias, 0, 1);
	if (!stralloc_copys(&qqeh, argv[3]))
		temp_nomem();
	msgsize = argv[4];
	recips = argv + 5;
	getcontrols();
	dns_init(0);
	if (env_get("SMTPS"))
		smtps = 1;
	protocol_t = smtps ? 'S' : 's';
	use_auth_smtp = env_get("AUTH_SMTP");
	relayhost = get_relayhost(recips);
	use_auth_smtp = env_get("AUTH_SMTP");
#ifdef SMTPUTF8
	enable_utf8 = env_get("SMTPUTF8");
#endif
	min_penalty = (x = env_get("MIN_PENALTY")) ? scan_int(x, &min_penalty) : MIN_PENALTY;
	max_tolerance = (x = env_get("MAX_TOLERANCE")) ? scan_ulong(x, &max_tolerance) : MAX_TOLERANCE;
#if BATV
	if (batvok && smtp_sender.len && batvkey.len) {
		if (!stralloc_0(&smtp_sender))
			temp_nomem();		/*- null terminate */
		smtp_sender.len--;
		i = str_rchr(*recips, '@');	/*- should check all recips, not just the first */
		j = str_rchr(smtp_sender.s, '@');
		if (!constmap(&mapnosignremote, *recips + i + 1, str_len(*recips + i + 1))
			&& !constmap(&mapnosignlocals, smtp_sender.s + j + 1, smtp_sender.len - (j + 1)))
			sign_batv(); /*- modifies sender */
	}
#endif
	if (ipme_init() != 1)
		temp_oserr();
	i = (env_get("DISABLE_CNAME_LOOKUP") ? 0 : !relayhost);
	flagallaliases = prep_reciplist(&qmtp_reciplist, recips, i, 0);
	flagallaliases = prep_reciplist(&smtp_reciplist, recips, i, 1);
	random = now() + (getpid() << 16);
#ifdef SMTPUTF8
	i = relayhost ? dns_ip(&ip, &host) : dns_mxip(&ip, smtputf8 ? &idnhost : &host, random);
#else
	i = relayhost ? dns_ip(&ip, &host) : dns_mxip(&ip, &host, random);
#endif
	switch (i)
	{
	case DNS_MEM:
		temp_nomem();
	case DNS_SOFT:
		temp_dns();
	case DNS_HARD:
		perm_dns();
	case 1:
		if (ip.len <= 0)
			temp_dns();
	}
	if (ip.len <= 0)
		perm_nomx();
	prefme = 100000;
	for (i = 0; i < ip.len; ++i) {
		if (ipme_is46(&ip.ix[i])) {
			if (ip.ix[i].pref < prefme)
				prefme = ip.ix[i].pref;
		}
	}
	if (relayhost)
		prefme = 300000;
	if (flagallaliases)
		prefme = 500000;
	for (i = 0; i < ip.len; ++i) {
		if (ip.ix[i].pref < prefme)
			break;
	}
	if (i >= ip.len)
		perm_ambigmx();
	x = 0;
#if defined(TLS) && defined(HASTLSA)
	if (!relayhost && notls == 0)
		do_tlsa = env_get("DANE_VERIFICATION");
#endif
#ifdef MXPS
	mxps = env_get("DISABLE_MXPS") ? 0 : 2;
#endif
	for (i = j = 0; i < ip.len; ++i) {
#ifdef MXPS
		if (!mxps && qmtp_priority(ip.ix[i].pref))
			mxps = 1;
		if (protocol_t == 'q' || mxps == 1 || ip.ix[i].pref < prefme)
#else
		if (protocol_t == 'q' || ip.ix[i].pref < prefme)
#endif
		{
			if (flagtcpto && tcpto(&ip.ix[i], min_penalty))
				continue;
#if defined(TLS) && defined(HASTLSA)
			use_daned = 0;
			if (do_tlsa) {
				if (tlsadomains.len && !is_in_tlsadomains(ip.ix[i].fqdn))
					do_tlsa = (char *) 0;
				else {
					for (x = do_tlsa; *x; x++)
						if (*x == '@')
							break;
					if (*x == '@') {
						/*- connect to qmail-daned */
						e = tlsacheck(do_tlsa, ip.ix[i].fqdn, QUERY_MODE, rbuf, timeoutfn, err_tmpfail);
						if (e && rbuf[1] == RECORD_OK)
							do_tlsa = (char *) 0;
						else
						if (e && rbuf[1] == RECORD_NOVRFY)
							quit(534, 1, "DConnected to ", " but recpient failed DANE validation", 0);
						else
						if (!e)
							use_daned = 1; /*- do inbuilt DANE Verification and update qmail-daned */
					}
				}
			}
#endif
#ifdef IPV6
			if ((smtpfd = (ip.ix[i].af == AF_INET ? socket_tcp4() : socket_tcp6())) == -1)
#else
			if ((smtpfd = socket_tcp4()) == -1)
#endif
				temp_oserr();
			if (!x) {
				if (!(x = alloc(IPFMT + 1)))
					temp_nomem();
			} else
			if (!alloc_re((char *) &x, j, j + IPFMT + 1))
				temp_nomem();
			j += ip4_fmt(x + j, &ip.ix[i].addr.ip);
			x[j++] = ',';
			x[j] = 0;
#if defined(TLS) && defined(HASTLSA)
			if (!relayhost && do_tlsa) {
				switch (get_tlsa_rr(ip.ix[i].fqdn, port))
				{
				case DNS_HARD:
					do_tlsa = (char *) 0;
					break;
				case DNS_SOFT:
					temp_dns_rr();
				case DNS_MEM:
					temp_nomem();
				}
			}
#endif
			for (;;) {
				if (!timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconn)) {
					if (flagtcpto)	   /*- clear the error */
						tcpto_err(&ip.ix[i], 0, max_tolerance);
					partner = ip.ix[i];
#ifdef TLS
					partner_fqdn = ip.ix[i].fqdn;
#else
					partner_fqdn = NULL;
#endif
#ifdef MXPS
					if (mxps != -1 && (protocol_t == 'q' || mxps == 1))
#else
					if (protocol_t == 'q')
#endif
					{
						if (j)
							x[j - 1] = 0;
						qmtp(&host, x, port); /*- does not return */
					} else
						do_smtp(partner_fqdn);	/*- only returns when the next MX is to be tried */
					smtp_error = 1;
					break;
				} else { /*- connect failed */

#ifdef MXPS
					if (mxps == 1) { /*- QMTP failed; try SMTP */
						mxps = -1;
						port = PORT_SMTP;
						continue;
					}
#endif
					smtp_error = 0;
					break;
				}
			} /*- for (;;) */
			/*-
			 * Add network errors preventing smtp connections.
			 * Add the IP address to tcp timeout table
			 * and try the next MX
			 */
			if (flagtcpto) {
				errors = (errno == error_timeout || errno == error_connrefused || errno == error_hostdown
						  || errno == error_netunreach || errno == error_hostunreach || smtp_error);
				tcpto_err(&ip.ix[i], errors, max_tolerance);
			}
			smtpfrom.p = 0; /*- clear anything in buffer if any - Franz Sirl */
			close(smtpfd);
		}
	} /*- for (i = j = 0; i < ip.len; ++i) */
	if (j) {
		x[j - 1] = 0; /*- remove the last ',' */
		temp_noconn(&host, x, port);
	} else
		temp_noconn(&host, 0, 0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_remote_c()
{
	static char    *x = "$Id: qmail-remote.c,v 1.165 2023-03-10 13:12:05+05:30 Cprogrammer Exp mbhangui $";
	x = sccsidqrdigestmd5h;
	x++;
}

/*
 * $Log: qmail-remote.c,v $
 * Revision 1.165  2023-03-10 13:12:05+05:30  Cprogrammer
 * skip smtp_auth function if remote doesn't support authenticated smtp
 *
 * Revision 1.164  2023-01-15 12:32:37+05:30  Cprogrammer
 * Use env variable SMPTS to immediately start in encrypted
 * quit() function changed to have varargs
 * handle auth smtp error codes using decode_smtpauth_err() as per RFC4954
 *
 * Revision 1.163  2023-01-13 22:15:04+05:30  Cprogrammer
 * fixed bug with SMTP AUTH PLAIN
 *
 * Revision 1.162  2023-01-13 12:12:33+05:30  Cprogrammer
 * moved setting relayhosts variable to get_relayhosts()
 * added feature to set env variables from [q,s]mtproutes, remote_auth.cdb
 *
 * Revision 1.161  2023-01-06 17:36:43+05:30  Cprogrammer
 * changed scope of global variables to static
 * moved tls/ssl functions to dossl.c
 *
 * Revision 1.160  2023-01-03 17:01:02+05:30  Cprogrammer
 * set default certificate dir to /etc/indimail/certs
 * use set_tls_method() from libqmail
 *
 * Revision 1.159  2022-11-08 23:32:53+05:30  Cprogrammer
 * clear input buffer when trying next mx
 *
 * Revision 1.158  2022-10-30 22:16:52+05:30  Cprogrammer
 * fetch username password for authenticated smtp from remote_auth.cdb
 *
 * Revision 1.157  2022-10-13 20:32:50+05:30  Cprogrammer
 * use batv prefix for batv control files
 * allow batv parameters to be set via env variables
 *
 * Revision 1.156  2022-09-07 20:18:53+05:30  Cprogrammer
 * fixed compilation on systems without TLS1_3_VERSION
 *
 * Revision 1.155  2022-08-24 08:03:54+05:30  Cprogrammer
 * fixed non-SCRAM methods getting skipped
 *
 * Revision 1.154  2022-08-23 20:01:29+05:30  Cprogrammer
 * use scram salted password instead of plaintext for SCRAM when SALTED_PASSWORD environment variable is set
 *
 * Revision 1.153  2022-08-23 00:05:38+05:30  Cprogrammer
 * added channel binding for SCRAM-*-PLUS methods
 *
 * Revision 1.152  2022-08-22 22:23:34+05:30  Cprogrammer
 * added check for return value of subsdio_put
 *
 * Revision 1.151  2022-08-21 19:39:22+05:30  Cprogrammer
 * fix compilation error when TLS is not defined in conf-tls
 * replace hard coded auth methods with defines in authmethods.h
 * added CRAM-SHA224, CRAM-SHA384, CRAM-SHA512 AUTH methods
 *
 * Revision 1.150  2022-05-18 13:30:05+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.149  2021-08-19 19:55:42+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.148  2021-08-13 15:12:46+05:30  Cprogrammer
 * fixed get3()
 *
 * Revision 1.147  2021-08-11 21:45:53+05:30  Cprogrammer
 * disable MXPS using DISABLE_MXPS
 * disable quoting (required for SMTP) when using MXPS
 *
 * Revision 1.146  2021-06-12 18:42:03+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.145  2021-06-10 10:54:26+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.144  2021-05-26 10:46:02+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.143  2021-01-27 13:21:47+05:30  Cprogrammer
 * corrected SMTP code for unicode error
 *
 * Revision 1.142  2021-01-23 21:25:17+05:30  Cprogrammer
 * added comments for smtputf8 variables
 *
 * Revision 1.141  2021-01-23 08:14:37+05:30  Cprogrammer
 * renamed env variale UTF8 to SMTPUTF8
 *
 * Revision 1.140  2020-12-07 12:05:49+05:30  Cprogrammer
 * renamed utf8message to flagutf8, firstpart to header
 *
 * Revision 1.139  2020-12-03 23:11:54+05:30  Cprogrammer
 * quote AUTH address
 *
 * Revision 1.138  2020-12-03 17:29:29+05:30  Cprogrammer
 * added EAI - RFC 6530-32 - unicode address support
 *
 * Revision 1.137  2020-11-29 10:13:00+05:30  Cprogrammer
 * use get1(), get3() functions to read smtp code
 *
 * Revision 1.136  2020-11-24 13:47:20+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.135  2020-11-22 23:12:06+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.134  2020-10-10 21:20:53+05:30  Cprogrammer
 * renamed digest_md5() to qr_digest_md5() to avoid clash with digest_md5()
 *
 * Revision 1.133  2020-05-11 11:11:24+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.132  2020-05-10 17:47:03+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.131  2019-07-09 10:24:29+05:30  Cprogrammer
 * speed up blast() function by reading in chunks of 4096 bytes
 *
 * Revision 1.130  2018-06-27 19:54:17+05:30  Cprogrammer
 * use TLSA only if TLS is defined
 *
 * Revision 1.129  2018-06-27 18:11:39+05:30  Cprogrammer
 * added control file control/notlshosts
 * added additonal check for notlshosts/host
 *
 * Revision 1.128  2018-06-13 08:52:25+05:30  Cprogrammer
 * made client cert configurable via env variable CLIENTCERT
 *
 * Revision 1.127  2018-06-12 07:15:43+05:30  Cprogrammer
 * initialize ssl to null after SSL_free()
 *
 * Revision 1.126  2018-05-31 17:13:28+05:30  Cprogrammer
 * fixed potential use of uninitialized variable in do_pkix()
 *
 * Revision 1.125  2018-05-30 20:04:10+05:30  Cprogrammer
 * using hexstring variable inside tlsa_vrfy_records() clobbered certDataField
 *
 * Revision 1.124  2018-05-30 19:17:47+05:30  Cprogrammer
 * use constants from tlsacheck.h
 *
 * Revision 1.123  2018-05-28 21:44:22+05:30  Cprogrammer
 * fixed code for openssl version < 0x10100000L
 *
 * Revision 1.122  2018-05-28 17:22:16+05:30  Cprogrammer
 * fixed compiler warning with unused variable when HASTLSA is not defined
 *
 * Revision 1.121  2018-05-27 23:38:52+05:30  Cprogrammer
 * added tlsadomains control file to restrict DANE verification to hosts in tlsadomains control file
 *
 * Revision 1.120  2018-05-27 22:18:01+05:30  mbhangui
 * added DANE verication using qmail-daned
 *
 * Revision 1.119  2018-05-27 17:49:02+05:30  Cprogrammer
 * refactored code for TLSA
 *
 * Revision 1.118  2018-05-27 11:18:20+05:30  Cprogrammer
 * check substdio_put() for error
 *
 * Revision 1.117  2018-05-26 15:59:35+05:30  Cprogrammer
 * replaced getdns lib with inbuilt dns_tlsarr() function in dns.c
 *
 * Revision 1.116  2018-05-25 08:44:14+05:30  Cprogrammer
 * removed extra white spaces
 *
 * Revision 1.115  2018-05-25 08:41:09+05:30  Cprogrammer
 * renamed do_dns_query() to do_tlsa_query()
 *
 * Revision 1.114  2018-05-25 08:34:01+05:30  Cprogrammer
 * removed getdns
 *
 * Revision 1.113  2018-05-23 20:15:52+05:30  Cprogrammer
 * added dane verification code
 *
 * Revision 1.112  2018-05-01 01:42:57+05:30  Cprogrammer
 * indented code
 *
 * Revision 1.111  2017-08-26 11:07:08+05:30  Cprogrammer
 * fixed readling of tlsclientmethod control file
 *
 * Revision 1.110  2017-08-24 13:20:24+05:30  Cprogrammer
 * improved logging of TLS method errors
 *
 * Revision 1.109  2017-08-23 13:13:07+05:30  Cprogrammer
 * replaced SSLv23_client_method() with TLS_client_method()
 *
 * Revision 1.108  2017-08-08 23:56:27+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.107  2017-05-15 23:21:07+05:30  Cprogrammer
 * fix for SMTPROUTE of the form domain:x.x.x.x::penalty:max_tolerance username password
 *
 * Revision 1.106  2017-05-15 19:18:53+05:30  Cprogrammer
 * use environment variable SMTPROUTESFILE, QMTPROUTESFILE, MORESMTPROUTESCDB to configure smtproutes, qmtproutes, moresmtproutes.cdb filenames
 *
 * Revision 1.105  2017-05-15 15:33:50+05:30  Cprogrammer
 * use X-SMTPROUTES env variable for setting artificial smtp routes. SMTPROUTES takes precendence over X-SMTPROUTES
 * bugfix - set port to 25 if relayhost is of the form :relayhost:
 *
 * Revision 1.104  2017-05-08 13:20:21+05:30  Cprogrammer
 * check for tlsv1_1_client_method() and tlsv1_2_client_method()
 *
 * Revision 1.103  2017-05-02 16:39:39+05:30  Cprogrammer
 * added SSL_CTX_free()
 *
 * Revision 1.102  2017-05-02 10:21:48+05:30  Cprogrammer
 * fixed location of tlsclientmethod control file
 *
 * Revision 1.101  2017-04-10 20:44:39+05:30  Cprogrammer
 * use SMTPS if SMTPS environment variable is set
 * added documentation and better variable name for type
 *
 * Revision 1.100  2017-04-06 15:58:00+05:30  Cprogrammer
 * new tls_quit() function to avoid mixed usage of tls_quit(), quit() functions
 *
 * Revision 1.99  2017-03-31 15:27:37+05:30  Cprogrammer
 * smtptext "Sorry I couldn't find any host named" was getting overwritten
 *
 * Revision 1.98  2017-03-31 10:22:46+05:30  Cprogrammer
 * added SMTPSTATUS env variable for success
 *
 * Revision 1.97  2017-03-24 15:34:35+05:30  Cprogrammer
 * fixes for onsuccess, onfailure scripts
 *
 * Revision 1.96  2017-03-21 15:39:33+05:30  Cprogrammer
 * use CERTDIR to override tls/ssl certificates
 *
 * Revision 1.95  2017-03-10 17:58:59+05:30  Cprogrammer
 * added back SSLv23 method
 *
 * Revision 1.94  2017-03-10 11:29:37+05:30  Cprogrammer
 * made TLS client method configurable using control file tlsclientmethod
 *
 * Revision 1.93  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.92  2015-08-24 19:08:34+05:30  Cprogrammer
 * replaced ip_scan() with ip4_scan(), replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.91  2014-12-18 11:18:41+05:30  Cprogrammer
 * added helohostbyip
 *
 * Revision 1.90  2014-04-03 21:35:39+05:30  Cprogrammer
 * added ability to disable specific AUTH methods - PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-SHA256
 * CRAM-RIPEMD, DIGEST-MD5
 * use secure authentication methods if SECURE_AUTH env variable is defined
 *
 * Revision 1.89  2014-03-18 14:39:33+05:30  Cprogrammer
 * set environment variable OUTGOINGIP
 *
 * Revision 1.88  2013-09-06 13:53:23+05:30  Cprogrammer
 * set SMTPHOST env variable for ONSUCCESS, ONFAILURE scripts
 * try next mx on helo temp failure
 *
 * Revision 1.87  2013-08-27 08:09:12+05:30  Cprogrammer
 * set SMTPTEXT for local failures
 *
 * Revision 1.86  2013-08-22 11:10:14+05:30  Cprogrammer
 * set env variable SMTPSTATUS as 'D' or 'Z' indicating permanent or temporary failure
 *
 * Revision 1.85  2013-08-21 18:59:06+05:30  Cprogrammer
 * fixed setting of SMTPTEXT env variable for success and failures
 * bypass tls if clientcert.pem is missing
 *
 * Revision 1.84  2012-12-17 15:34:37+05:30  Cprogrammer
 * fix for smtptext during TLS negotiation
 *
 * Revision 1.83  2012-11-25 07:57:25+05:30  Cprogrammer
 * set smtptext for qmtp
 *
 * Revision 1.82  2012-11-24 11:06:04+05:30  Cprogrammer
 * fixed typo (SMTPCODE instead of SMTPTEXT)
 *
 * Revision 1.81  2012-11-24 07:50:45+05:30  Cprogrammer
 * set [S|Q]MTPTEXT and [S|Q]MTPCODE for transient errors
 * unset ONSUCCESS_REMOTE, ONFAILURE_REMOTE, ONTRANSIENT_REMOTE selectively
 * fix SIGSEGV setting SMTPTEXT environment variable
 *
 * Revision 1.80  2012-10-09 18:09:45+05:30  Cprogrammer
 * added DISABLE_CNAME_LOOKUP to bypass dns cname resolution
 *
 * Revision 1.79  2011-12-10 15:24:00+05:30  Cprogrammer
 * added hmac_sha256() function
 *
 * Revision 1.78  2011-12-08 20:32:06+05:30  Cprogrammer
 * fixed compilation error
 *
 * Revision 1.77  2011-12-08 14:48:00+05:30  Cprogrammer
 * added version info for all sub sources
 *
 * Revision 1.76  2011-12-05 15:10:56+05:30  Cprogrammer
 * added version information
 *
 * Revision 1.75  2011-10-29 20:42:53+05:30  Cprogrammer
 * added CRAM-RIPEMD, CRAM-SHA1, DIGEST-MD5 auth methods
 *
 * Revision 1.74  2011-07-08 13:52:25+05:30  Cprogrammer
 * ipv6, ipv4 code organized
 *
 * Revision 1.73  2011-07-03 16:56:26+05:30  Cprogrammer
 * use control_readrandom() to pick up a random line from control file
 *
 * Revision 1.72  2011-01-08 16:30:05+05:30  Cprogrammer
 * use OUTGOINGIP env variable to set local interface address
 *
 * Revision 1.71  2011-01-06 22:40:06+05:30  Cprogrammer
 * added environment variable OUTIP to select OUTGOINGIP
 * added check for correct number of command line arguments
 *
 * Revision 1.70  2010-08-05 20:56:37+05:30  Cprogrammer
 * added cram-md5 authentication
 *
 * Revision 1.69  2010-07-26 19:26:23+05:30  Cprogrammer
 * added default case in switch statement in run_script function()
 *
 * Revision 1.68  2010-07-25 19:48:21+05:30  Cprogrammer
 * replaced success(), failure() script with a single run_script()
 *
 * Revision 1.67  2010-07-24 20:15:31+05:30  Cprogrammer
 * define ERRTEXT env variable for ONSUCCESS_REMOTE, ONFAILURE_REMOTE scripts
 *
 * Revision 1.66  2010-07-24 17:37:52+05:30  Cprogrammer
 * fixed SMTPCODE, SMTPTEXT not getting set for failures
 * fixed logic for quit() function
 * execute failure() script only for permanent failures
 *
 * Revision 1.65  2010-07-20 20:11:12+05:30  Cprogrammer
 * execute program/script on failure if ONFAILURE_REMOTE is defined
 * set SMTPTEXT, SMTPCODE environment variables if success() or failure() function
 * is executed
 *
 * Revision 1.64  2010-07-17 16:21:32+05:30  Cprogrammer
 * use qmail-remote as argv0 when executing ONSUCCESS_REMOTE program
 *
 * Revision 1.63  2010-07-16 15:41:02+05:30  Cprogrammer
 * execute script on successful delivery
 *
 * Revision 1.62  2010-07-08 21:53:42+05:30  Cprogrammer
 * domainbindings based on envelope sender address
 *
 * Revision 1.61  2010-06-27 08:43:43+05:30  Cprogrammer
 * display bind ip (outgoing ip) in error messages for failed connections
 *
 * Revision 1.60  2010-06-24 08:54:30+05:30  Cprogrammer
 * made outgoingip control file name configureable
 *
 * Revision 1.59  2010-05-29 20:54:41+05:30  Cprogrammer
 * environment variable QMTPROUTE, SMTPROUTE takes precedence over control files
 *
 * Revision 1.58  2010-05-29 17:27:27+05:30  Cprogrammer
 * added fallback to SMTP as per MXPS
 *
 * Revision 1.57  2010-05-28 14:25:00+05:30  Cprogrammer
 * indicate protocol in the accepted message
 *
 * Revision 1.56  2010-04-30 13:17:44+05:30  Cprogrammer
 * fixed domainbindings for ipv4 addresses
 *
 * Revision 1.55  2010-04-24 20:13:17+05:30  Cprogrammer
 * ability to use either QMTP or SMTP for clustered domains
 *
 * Revision 1.54  2010-03-25 10:16:24+05:30  Cprogrammer
 * added QMTP support
 *
 * Revision 1.53  2010-02-01 10:11:30+05:30  Cprogrammer
 * fix for xtext function
 *
 * Revision 1.52  2009-12-17 09:15:55+05:30  Cprogrammer
 * log real envelope recipients of messages after host name canonicalization
 * patch by James Raftery
 * log from and recipient on success and failure
 *
 * Revision 1.51  2009-12-05 20:16:03+05:30  Cprogrammer
 * ansic conversion
 *
 * Revision 1.50  2009-11-12 19:29:33+05:30  Cprogrammer
 * record the helo name if rejected (Stupid MS Exchange rejects helo domain)
 *
 * Revision 1.49  2009-09-16 15:45:56+05:30  Cprogrammer
 * do not try next MX for permanent errors
 *
 * Revision 1.48  2009-09-02 11:30:58+05:30  Cprogrammer
 * added Bounce Address Tag Validation - BATV
 *
 * Revision 1.47  2009-08-13 19:09:09+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.46  2009-05-07 08:47:52+05:30  Cprogrammer
 * made DOMAINBINDINGS file configurable
 *
 * Revision 1.45  2009-05-06 22:56:26+05:30  Cprogrammer
 * use per recipient domain outgoingip
 *
 * Revision 1.44  2009-04-16 11:01:58+05:30  Cprogrammer
 * made min_penalty, max_tolerance configurable through env variables
 *
 * Revision 1.43  2008-09-30 08:42:33+05:30  Cprogrammer
 * have domainbindings take precedence over outgoingips
 *
 * Revision 1.42  2008-09-16 08:25:40+05:30  Cprogrammer
 * fixed case sensitivity
 *
 * Revision 1.41  2008-07-15 20:04:05+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.40  2008-06-01 15:38:32+05:30  Cprogrammer
 * Frederik Vermeulen <qmail-tls akrul inoa.net> 20070408 TLS patch
 *
 * Revision 1.39  2008-02-05 15:32:13+05:30  Cprogrammer
 * added domainbinding functionality
 *
 * Revision 1.38  2007-12-20 13:55:58+05:30  Cprogrammer
 * initialized variable i = 0
 *
 * Revision 1.37  2006-02-17 16:14:53+05:30  Cprogrammer
 * print the host for which SMTP connecton failed
 *
 * Revision 1.36  2005-12-29 14:02:30+05:30  Cprogrammer
 * made min_penalty, max_tolerance configurable.
 * initialized fdmoreroutes to -1
 *
 * Revision 1.35  2005-06-17 21:49:50+05:30  Cprogrammer
 * ipv6 support
 * replaced struct ip_address and struct ip6_address with shorter typedefs
 *
 * Revision 1.34  2005-06-11 21:46:52+05:30  Cprogrammer
 * changes for ipv6
 *
 * Revision 1.33  2005-04-05 20:06:34+05:30  Cprogrammer
 * use Authenticated SMTP only if AUTH_SMTP is defined
 *
 * Revision 1.32  2005-04-03 14:29:59+05:30  Cprogrammer
 * added authenticated SMTP methods
 *
 * Revision 1.31  2005-02-14 23:06:00+05:30  Cprogrammer
 * added moresmtproutes
 *
 * Revision 1.30  2004-11-03 23:42:11+05:30  Cprogrammer
 * BUG - memory error erroneously skipped
 *
 * Revision 1.29  2004-10-22 20:29:30+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.28  2004-10-22 15:37:55+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.27  2004-10-11 13:58:50+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.26  2004-07-30 18:05:19+05:30  Cprogrammer
 * TLS code overhaul - Fredrik Vermeulen 20040419
 *
 * Revision 1.25  2004-07-17 21:21:26+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.24  2004-03-17 12:16:40+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.23  2003-12-05 14:46:51+05:30  Cprogrammer
 * comment for arguments corrected
 *
 * Revision 1.22  2003-11-25 20:46:01+05:30  Cprogrammer
 * do not compare with mapsmtproutes if smtproutes is absent
 *
 * Revision 1.21  2003-10-23 01:25:33+05:30  Cprogrammer
 * replaced strcmp with str_diffn
 * fixed compilation warnings
 *
 * Revision 1.20  2003-10-13 10:03:59+05:30  Cprogrammer
 * try ESMTP SIZE extention of remote smtp supports it
 *
 * Revision 1.19  2003-10-01 19:05:39+05:30  Cprogrammer
 * changed return type to int
 *
 * Revision 1.18  2003-09-29 16:48:38+05:30  Cprogrammer
 * corrected clobbering of smtptext by smtpcode() when STARTTLS was done
 *
 * Revision 1.17  2003-08-02 16:15:54+05:30  Cprogrammer
 * use ehlo protocol when using authenticated SMTP
 *
 * Revision 1.16  2003-07-20 17:13:24+05:30  Cprogrammer
 * TLS patch
 *
 * Revision 1.15  2003-07-07 00:05:47+05:30  Cprogrammer
 * option to ship mails through AUTH SMTP
 *
 * Revision 1.14  2002-12-16 20:23:25+05:30  Cprogrammer
 * added display of port in connection errors
 *
 * Revision 1.13  2002-09-14 20:50:01+05:30  Cprogrammer
 * corrected display of ip addresses in temp_noconn()
 *
 * Revision 1.12  2002-09-11 15:41:02+05:30  Cprogrammer
 * changed error messages to display the host
 *
 * Revision 1.11  2002-09-11 11:48:13+05:30  Cprogrammer
 * try next MX if connection to smtp port dies before remote end replies with a greeting
 *
 * Revision 1.10  2002-09-10 20:05:22+05:30  Cprogrammer
 * try next MX also for permanent errors (rfc 2821)
 *
 * Revision 1.9  2002-09-08 23:47:12+05:30  Cprogrammer
 * RFC 2821 support - Retry next MX on transient errors
 *
 * Revision 1.8  2002-09-07 20:35:41+05:30  Cprogrammer
 * removed unecessary stralloc_ready()
 *
 * Revision 1.7  2002-08-16 19:49:57+05:30  Cprogrammer
 * added printing of correct control directory in perm_ambigmx()
 *
 * Revision 1.6  2002-08-16 19:31:53+05:30  Cprogrammer
 * added printing of the correct control directory
 *
 * Revision 1.5  2002-08-15 19:22:44+05:30  Cprogrammer
 * change for configurable control dir
 *
 * Revision 1.4  2002-08-14 15:00:10+05:30  Cprogrammer
 * RFC 2821 compliance
 *
 * Revision 1.3  2002-03-19 20:45:04+05:30  Cprogrammer
 * included env.h to supress compiler warnings
 *
 * Revision 1.2  2002-03-13 11:29:09+05:30  Cprogrammer
 * code indentation
 * removed mysql code
 * added code to get variables from environment variable SMTPROUTE
 *
 * Revision 1.1  2001-12-23 00:55:49+05:30  Cprogrammer
 * Initial revision
 *
 */
