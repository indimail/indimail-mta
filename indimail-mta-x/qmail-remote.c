/*-
 * RCS log at bottom
 * $Id: qmail-remote.c,v 1.152 2022-08-22 22:23:34+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cdb.h>
#include <open.h>
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
#include "tls.h"
#include "ssl_timeoutio.h"
#include "hastlsv1_1_client.h"
#include "hastlsv1_2_client.h"
#ifdef HASTLSA
#include "tlsacheck.h"
#endif
#endif /*- #ifdef TLS */

#include "haslibgsasl.h"
#ifdef HASLIBGSASL
#include <gsasl.h>
#endif

/* email address internationalization EAI */
#include "hassmtputf8.h"
#ifdef SMTPUTF8
#include <idn2.h>
#endif

#include "auto_control.h"
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

#define EHLO 1
#define HUGESMTPTEXT  5000
#define MIN_PENALTY   3600
#define MAX_TOLERANCE 120

#define PORT_SMTP     25 /*- silly rabbit, /etc/services is for users */
#define PORT_QMTP     209

unsigned long   port = PORT_SMTP;

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)

stralloc        sauninit = { 0 };
stralloc        helohost = { 0 };
stralloc        senderbind = { 0 };
stralloc        localips = { 0 };
stralloc        helohosts = { 0 };
stralloc        outgoingip = { 0 };

int             cntrl_stat1, cntrl_stat2;
stralloc        smtproutes = { 0 };
stralloc        qmtproutes = { 0 };

struct constmap mapsmtproutes;
struct constmap mapqmtproutes;
stralloc        bounce = { 0 };

int             protocol_t = 0;	/*- defines smtps, smtp, qmtp */
#ifdef MXPS
int             mxps = 0;
#endif
struct constmap maplocalips;
struct constmap maphelohosts;
stralloc        host = { 0 };
stralloc        rhost = { 0 }; /*- host to which qmail-remote ultimately connects */
stralloc        qmtp_sender = { 0 };
stralloc        smtp_sender = { 0 };
stralloc        qqeh = { 0 };
stralloc        user = { 0 };
stralloc        pass = { 0 };
stralloc        slop = { 0 };
stralloc        chal = { 0 };
saa             qmtp_reciplist = { 0 };
saa             smtp_reciplist = { 0 };

struct ip_mx    partner;
union v46addr   outip;
static int      inside_greeting = 0;
static char    *msgsize, *use_auth_smtp;
char          **my_argv;
int             my_argc;
#ifdef TLS
const char     *ssl_err_str = 0;
stralloc        saciphers = { 0 }, tlsFilename = { 0 }, clientcert = { 0 };
SSL_CTX        *ctx;
int             notls = 0;
stralloc        notlshosts = { 0 } ;
struct constmap mapnotlshosts;
#endif

int             fdmoreroutes = -1;
int             flagtcpto = 1;
int             min_penalty = MIN_PENALTY;
unsigned long   max_tolerance = MAX_TOLERANCE;
stralloc        smtptext = { 0 };
stralloc        smtpenv = { 0 };
stralloc        helo_str = { 0 };

/*- http://mipassoc.org/pipermail/batv-tech/2007q4/000032.html */
#ifdef BATV
#include <openssl/ssl.h>
#define BATVLEN 3 /*- number of bytes */
#include "byte.h"
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/evp.h>
#else
#include <openssl/md5.h>
#endif
char            batvok = 0;
stralloc        signkey = { 0 };
stralloc        nosign = { 0 };
stralloc        nosigndoms = { 0 };

struct constmap mapnosign;
struct constmap mapnosigndoms;
#endif

#if defined(TLS) && defined(HASTLSA)
char           *do_tlsa = 0, *tlsadomainsfn = 0;
int             use_daned = 0;
tlsarralloc     ta = { 0 };
stralloc        hexstring = { 0 };
stralloc        hextmp = { 0 };
stralloc        tlsadomains = { 0 };
struct constmap maptlsadomains;
#endif

#ifdef SMTPUTF8
static stralloc header = { 0 };
static stralloc idnhost = { 0 };
static int      smtputf8 = 0;    /*- if remote has SMTPUTF8 capability */
static char    *enable_utf8 = 0; /*- enable utf8 using SMTPUTF8 env variable */
int             flagutf8;        /*- sender, recipient or received header has UTF8 */
#endif

#ifdef HASLIBGSASL
static stralloc gsasl_str = { 0 };
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
		args[i] = 0;
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
		ssl = 0;
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
			strnum[fmt_ulong(strnum, code)] = 0;
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
			strnum[fmt_ulong(strnum, max_tolerance)] = 0;
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
		out((char *) ssl_err_str);
		out(" ");
		if (!stralloc_cats(&smtptext, (char *) ssl_err_str) ||
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
		strnum[fmt_ulong(strnum, port_num)] = 0;
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
		strnum[fmt_ulong(strnum, port_num)] = 0;
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

int             timeoutconnect = 60;
int             smtpfd;
int             timeout = 1200;

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

#ifdef TLS
	if (ssl) {
		if ((r = ssl_timeoutread(timeout, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutread(timeout, smtpfd, buf, len);
#else
	r = timeoutread(timeout, smtpfd, buf, len);
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
		if ((r = ssl_timeoutwrite(timeout, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutwrite(timeout, smtpfd, buf, len);
#else
	r = timeoutwrite(timeout, smtpfd, buf, len);
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
	str[i] = 0;
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
				*p = 0;
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
quit(char *prepend, char *append, int code, int die)
{
	if (substdio_putflush(&smtpto, "QUIT\r\n", 6) == -1)
		temp_write();
	/*- waiting for remote side is just too ridiculous */
	out(prepend);
	outhost();
	out(append);
	out(".\n");
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

#ifdef TLS

char           *partner_fqdn = 0;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define SSL_ST_BEFORE 0x4000
#endif
no_return void
tls_quit(const char *s1, char *s2, char *s3, char *s4, stralloc *saptr)
{
	char            ch;
	int             i, state;

	out((char *) s1);
	if (s2)
		out((char *) s2);
	if (s3)
		out((char *) s3);
	if (s4)
		out((char *) s4);
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

int
match_partner(char *s, int len)
{
	if (!case_diffb(partner_fqdn, len, (char *) s) && !partner_fqdn[len])
		return 1;
	/*- we also match if the name is *.domainname */
	if (*s == '*') {
		const char     *domain = partner_fqdn + str_chr(partner_fqdn, '.');
		if (!case_diffb((char *) domain, --len, (char *) ++s) && !domain[len])
			return 1;
	}
	return 0;
}

/*-
 * don't want to fail handshake if certificate can't be verified
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX *ctx_t)
{
	return 1;
}

void
do_pkix(char *servercert)
{
	X509           *peercert;
	STACK_OF(GENERAL_NAME) *gens;
	int             r, i = 0;
	char           *t;

	/*- PKIX */
	if ((r = SSL_get_verify_result(ssl)) != X509_V_OK) {
		t = (char *) X509_verify_cert_error_string(r);
		if (!stralloc_copyb(&smtptext, "TLS unable to verify server with ", 33) ||
				!stralloc_cats(&smtptext, servercert) ||
				!stralloc_catb(&smtptext, ": ", 2) ||
				!stralloc_cats(&smtptext, t))
			temp_nomem();
		tls_quit("ZTLS unable to verify server with ", servercert, ": ", t, 0);
	}
	if (!(peercert = SSL_get_peer_certificate(ssl))) {
		if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28) ||
				!stralloc_cats(&smtptext, partner_fqdn) ||
				!stralloc_catb(&smtptext, ": no certificate provided", 25))
			temp_nomem();
		tls_quit("ZTLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);
	}

	/*-
	 * RFC 2595 section 2.4: find a matching name
	 * first find a match among alternative names
	 */
	if ((gens = X509_get_ext_d2i(peercert, NID_subject_alt_name, 0, 0))) {
		for (i = 0, r = sk_GENERAL_NAME_num(gens); i < r; ++i) {
			const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, i);
			if (gn->type == GEN_DNS)
				if (match_partner((char *) gn->d.ia5->data, gn->d.ia5->length))
					break;
		}
		sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
	}

	/*- no alternative name matched, look up commonName */
	if (!gens || i >= r) {
		stralloc        peer = { 0 };
		X509_NAME      *subj = X509_get_subject_name(peercert);
		if ((i = X509_NAME_get_index_by_NID(subj, NID_commonName, -1)) >= 0) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			X509_NAME_ENTRY *xnet;
			xnet = X509_NAME_get_entry(subj, i);
			ASN1_STRING    *s = X509_NAME_ENTRY_get_data(xnet);
#else
			const ASN1_STRING *s = X509_NAME_get_entry(subj, i)->value;
#endif
			if (s) {
				peer.len = s->length;
				peer.s = (char *) s->data;
			}
		}
		if (peer.len <= 0) {
			if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28) ||
					!stralloc_cats(&smtptext, partner_fqdn) ||
					!stralloc_catb(&smtptext, ": certificate contains no valid commonName", 42))
				temp_nomem();
			tls_quit("ZTLS unable to verify server ", partner_fqdn, ": certificate contains no valid commonName", 0, 0);
		}
		if (!match_partner((char *) peer.s, peer.len)) {
			if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28) ||
					!stralloc_cats(&smtptext, partner_fqdn) ||
					!stralloc_catb(&smtptext, ": received certificate for ", 27) ||
					!stralloc_cat(&smtptext, &peer) ||
					!stralloc_0(&smtptext))
				temp_nomem();
			tls_quit("ZTLS unable to verify server ", partner_fqdn, ": received certificate for ", 0, &peer);
		}
	}
	X509_free(peercert);
	return;
}

/*
 * 1. returns 0 --> fallback to non-tls
 *    if certs do not exist
 *    host is in notlshosts
 *    smtps == 0 and tls session cannot be initiated
 * 2. returns 1 if tls session was initiated
 * 3. exits on error, if smtps == 1 and tls session did not succeed
 */
int
tls_init(int pkix, int *needtlsauth, char **scert)
{
	int             code, i = 0, _needtlsauth = 0;
	static char     ssl_initialized;
	const char     *ciphers = 0;
	char           *t, *servercert = 0, *certfile;
	static SSL     *myssl;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	stralloc        ssl_option = { 0 };
	int             method = 4;	/*- (1..2 unused) [1..3] = ssl[1..3], 4 = tls1, 5=tls1.1, 6=tls1.2 */
#endif
	int             method_fail = 1;

	if (notls) /*- if found in control/notlshosts control file */
		return (0);
	if (needtlsauth)
		*needtlsauth = 0;
	if (scert)
		*scert = 0;
	/*-
	 * tls_init() gets called in smtp()
	 * if smtp() returns for trying next mx
	 * we need to re-initialize
	 */
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}
	if (myssl) {
		SSL_free(myssl);
		ssl = myssl = (SSL *) 0;
	}
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (!stralloc_copys(&tlsFilename, controldir) ||
			!stralloc_catb(&tlsFilename, "/tlsclientmethod", 16) ||
			!stralloc_0(&tlsFilename))
		temp_nomem();
	if (control_rldef(&ssl_option, tlsFilename.s, 0, "TLSv1_2") != 1)
		temp_control("Unable to read control files", tlsFilename.s);
	if (!stralloc_0(&ssl_option))
		temp_nomem();
	if (str_equal(ssl_option.s, "SSLv23"))
		method = 2;
	else
	if (str_equal(ssl_option.s, "SSLv3"))
		method = 3;
	else
	if (str_equal(ssl_option.s, "TLSv1"))
		method = 4;
	else
	if (str_equal(ssl_option.s, "TLSv1_1"))
		method = 5;
	else
	if (str_equal(ssl_option.s, "TLSv1_2"))
		method = 6;
#endif
	if (!certdir && !(certdir = env_get("CERTDIR")))
		certdir = auto_control;
	if (!stralloc_copys(&clientcert, certdir) ||
			!stralloc_append(&clientcert, "/"))
		temp_nomem();
	certfile = ((certfile = env_get("CLIENTCERT")) ? certfile : "clientcert.pem");
	if (!stralloc_cats(&clientcert, certfile) ||
			!stralloc_0(&clientcert))
		temp_nomem();
	if (access(clientcert.s, F_OK)) {
		if (errno != error_noent)
			temp_control("Unable to read client certificate", clientcert.s);
		return (0);
	}
	if (partner_fqdn) {
		struct stat     st;
		if (!stralloc_copys(&tlsFilename, certdir) ||
				!stralloc_catb(&tlsFilename, "/tlshosts/", 10) ||
				!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn)) ||
				!stralloc_catb(&tlsFilename, ".pem", 4) ||
				!stralloc_0(&tlsFilename))
			temp_nomem();
		if (stat(tlsFilename.s, &st)) {
			_needtlsauth = 0;
			if (needtlsauth)
				*needtlsauth = 0;
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/notlshosts/", 12) ||
					!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn) + 1) ||
					!stralloc_0(&tlsFilename)) /*- fqdn */
				temp_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/notlshosts/", 12) ||
					!stralloc_catb(&tlsFilename, host.s, host.len) ||
					!stralloc_0(&tlsFilename)) /*- domain */
				temp_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
			if (!stralloc_copys(&tlsFilename, certdir) ||
					!stralloc_catb(&tlsFilename, "/tlshosts/exhaustivelist", 24) ||
					!stralloc_0(&tlsFilename))
				temp_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
		} else {
			*scert = servercert = tlsFilename.s;
			_needtlsauth = 1;
			if (needtlsauth)
				*needtlsauth = 1;
		}
	}

	if (!smtps) {
		stralloc       *saptr = ehlokw.sa;
		unsigned int    len = ehlokw.len;

		/*- look for STARTTLS among EHLO keywords */
		for (; len && case_diffs(saptr->s, "STARTTLS"); ++saptr, --len);
		if (!len) {
			if (!_needtlsauth)
				return (0);
			if (!stralloc_copyb(&smtptext, "No TLS achieved while ", 22) ||
					!stralloc_cats(&smtptext, servercert) ||
					!stralloc_catb(&smtptext, " exists", 7))
				temp_nomem();
			tls_quit("ZNo TLS achieved while", tlsFilename.s, " exists", 0, 0);
		}
	}
	if (!ssl_initialized++)
		SSL_library_init();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (method == 2 && (ctx = SSL_CTX_new(SSLv23_client_method())))
		method_fail = 0;
	else
	if (method == 3 && (ctx = SSL_CTX_new(SSLv3_client_method())))
		method_fail = 0;
#if defined(TLSV1_CLIENT_METHOD) || defined(TLS1_VERSION)
	else
	if (method == 4 && (ctx = SSL_CTX_new(TLSv1_client_method())))
		method_fail = 0;
#endif
#if defined(TLSV1_1_CLIENT_METHOD) || defined(TLS1_1_VERSION)
	else
	if (method == 5 && (ctx = SSL_CTX_new(TLSv1_1_client_method())))
		method_fail = 0;
#endif
#if defined(TLSV1_2_CLIENT_METHOD) || defined(TLS1_2_VERSION)
	else
	if (method == 6 && (ctx = SSL_CTX_new(TLSv1_2_client_method())))
		method_fail = 0;
#endif
#else /*- #if OPENSSL_VERSION_NUMBER < 0x10100000L */
	if ((ctx = SSL_CTX_new(TLS_client_method())))
		method_fail = 0;
	/*- SSL_OP_NO_SSLv3, SSL_OP_NO_TLSv1, SSL_OP_NO_TLSv1_1 and SSL_OP_NO_TLSv1_2 */
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
#endif /*- #if OPENSSL_VERSION_NUMBER < 0x10100000L */
	if (method_fail) {
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			return (0);
		}
		t = (char *) ssl_error();
		if (!stralloc_copyb(&smtptext, "TLS error initializing ctx: ", 28) ||
				!stralloc_cats(&smtptext, t))
			temp_nomem();
		SSL_CTX_free(ctx);
		switch (method_fail)
		{
		case 2:
			tls_quit("ZTLS error initializing SSLv23 ctx: ", t, 0, 0, 0);
			break;
		case 3:
			tls_quit("ZTLS error initializing SSLv3 ctx: ", t, 0, 0, 0);
			break;
		case 4:
			tls_quit("ZTLS error initializing TLSv1 ctx: ", t, 0, 0, 0);
			break;
		case 5:
			tls_quit("ZTLS error initializing TLSv1_1 ctx: ", t, 0, 0, 0);
			break;
		case 6:
			tls_quit("ZTLS error initializing TLSv1_2 ctx: ", t, 0, 0, 0);
			break;
		}
	}

	if (_needtlsauth) {
		if (!SSL_CTX_load_verify_locations(ctx, servercert, NULL)) {
			t = (char *) ssl_error();
			if (!stralloc_copyb(&smtptext, "TLS unable to load ", 19) ||
					!stralloc_cats(&smtptext, servercert) ||
					!stralloc_catb(&smtptext, ": ", 2) ||
					!stralloc_cats(&smtptext, t))
				temp_nomem();
			SSL_CTX_free(ctx);
			tls_quit("ZTLS unable to load ", servercert, ": ", t, 0);
		}
		/*- set the callback here; SSL_set_verify didn't work before 0.9.6c */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
	}

	/*- let the other side complain if it needs a cert and we don't have one */
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert.s))
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		SSL_CTX_use_PrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
#else
		SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
#endif

	if (!(myssl = SSL_new(ctx))) {
		t = (char *) ssl_error();
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			return (0);
		}
		if (!stralloc_copyb(&smtptext, "TLS error initializing ssl: ", 28) ||
				!stralloc_cats(&smtptext, t))
			temp_nomem();
		SSL_CTX_free(ctx);
		tls_quit("ZTLS error initializing ssl: ", t, 0, 0, 0);
	} else
		SSL_CTX_free(ctx);

	if (!smtps && substdio_putflush(&smtpto, "STARTTLS\r\n", 10) == -1)
		temp_write();

	/*- while the server is preparing a response, do something else */
	if (!ciphers) {
		if (control_readfile(&saciphers, "tlsclientciphers", 0) == -1) {
			while (SSL_shutdown(myssl) == 0);
			SSL_free(myssl);
			temp_control("Unable to read control file", "tlsclientciphers");
		}
		if (saciphers.len) {
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		} else
			ciphers = "DEFAULT";
	}
	SSL_set_cipher_list(myssl, ciphers);

	/*- SSL_set_options(myssl, SSL_OP_NO_TLSv1); */
	SSL_set_fd(myssl, smtpfd);

	/*- read the response to STARTTLS */
	if (!smtps) {
		if (smtpcode() != 220) {
			while (SSL_shutdown(myssl) == 0);
			SSL_free(myssl);
			ssl = myssl = (SSL *) 0;
			if (!_needtlsauth)
				return (0);
			if (!stralloc_copyb(&smtptext, "STARTTLS rejected while ", 24) ||
					!stralloc_cats(&smtptext, tlsFilename.s) ||
					!stralloc_catb(&smtptext, " exists", 7))
				temp_nomem();
			tls_quit("ZSTARTTLS rejected while ", tlsFilename.s, " exists", 0, 0);
		}
	}
	ssl = myssl;
	if (ssl_timeoutconn(timeout, smtpfd, smtpfd, ssl) <= 0) {
		t = (char *) ssl_error_str();
		if (!stralloc_copyb(&smtptext, "TLS connect failed: ", 20) ||
				!stralloc_cats(&smtptext, t))
			temp_nomem();
		tls_quit("ZTLS connect failed: ", t, 0, 0, 0);
	}
	if (smtps && (code = smtpcode()) != 220)
		quit("ZTLS Connected to ", " but greeting failed", code, -1);
	if (pkix && _needtlsauth) /*- 220 ready for tls */
		do_pkix(servercert);
	return (1);
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

stralloc        temphost = { 0 };
stralloc        sa = { 0 };

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

/*-
 * USAGE
 * 0, 1, 2, 3, 255
 * 255 PrivCert
 *
 * SELECTOR
 * --------
 * 0, 1, 255
 * 255 PrivSel
 *
 * Matching Type
 * -------------
 * 0, 1, 2, 255
 * 255 PrivMatch
 *
 * Return Value
 * 0   - match target certificate & payload data
 * 1,2 - successful match
 */
stralloc        certData = { 0 };
int
tlsa_vrfy_records(char *certDataField, int usage, int selector, int match_type, char **err_str )
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX     *mdctx;
#else
	EVP_MD_CTX     *mdctx;
	EVP_MD_CTX      _mdctx;
#endif
	const EVP_MD   *md = 0;
	BIO            *membio = 0;
	EVP_PKEY       *pkey = 0;
	X509           *xs = 0;
	STACK_OF(X509) *sk;
	static char     errbuf[256];
	char            buffer[1024], hex[2];
	unsigned char   md_value[EVP_MAX_MD_SIZE];
	unsigned char  *ptr;
	char           *cp;
	int             i, len;
	unsigned int    md_len;

	if (!ssl)
		return (-1);
	switch (usage)
	{
	/*- Trust anchor */
	case 0: /*- PKIX-TA(0) maps to DANE-TA(2) */
		/*- flow through */
	case 2: /*- DANE-TA(2) */
		usage = 2;
		break;
	case 1: /*- PKIX-EE(1) maps to DANE-EE(3) */
		/*- flow through */
	case 3: /*- DANE-EE(3) */
		usage = 3;
		break;
	default:
		return (-2);
	}
	switch (selector) /*- match full certificate or subjectPublicKeyInfo */
	{
	case 0: /*- Cert(0) - match full certificate   data/SHA256fingerprint/SHA512fingerprint */
		break;
	case 1: /*- SPKI(1) - match subject public key data/SHA256fingerprint/SHA512fingerprint  */
		break;
	default:
		return (-2);
	}
	switch (match_type) /*- sha256, sha512 */
	{
	case 0: /*- Full(0) - match full cert data or subjectPublicKeyInfo data */
		break;
	case 1: /*- SHA256(1) fingerprint - servers should publish this mandatorily */
		md = EVP_get_digestbyname("sha256");
		break;
	case 2: /*- SHA512(2)  fingerprint - servers should not exclusively publish this */
		md = EVP_get_digestbyname("sha512");
		break;
	default:
		return (-2);
	}
	/*- SSL_ctrl(ssl, SSL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, servername); -*/
	if (!(sk =  SSL_get_peer_cert_chain(ssl))) {
		if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28) ||
				!stralloc_cats(&smtptext, partner_fqdn) ||
				!stralloc_catb(&smtptext, ": no certificate provided", 25))
			temp_nomem();
		tls_quit("ZTLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);
	}
	/*-
	 * the server certificate is generally presented
	 * as the first certificate in the stack along with
	 * the remaining chain.
	 * last certificate in the list is a trust anchor
	 * 5.2.2.  Trust Anchor Digests and Server Certificate Chain
	 * https://tools.ietf.org/html/rfc7671
	 *
	 * for Usage 2, check the last  certificate - sk_X509_value(sk, sk_509_num(sk) - 1)
	 * for Usage 3, check the first certificate - sk_X509_value(sk, 0)
	 */
	i = (usage == 2 ? sk_X509_num(sk) - 1 : 0);
	xs = sk_X509_value(sk, i);
	/*- 
	 * DANE Validation 
	 * server cert - X = 2
	 * anchor cert - X = 3
	 * case 1 - match full certificate data -                            - X 0 0
	 * case 2 - match full subjectPublicKeyInfo data                     - X 1 0
	 * case 3 - match  SHA512/SHA256 fingerprint of full certificate     - X 0 1, X 0 2
	 * case 4 - match  SHA512/SHA256 fingerprint of subjectPublicKeyInfo - X 1 1, X 1 2
	 */
	if (match_type == 0 && selector == 0) { /*- match full certificate data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("ZDANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_X509(membio, xs)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("ZDANE unable to create write to membio: ", *err_str, 0, 0, 0);
		}
		for (certData.len = 0;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN CERTIFICATE-----"))
				continue;
			if (str_start(buffer, "-----END CERTIFICATE-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				temp_nomem();
		}
		if (!stralloc_0(&certData))
			temp_nomem();
		certData.len--;
		BIO_free_all(membio);
		if (!str_diffn(certData.s, certDataField, certData.len))
			return (0);
		return (1);
	}
	if (match_type == 0 && selector == 1) { /*- match full subjectPublicKeyInfo data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("ZDANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!(pkey = X509_get_pubkey(xs))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("ZDANE unable to get pubkey: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_PUBKEY(membio, pkey)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("ZDANE unable to write pubkey to membio: ", *err_str, 0, 0, 0);
		}
		for (;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN PUBLIC KEY-----"))
				continue;
			if (str_start(buffer, "-----END PUBLIC KEY-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				temp_nomem();
		}
		if (!stralloc_0(&certData))
			temp_nomem();
		certData.len--;
		BIO_free_all(membio);
		if (!str_diffn(certData.s, certDataField, certData.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of full certificate */
	if ((match_type == 2 || match_type == 1) && selector == 0) {
		if (!X509_digest(xs, md, md_value, &md_len))
			tls_quit("ZTLS Unable to get peer cerficatte digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				temp_nomem();
		}
		cp = hextmp.s;
		if (!str_diffn(certDataField, (char *) cp, hextmp.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of subjectPublicKeyInfo */
	if ((match_type == 2 || match_type == 1) && selector == 1) {
		unsigned char  *tmpbuf = (unsigned char *) 0;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		if (!(mdctx = EVP_MD_CTX_new()))
			temp_nomem();
#else
		mdctx = &_mdctx;
#endif
		if (!(pkey = X509_get_pubkey(xs)))
			tls_quit("ZTLS Unable to get public key", 0, 0, 0, 0);
		if (!EVP_DigestInit_ex(mdctx, md, NULL))
			tls_quit("ZTLS Unable to initialize EVP Digest", 0, 0, 0, 0);
		if ((len = i2d_PUBKEY(pkey, NULL)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!(tmpbuf = (unsigned char *) OPENSSL_malloc(len * sizeof(char))))
			temp_nomem();
		ptr = tmpbuf;
		if ((len = i2d_PUBKEY(pkey, &ptr)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!EVP_DigestUpdate(mdctx, tmpbuf, len))
			tls_quit("ZTLS Unable to update EVP Digest", 0, 0, 0, 0);
		OPENSSL_free(tmpbuf);
		if (!EVP_DigestFinal_ex(mdctx, md_value, &md_len))
			tls_quit("ZTLS Unable to compute EVP Digest", 0, 0, 0, 0);
		for (i = hextmp.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hextmp, hex, 2))
				temp_nomem();
		}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		cp = hextmp.s;
		if (!str_diffn(certDataField, (char *) cp, hextmp.len))
			return (0);
		return (1);
	}
	return (1);
}
#endif /*- #ifdef HASTLSA */
#endif /*- #ifdef TLS */

stralloc        xuser = { 0 };
stralloc        auth = { 0 };
stralloc        plain = { 0 };

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
stralloc        realm = { 0 }, nonce = { 0 }, digesturi = { 0 };

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
		quit("ZConnected to ", " but authentication was rejected (AUTH DIGEST-MD5).", code, -1);
	if ((i = str_chr(smtptext.s + 5, '\n')) > 0) {	/*- Challenge */
		slop.len = 0;
		if (!stralloc_copyb(&slop, smtptext.s + 4, smtptext.len - 5))
			temp_nomem();
		if (b64decode((unsigned char *) slop.s, slop.len, &chal))
			quit("ZConnected to ", " but unable to base64decode challenge.", -1, -1);
	} else
		quit("ZConnected to ", " but got no challenge.", -1, -1);

	if (scan_response(&nonce, &chal, "nonce") == 0)
		quit("ZConnected to ", " but got no response= in challenge.", -1, -1);
	s = unique;
	s += (i = fmt_uint((char *) s, getpid()));
	len += i;
	s += (i = fmt_str((char *) s, "."));
	len += i;
	s += (i = fmt_ulong((char *) s, (unsigned long) now()));
	len += i;
	s += (i = fmt_str((char *) s, "@"));
	len += i;
	*s++ = 0;
	hmac_sha1(unique, len, unique + 3, len - 3, digest);
	for (i = 0; i < 20; i++) {
		*x = hextab[digest[i] / 16];
		++x;
		*x = hextab[digest[i] % 16];
		++x;
	}
	*x = 0;
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
		quit("ZConnected to ", " but unable to generate response.", -1, -1);
	if (!stralloc_cats(&slop, (char *) s) ||
			!stralloc_catb(&slop, ",username=\"", 11) ||
			!stralloc_cat(&slop, &user) ||
			!stralloc_catb(&slop, "\"", 1))
		temp_nomem();
	if (b64encode(&slop, &auth))
		quit("ZConnected to ", " but unable to base64encode username+digest.", -1, -1);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
	if (substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_write();
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
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
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-MD5).", code, -1);
		break;
	case AUTH_CRAM_RIPEMD:
		iter = RIPEMD160_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-RIPEMD\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-RIPEMD).", code, -1);
		break;
	case AUTH_CRAM_SHA1:
		iter = SHA_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA1\r\n", 16) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA1).", code, -1);
		break;
	case AUTH_CRAM_SHA224:
		iter = SHA224_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA224\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA224).", code, -1);
		break;
	case AUTH_CRAM_SHA256:
		iter = SHA256_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA256\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA256).", code, -1);
		break;
	case AUTH_CRAM_SHA384:
		iter = SHA384_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA384\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA384).", code, -1);
		break;
	case AUTH_CRAM_SHA512:
		iter = SHA512_DIGEST_LENGTH;
		if (substdio_put(&smtpto, "AUTH CRAM-SHA512\r\n", 18) == -1 ||
				substdio_flush(&smtpto) == -1)
			temp_write();
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA512).", code, -1);
		break;
	}
	if ((j = str_chr(smtptext.s + 5, '\n')) > 0) {	/*- Challenge */
		if (!stralloc_copys(&slop, "") ||
				!stralloc_copyb(&slop, smtptext.s + 4, smtptext.len - 5))
			temp_nomem();
		if (b64decode((unsigned char *) slop.s, slop.len, &chal))
			quit("ZConnected to ", " but unable to base64decode challenge.", -1, -1);
	} else
		quit("ZConnected to ", " but got no challenge.", -1, -1);
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
	*e = 0;

	/*- copy user-id and digest */
	if (!stralloc_copy(&slop, &user) ||
			!stralloc_catb(&slop, " ", 1) ||
			!stralloc_cats(&slop, (char *) encrypted))
		temp_nomem();

	if (b64encode(&slop, &auth))
		quit("ZConnected to ", " but unable to base64encode username+digest.", -1, -1);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1)
		temp_write();
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
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
		quit("ZConnected to ", " but authentication was rejected (AUTH PLAIN).", code, -1);
	if (!stralloc_cat(&plain, &smtp_sender) || /*- Mail From: <auth-id> */
			!stralloc_0(&plain) ||
			!stralloc_cat(&plain, &user) || /*- userid */
			!stralloc_0(&plain) ||
			!stralloc_cat(&plain, &pass) || /*- password */
			!stralloc_0(&plain))
		temp_nomem();
	if (b64encode(&plain, &auth))
		quit("ZConnected to ", " but unable to base64encode (plain).", -1, -1);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (plain).", code, -1);
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
		quit("ZConnected to ", " but authentication was rejected (AUTH LOGIN).", code, -1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&user, &auth))
		quit("ZConnected to ", " but unable to base64encode user.", -1, -1);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (username).", code, -1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&pass, &auth))
		quit("ZConnected to ", " but unable to base64encode pass.", -1, -1);
	if (substdio_put(&smtpto, auth.s, auth.len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		temp_nomem();
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (password)", code, -1);
	mailfrom_xtext(use_size);
}

#ifdef HASLIBGSASL
static void
remove_newline()
{
	int             i;

	if (!stralloc_copyb(&gsasl_str, smtptext.s + 4, smtptext.len - 4))
		temp_nomem();
	i = str_rchr(gsasl_str.s, '\r');
	if (gsasl_str.s[i])
		gsasl_str.s[i] = 0;
	else {
		i = str_rchr(gsasl_str.s, '\n');
		if (gsasl_str.s[i])
			gsasl_str.s[i] = 0;
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
				!stralloc_catb(&gsasl_str, ").", 2) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, code, -1);
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
						!stralloc_catb(&gsasl_str, ").", 2) ||
						!stralloc_0(&gsasl_str))
					temp_nomem();
				quit("ZConnected to ", gsasl_str.s, code, -1);
			}
			remove_newline();
		}
	} while (rc == GSASL_NEEDS_MORE);
	if (rc != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " but authentication failed: ", 28) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, -1, 1);
	}
	/*
	 * The client is done.  Here you would typically check if the server
	 * let the client in.  If not, you could try again. 
	 */
	if ((code = smtpcode()) != 235) {
		if (!stralloc_copyb(&gsasl_str, " but authentication was rejected (AUTH ", 39) ||
				!stralloc_cats(&gsasl_str, mech) ||
				!stralloc_catb(&gsasl_str, ").", 2) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, code, -1);
	}
}

static void
client(Gsasl *gsasl_ctx, int method)
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
		strnum[i = fmt_int(strnum, method)] = 0;
		if (!stralloc_copyb(&gsasl_str, " but got unknown method=", 24) ||
				!stralloc_catb(&gsasl_str, strnum, i) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, -1, 1);
		break;
	}
	/* Create new authentication session.  */
	if ((rc = gsasl_client_start(gsasl_ctx, mech, &session)) != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to initialize client: ", 30) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, -1, 1);
	}

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
		quit("ZConnected to ", gsasl_str.s, -1, 1);
		return;
	}
	rc = gsasl_property_set(session, GSASL_PASSWORD, pass.s);
	if (rc != GSASL_OK) {
		if (!stralloc_copyb(&gsasl_str, " failed to set password: ", 25) ||
				!stralloc_cats(&gsasl_str, gsasl_strerror(rc)) ||
				!stralloc_0(&gsasl_str))
			temp_nomem();
		quit("ZConnected to ", gsasl_str.s, -1, 1);
		return;
	}
#else
	gsasl_property_set(session, GSASL_AUTHID, user.s);
	gsasl_property_set(session, GSASL_PASSWORD, pass.s);
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
		quit("ZConnected to ", gsasl_str.s, -1, 1);
	}
	/* Do it.  */
	client(gsasl_ctx, method);
	/* Cleanup.  */
	gsasl_done(gsasl_ctx);
	mailfrom_xtext(use_size);
	return;
}
#endif

void
smtp_auth(char *type, int use_size)
{
	int             i = 0, login_supp = 0, plain_supp = 0, cram_md5_supp = 0, cram_sha1_supp = 0,
					cram_sha224_supp, cram_sha256_supp = 0, cram_sha384_supp, cram_sha512_supp = 0,
					cram_rmd_supp = 0, digest_md5_supp = 0, secure_auth;
	char           *ptr, *no_auth_login, *no_auth_plain, *no_cram_md5, *no_cram_sha1, *no_cram_sha224,
				   *no_cram_sha256, *no_cram_sha384, *no_cram_sha512, *no_cram_ripemd, *no_digest_md5;
#ifdef HASLIBGSASL
	int             scram_sha1_supp = 0, scram_sha256_supp = 0, scram_sha1_plus_supp = 0,
					scram_sha256_plus_supp = 0;
	char           *no_scram_sha1, *no_scram_sha256, *no_scram_sha1_plus, *no_scram_sha256_plus;
#endif

	if (!type) {
		mailfrom(use_size);
		return;
	}
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
#ifdef HASLIBGSASL
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-1-PLUS"))
				scram_sha1_plus_supp = 1;
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-1"))
				scram_sha1_supp = 1;
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-256-PLUS"))
				scram_sha256_plus_supp = 1;
			else
			if (case_starts(ptr - 5, "SCRAM-SHA-256"))
				scram_sha256_supp = 1;
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
#ifdef HASLIBGSASL
	no_scram_sha1 = env_get("DISABLE_SCRAM_SHA1");
	no_scram_sha256 = env_get("DISABLE_SCRAM_SHA256");
	no_scram_sha1_plus = env_get("DISABLE_SCRAM_SHA1_PLUS");
	no_scram_sha256_plus = env_get("DISABLE_SCRAM_SHA256_PLUS");
#endif
#ifdef HASLIBGSASL
	if (scram_sha1_supp || scram_sha256_supp ||
			scram_sha1_plus_supp || scram_sha256_plus_supp) {
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
		}
	}
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
	} else
	if (!*type) {
#ifdef HASLIBGSASL
		if (!no_scram_sha256_plus && scram_sha256_plus_supp) {
			auth_scram(AUTH_SCRAM_SHA256_PLUS, use_size);
			return;
		} else
		if (!no_scram_sha1_plus && scram_sha1_plus_supp) {
			auth_scram(AUTH_SCRAM_SHA1_PLUS, use_size);
			return;
		} else
		if (!no_scram_sha256 && scram_sha256_supp) {
			auth_scram(AUTH_SCRAM_SHA256, use_size);
			return;
		} else
		if (!no_scram_sha1 && scram_sha1_supp) {
			auth_scram(AUTH_SCRAM_SHA1, use_size);
			return;
		}
#endif
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
		if (!no_cram_ripemd && cram_rmd_supp) {
			auth_cram(AUTH_CRAM_RIPEMD, use_size);
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
		if (!no_digest_md5 && digest_md5_supp) {
			auth_digest_md5(use_size);
			return;
		}
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
		quit("Z", "unable to fstat fd 0", -1, -1);
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
smtp()
{
	unsigned long   code;
	int             flagbother;
	int             i, use_size = 0, is_esmtp = 1;
#if defined(TLS) && defined(HASTLSA)
	char           *err_str = 0, *servercert = 0;
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
		quit("DConnected to ", " but greeting failed", code, 1);
	else
	if (code >= 400 && code < 500)
		return;		/*- try next MX, see RFC-2821 */
	else
	if (code != 220)
		quit("ZConnected to ", " but greeting failed", code, -1);
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
		match0Or512 = authfullMatch = authsha256 = authsha512 = 0;
		if (!tls_init(0, &needtlsauth, &servercert)) /*- tls is needed for DANE */
			quit("ZConnected to ", " but unable to intiate TLS for DANE", 530, -1);
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
			if (!(tlsa_status = tlsa_vrfy_records(hexstring.s, rp->usage, rp->selector, rp->mtype, &err_str))) {
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
			(void) tlsacheck(do_tlsa, partner_fqdn, UPDATE_SUCCESS, rbuf, timeoutfn, err_tmpfail);
			if (needtlsauth && (!usage || usage == 2))
				do_pkix(servercert);
			code = ehlo();
		} else { /*- dane validation failed */
			if (use_daned)
				(void) tlsacheck(do_tlsa, partner_fqdn, UPDATE_FAILURE, rbuf, timeoutfn, err_tmpfail);
			quit("DConnected to ", " but recpient failed DANE validation", 534, 1);
		}
	} else /*- no tlsa rr records */
	if (tls_init(1, 0, 0))
		code = ehlo();
#else
	if (tls_init(1, 0, 0))
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
		quit(code >= 500 ? "DConnected to " : "ZConnected to ", helo_str.s, code, code >= 500 ? 1 : -1);
	}
	/*-
 	 * go through all lines of the multi line answer until one begins
 	 * with "XXX[ |-]SIZE", XXX[ |-]SMTPUTF8 or we reach the last line
 	 */
	if (is_esmtp) {
		i = 0;
		do {
			i += 5 + str_chr(smtptext.s + i, '\n');
			use_size = !case_diffb(smtptext.s + i, 4, "SIZE");
#ifdef SMTPUTF8
			smtputf8 = enable_utf8 ? !case_diffb(smtptext.s + i, 9, "SMTPUTF8") : 0;
			if (enable_utf8) {
				if (use_size && smtputf8)
					break;
			} else
			if (use_size)
				break;
#else
			if (use_size)
				break;
#endif
		} while (smtptext.s[i - 1] == '-');
	}
#ifdef SMTPUTF8
	if (enable_utf8) {
		if (!flagutf8)
			checkutf8message();
		if (flagutf8 && !smtputf8)
			quit("DConnected to ", " but server does not support unicode in email addresses", 553, 1);
	}
#endif
	smtp_auth(use_auth_smtp, use_size);
	substdio_flush(&smtpto);
	code = smtpcode();
	if (code >= 500)
		quit("DConnected to ", " but sender was rejected", code, 1);
	if (code >= 400)
		quit("ZConnected to ", " but sender was rejected", code, -1);
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
		quit("DGiving up on ", "", code, 1);
	if (substdio_putflush(&smtpto, "DATA\r\n", 6) == -1)
		temp_write();
	code = smtpcode();
	if (code >= 500)
		quit("D", " failed on DATA command", code, 1);
	if (code >= 400)
		quit("Z", " failed on DATA command", code, -1);
#ifdef SMTPUTF8
	if (enable_utf8 && header.len &&
			substdio_put(&smtpto, header.s, header.len) == -1)
		temp_write();
#endif
	blast();
	code = smtpcode();
	flagcritical = 0;
	if (code >= 500)
		quit("D", " failed after I sent the message", code, 1);
	if (code >= 400)
		quit("Z", " failed after I sent the message", code, -1);
	quit("K", " accepted message - Protocol SMTP", code, 0);
}

stralloc        canonhost = { 0 };
stralloc        canonbox = { 0 };

void	 /*- host has to be canonical, box has to be quoted */
addrmangle(stralloc *saout, char *s, int *flagalias, int flagcname,
		int flagquote)
{
	int             j;

	*flagalias = flagcname;
	j = str_rchr(s, '@');
	if (!s[j]) {
		if (!stralloc_copys(saout, s))
			temp_nomem();
		return;
	}
	if (!stralloc_copys(&canonbox, s))
		temp_nomem();
	canonbox.len = j;
	if (!(flagquote ? quote(saout,&canonbox) : stralloc_copy(saout,&canonbox)) ||
			!stralloc_cats(saout, "@") ||
			!stralloc_copys(&canonhost, s + j + 1))
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
int
cdb_match(char *fn, char *addr, int len)
{
	static stralloc controlfile = {0};
	static stralloc temp = { 0 };
	uint32          dlen;
	int             fd_cdb, cntrl_ok;

	if (!len || !*addr || !fn)
		return (0);
	if (!stralloc_copys(&controlfile, controldir) ||
			!stralloc_cats(&controlfile, "/") ||
			!stralloc_cats(&controlfile, fn) ||
			!stralloc_cats(&controlfile, ".cdb") ||
			!stralloc_0(&controlfile))
		temp_nomem();
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent)
			temp_control("Unable to read control file", controlfile.s);
		/*- cdb missing or entry missing */
		return (0);
	}
	if (!stralloc_copyb(&temp, addr, len)) {
		close(fd_cdb);
		temp_nomem();
	}
	if ((cntrl_ok = cdb_seek(fd_cdb, temp.s, len, &dlen)) == -1) {
		close(fd_cdb);
		temp_oserr();
	}
	close(fd_cdb);
	return (cntrl_ok ? 1 : 0);
}

static int
dmatch(char *fn, stralloc *domain, stralloc *content,
	struct constmap *ptrmap)
{
	int             x, len;
	char           *ptr;

	if (fn && (x = cdb_match(fn, domain->s, domain->len - 1)))
		return (x);
	else
	if (ptrmap && constmap(ptrmap, domain->s, domain->len - 1))
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
	char           *routes, *senderdomain, *ip, *x;
	char           *smtproutefile, *moresmtproutefile, *qmtproutefile;
	static stralloc controlfile, outgoingipfn;

	if (control_init() == -1)
		temp_control("Unable to initialize control files", 0);
	if (control_readint(&timeout, "timeoutremote") == -1)
		temp_control("Unable to read control file", "timeoutremote");
	if (control_readint(&timeoutconnect, "timeoutconnect") == -1)
		temp_control("Unable to read control file", "timeoutconnect");
	if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
		temp_control("Unable to read control file", "helohost");
	if ((routes = env_get("QMTPROUTE"))) {	/*- mysql */
		if (!stralloc_copyb(&qmtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat2 = 2;
	} else {
		qmtproutefile = (qmtproutefile = env_get("QMTPROUTEFILE")) ? qmtproutefile : "qmtproutes";
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
	case 2:
		if (!constmap_init(&mapqmtproutes, qmtproutes.s, qmtproutes.len, 1))
			temp_nomem();
		break;
	}
	routes = (routes = env_get("SMTPROUTE")) ? routes : env_get("X-SMTPROUTES");
	if (routes) {				/*- mysql or X-SMTPROUTES from header */
		if (!stralloc_copyb(&smtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat1 = 2;
	} else {
		smtproutefile = (smtproutefile = env_get("SMTPROUTEFILE")) ? smtproutefile : "smtproutes";
		moresmtproutefile = (moresmtproutefile = env_get("MORESMTPROUTESCDB")) ? moresmtproutefile : "moresmtproutes.cdb";
		cntrl_stat1 = control_readfile(&smtproutes, smtproutefile, 0);
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
	case 2:
		if (!constmap_init(&mapsmtproutes, smtproutes.s, smtproutes.len, 1))
			temp_nomem();
		break;
	}
#ifdef BATV
	if ((batvok = control_readline(&signkey, (x = env_get("SIGNKEY")) ? x : "signkey")) == -1)
		temp_control("Unable to read control file", x);
	if (batvok) {
		switch (control_readfile(&nosign, "nosignhosts", 0))
		{
		case -1:
			temp_control("Unable to read control file", "nosignhosts");
		case 0:
			if (!constmap_init(&mapnosign, "", 0, 1))
				temp_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosign, nosign.s, nosign.len, 0))
				temp_nomem();
			break;
		}
		switch (control_readfile(&nosigndoms, "nosignmydoms", 0))
		{
		case -1:
			temp_control("Unable to read control file", "nosignmydoms");
		case 0:
			if (!constmap_init(&mapnosigndoms, "", 0, 1))
				temp_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosigndoms, nosigndoms.s, nosigndoms.len, 0))
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
	switch (control_readfile(&tlsadomains, (x = env_get("TLSADOMAINS")) ? x : "tlsadomains", 0))
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
stralloc        newsender = { 0 };
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
no_return void
temp_batv(char *arg)
{
	out("Zerror verifying batv signature. (#4.3.0)\n");
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
	const EVP_MD   *md = 0;
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
			!EVP_DigestUpdate(mdctx, signkey.s, signkey.len) ||
			!EVP_DigestFinal_ex(mdctx, md5digest, &md_len))
		temp_batv("batv: unable to hash md5 message digest");
	EVP_MD_free(md);
#else
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, smtp_sender.s, smtp_sender.len);
	MD5_Update(&md5, signkey.s, signkey.len);
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
lookup_host(char *hst, int len)
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
	protocol_t = env_get("SMTPS") ? 'S' : 's';
	use_auth_smtp = env_get("AUTH_SMTP");
#ifdef SMTPUTF8
	enable_utf8 = env_get("SMTPUTF8");
#endif
	/*- Per user SMTPROUTE functionality using moresmtproutes.cdb */
	relayhost = lookup_host(*recips, str_len(*recips));
	min_penalty = (x = env_get("MIN_PENALTY")) ? scan_int(x, &min_penalty) : MIN_PENALTY;
	max_tolerance = (x = env_get("MAX_TOLERANCE")) ? scan_ulong(x, &max_tolerance) : MAX_TOLERANCE;
	if (smtp_sender.len == 0) { /*- bounce routes */
		if (!stralloc_copys(&bounce, "!@"))
			temp_nomem();
		if ((relayhost = constmap(&mapqmtproutes, bounce.s, bounce.len))) {
			protocol_t = 'q';
			port = PORT_QMTP;
		} else
		if (!(relayhost = constmap(&mapsmtproutes, bounce.s, bounce.len)))
			relayhost = lookup_host("!@", 2);
	}
	if (relayhost && !*relayhost)
		relayhost = 0;
	if (cntrl_stat1 || cntrl_stat2) { /*- set in getcontrols() above */
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
				if ((relayhost = lookup_host(host.s + i, host.len - i)))
					break;
			}
		}
		if (relayhost && !*relayhost)
			relayhost = 0;
	}
	if (relayhost) {
		if (use_auth_smtp) {
			/*-
			 * test.com:x.x.x.x:port username password
			 *         or
			 * domain:relay:port:penalty:max_tolerance username password
			 */
			i = str_chr(relayhost, ' ');
			if (relayhost[i]) {
				relayhost[i] = 0;
				j = str_chr(relayhost + i + 1, ' ');
				if (relayhost[i + j + 1]) { /*- if password is present */
					relayhost[i + j + 1] = 0;
					if (relayhost[i + 1] && relayhost[i + j + 2]) {	/*- both user and password are present */
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
			} else
				use_auth_smtp = 0;
		}
		/*-
		 * test.com:x.x.x.x:port username password
		 *         or
		 * domain:relay:port:penalty:max_tolerance username password
		 */
		i = str_chr(relayhost, ':');
		if (relayhost[i]) {
			if (relayhost[i + 1]) {
				if (relayhost[i + 1] == ':')
					port = PORT_SMTP;
				else			/*- port is present */
					scan_ulong(relayhost + i + 1, &port);
				relayhost[i] = 0;
				x = relayhost + i + 1;	/*- port */
				i = str_chr(x, ':'); /*- : before min_penalty */
				if (x[i]) {
					if (x[i + 1] != ':')	 /*- if penalty figure is present */
						scan_int(x + i + 1, &min_penalty);
					x = relayhost + i + 1; /*- min_penalty */
					i = str_chr(x, ':');
					if (x[i]) {
						if (x[i + 1] != ':')	 /*- if tolerance figure is present */
							scan_ulong(x + i + 1, &max_tolerance);
					}
					if (!min_penalty)
						flagtcpto = 0;
				}
			} else
				relayhost[i] = 0;
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
#if BATV
	if (batvok && smtp_sender.len && signkey.len) {
		if (!stralloc_0(&smtp_sender))
			temp_nomem();		/*- null terminate */
		smtp_sender.len--;
		i = str_rchr(*recips, '@');	/*- should check all recips, not just the first */
		j = str_rchr(smtp_sender.s, '@');
		if (!constmap(&mapnosign, *recips + i + 1, str_len(*recips + i + 1))
			&& !constmap(&mapnosigndoms, smtp_sender.s + j + 1, smtp_sender.len - (j + 1)))
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
							quit("DConnected to ", " but recpient failed DANE validation", 534, 1);
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
				if (!timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconnect)) {
					if (flagtcpto)	   /*- clear the error */
						tcpto_err(&ip.ix[i], 0, max_tolerance);
					partner = ip.ix[i];
#ifdef TLS
					partner_fqdn = ip.ix[i].fqdn;
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
						smtp();	/*- only returns when the next MX is to be tried */
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
	static char    *x = "$Id: qmail-remote.c,v 1.152 2022-08-22 22:23:34+05:30 Cprogrammer Exp mbhangui $";
	x = sccsidqrdigestmd5h;
	x++;
}

/*
 * $Log: qmail-remote.c,v $
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
 * use environment variable SMTPROUTEFILE, QMTPROUTEFILE, MORESMTPROUTESCDB to configure smtproutes, qmtproutes, moresmtproutes.cdb filenames
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
