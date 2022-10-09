/*
 * RCS log at bottom
 * $Id: smtpd.c,v 1.269 2022-10-09 23:00:33+05:30 Cprogrammer Exp mbhangui $
 */
#include <sig.h>
#include <stralloc.h>
#include <substdio.h>
#include <alloc.h>
#include <constmap.h>
#include <case.h>
#include <error.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <byte.h>
#include <env.h>
#include <now.h>
#include <open.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <commands.h>
#include <wait.h>
#include <fd.h>
#include <datetime.h>
#include <date822fmt.h>
#include <base64.h>
#include <getln.h>
#include "auto_control.h"
#include "auto_prefix.h"
#include "control.h"
#include "received.h"
#include "ipme.h"
#include "ip.h"
#include "qmail.h"
#include "rcpthosts.h"
#include "recipients.h"
#include "dns.h"
#include "etrn.h"
#include "greylist.h"
#include "variables.h"
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "indimail_stub.h"
#include "envrules.h"
#include "matchregex.h"
#include "tablematch.h"
#include "bodycheck.h"
#include "qregex.h"
#include "hasmysql.h"
#include "mail_acl.h"
#ifdef TLS
#include "tls.h"
#include "ssl_timeoutio.h"
#include "hastlsv1_1_server.h"
#include "hastlsv1_2_server.h"
#endif
#ifdef USE_SPF
#include "spf.h"
#endif
#ifdef HAS_MYSQL
#include "sqlmatch.h"
#endif

#ifdef BATV
#include <openssl/ssl.h>
#include <cdb.h>
#define BATVLEN 3	/* number of bytes */
#define BATVSTALE 7	/* accept for a week */
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <getEnvConfig.h>
#include <openssl/evp.h>
#else
#include <openssl/md5.h>
#endif
#endif
#ifdef SMTP_PLUGIN
#include "smtp_plugin.h"
#include <dlfcn.h>
#endif
#include <pwd.h>
#include "hassmtputf8.h"
#include "haslibgsasl.h"
#include <authmethods.h>
#include <hmac.h>
#ifdef HASLIBGSASL
#include <gsasl.h>
#include <get_scram_secrets.h>
#endif

#define MAXHOPS   100
#define SMTP_PORT  25
#define ODMR_PORT 366 /*- On Demand Mail Relay Protocol RFC 2645 */
#define SUBM_PORT 587 /*- Message Submission Port RFC 2476 */

#ifdef TLS
void            tls_init();
int             tls_verify();
void            tls_nogateway();
#endif
ssize_t         safewrite(int, char *, int);
ssize_t         saferead(int, char *, int);
static int      auth_login(char *);
static int      auth_plain(char *);
static int      auth_cram_md5();
static int      auth_cram_sha1();
static int      auth_cram_sha224();
static int      auth_cram_sha256();
static int      auth_cram_sha384();
static int      auth_cram_sha512();
static int      auth_cram_ripemd();
static int      auth_digest_md5();
#ifdef HASLIBGSASL
static int      auth_scram_sha1();
static int      auth_scram_sha1_plus();
static int      auth_scram_sha256();
static int      auth_scram_sha256_plus();
#if 0
static int      auth_scram_sha512();
static int      auth_scram_sha512_plus();
#endif
#endif
int             err_noauth();
int             err_noauthallowed();
int             addrrelay();
void            smtp_greet(char *);
int             atrn_queue(char *, char *);

typedef struct passwd  PASSWD;
#ifdef TLS
int             secure_auth = 0;
int             ssl_rfd = -1, ssl_wfd = -1;	/*- SSL_get_Xfd() are broken */
char           *servercert, *clientca, *clientcrl;
#endif
char           *revision = "$Revision: 1.269 $";
char           *protocol = "SMTP";
stralloc        proto = { 0 };
static stralloc Revision = { 0 };
static stralloc greeting = { 0 };

#ifdef USE_SPF
stralloc        spflocal = { 0 };
stralloc        spfguess = { 0 };
stralloc        spfexp = { 0 };

int             flagbarfspf;
unsigned int    spfbehavior = 0;
char           *spfbehaviorFn = 0;
unsigned int    spfipv6 = 0;
char           *spfipv6Fn = 0;
static stralloc spfbarfmsg = { 0 };
#endif
stralloc        helohost = { 0 };
stralloc        addr = { 0 }; /*- will be 0-terminated, if addrparse returns 1 */
static stralloc liphost = { 0 };
static stralloc mailfrom = { 0 };
static stralloc rcptto = { 0 };

static stralloc authin = { 0 };
static stralloc user = { 0 };
static stralloc pass = { 0 };
static stralloc resp = { 0 };
static stralloc slop = { 0 };
static stralloc authmethod = { 0 };

stralloc        locals = { 0 };
struct constmap maplocals;

#ifdef BATV
stralloc        signkey = { 0 };

char           *batvfn = 0;
stralloc        nosign = { 0 };
static stralloc libfn = { 0 };

int             batvok;
int             signkeystale = 7; /*- accept signkey for a week */
char           *signkeystaleFn = 0;
struct constmap mapnosign;
char            isbounce;
#endif

/*- SpaceNet - maex */
static char     strnum[FMT_ULONG];
static char     accept_buf[FMT_ULONG];

char           *remoteip, *remotehost, *remoteinfo, *local, *relayclient, *nodnscheck, *msgsize, *fakehelo;
char           *hostname, *bouncemail, *requireauth, *localip, *greyip;
#ifdef IPV6
char           *remoteip4;
#endif
char          **childargs;

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof (sserrbuf));
static char     upbuf[128];
static substdio ssup;

unsigned long   databytes = 0;
unsigned long   msg_size = 0;
unsigned long   BytesToOverflow = 0;

int             hasvirtual = 0;
int             liphostok = 0;
int             maxhops = MAXHOPS;
int             timeout = 1200;
int             authd = 0;
int             seenhelo = 0;
int             authenticated;
int             seenmail = 0;
int             setup_state = 0;
int             rcptcount;
int             dsn;
int             qregex = 0;
char           *qregexFn = 0;
int             rcpt_errcount = 0;
int             max_rcpt_errcount = 1;

struct qmail    qqt;

int             greetdelay = 0;
char           *greetdelayFn = 0;
char           *errStr = 0;
/*- badmailfrom */
int             bmfok = 0;
static stralloc bmf = { 0 };

struct constmap mapbmf;
int             bmpok = 0;
static stralloc bmp = { 0 };

char           *bmfFn = 0;
char           *bmfFnp = 0;
/*- blackholedrcpt */
int             bhrcpok = 0;
static stralloc bhrcp = { 0 };

struct constmap mapbhrcp;
int             bhbrpok = 0;
static stralloc bhbrp = { 0 };

char           *bhrcpFn = 0;
char           *bhrcpFnp = 0;
/*- BLACKHOLE Sender Check Variables */
int             bhfok = 0;
stralloc        bhf = { 0 };

struct constmap mapbhf;
int             bhpok = 0;
static stralloc bhp = { 0 };

char           *bhsndFn = 0;
char           *bhsndFnp = 0;
/*- badrcptto */
int             rcpok = 0;
static stralloc rcp = { 0 };

struct constmap maprcp;
int             brpok = 0;
static stralloc brp = { 0 };

char           *rcpFn = 0;
char           *rcpFnp = 0;
/*- accesslist */
int             acclistok = 0;
static stralloc acclist = { 0 };

char           *accFn = 0;
/*- RELAYCLIENT Check Variables */
int             relayclientsok = 0;
static stralloc relayclients = { 0 };

struct constmap maprelayclients;
/*- RELAYDOMAIN Check Variables */
int             relaydomainsok = 0;
static stralloc relaydomains = { 0 };

struct constmap maprelaydomains;
/*- RELAYMAILFROM Check Variables */
int             rmfok = 0;
static stralloc rmf = { 0 };

struct constmap maprmf;
/*- NODNSCHECK Check Variables */
int             nodnschecksok = 0;
static stralloc nodnschecks = { 0 };

struct constmap mapnodnschecks;
char           *nodnsFn;
/*- badip Check */
char           *dobadipcheck = (char *) 0;
char           *badipfn = (char *) 0;
int             briok = 0;
stralloc        bri = { 0 };

char           *badhostfn = (char *) 0;
stralloc        ipaddr = { 0 };

struct constmap mapbri;
/*- badhost Check */
char           *dobadhostcheck = (char *) 0;
int             brhok = 0;
stralloc        brh = { 0 };

struct constmap mapbrh;
/*- Helo Check */
char           *dohelocheck = (char *) 0;
char           *badhelofn = (char *) 0;
int             badhelook = 0;
stralloc        badhelo = { 0 };

struct constmap maphelo;
/*- authdomains */
int             chkdomok;
static stralloc chkdom = { 0 };

struct constmap mapchkdom;
/*- goodrcptto */
int             chkgrcptok = 0;
static stralloc grcpt = { 0 };

struct constmap mapgrcpt;
int             chkgrcptokp = 0;
static stralloc grcptp = { 0 };

char           *grcptFn = 0;
char           *grcptFnp = 0;
/*- SPAM Ingore Sender Check Variables */
int             spfok = 0;
stralloc        spf = { 0 };

struct constmap mapspf;
int             sppok = 0;
static stralloc spp = { 0 };

char           *spfFn = 0;
char           *spfFnp = 0;
/*- check recipients using inquery chkrcptdomains */
int             chkrcptok = 0;
static stralloc chkrcpt = { 0 };

struct constmap mapchkrcpt;
/*- TARPIT Check Variables */
int             tarpitcount = 0;
int             tarpitdelay = 5;
char           *tarpitcountFn = 0;
char           *tarpitdelayFn = 0;
/*- MAXRECIPIENTS Check Variable */
int             maxrcptcount = 0;
char           *maxrcptcountFn = 0;
/*- Russel Nelson's Virus Patch */
int             sigsok = 0;
char           *sigsFn = 0;
int             sigsok_orig = 0;
stralloc        sigs = { 0 };

char           *virus_desc;
int             bodyok = 0;
int             bodyok_orig = 0;
stralloc        body = { 0 };

char           *bodyFn = 0;
char           *content_desc;
#ifdef SMTP_PLUGIN
PLUGIN        **plug = (PLUGIN **) 0;
void          **plughandle;
int             plugin_count;
#endif
int             logfd = 255;
static int      old_client_bug = 0;

struct authcmd {
	char           *text;
	int             (*fun) ();
} authcmds[] = {
	{"login", auth_login},
	{"plain", auth_plain},
	{"cram-md5", auth_cram_md5},
	{"cram-sha1", auth_cram_sha1},
	{"cram-sha224", auth_cram_sha224},
	{"cram-sha256", auth_cram_sha256},
	{"cram-sha384", auth_cram_sha384},
	{"cram-sha512", auth_cram_sha512},
	{"cram-ripemd", auth_cram_ripemd},
	{"digest-md5", auth_digest_md5},
#ifdef HASLIBGSASL
	{"scram-sha-1", auth_scram_sha1},
	{"scram-sha-1-plus", auth_scram_sha1_plus},
	{"scram-sha-256", auth_scram_sha256},
	{"scram-sha-256-plus", auth_scram_sha256_plus},
#if 0
	{"scram-sha-512", auth_scram_sha512},
	{"scram-sha-512-plus", auth_scram_sha512_plus},
#endif
#endif
	{0, err_noauth}
};

/*- misc */
static stralloc sa = { 0 };
static stralloc domBuf = { 0 };
#ifdef SMTPUTF8
int             smtputf8 = 0, smtputf8_enable = 0;
#endif

int             smtp_port;
void           *phandle;
char           *no_help, *no_vrfy;
#ifdef HASLIBGSASL
static Gsasl   *gsasl_ctx;
typedef enum {
	tls_unique,
	tls_exporter,
} cb_type;
static char    *no_scram_sha1_plus, *no_scram_sha256_plus, *no_scram_sha512_plus;
#endif

extern char   **environ;

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
logerrpid()
{
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logerr("pid ");
	logerr(strnum);
	logerr(" from ");
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

#ifdef TLS
	if (ssl && fd == ssl_wfd)
		r = ssl_timeoutwrite(timeout, ssl_rfd, ssl_wfd, ssl, buf, len);
	else
		r = timeoutwrite(timeout, fd, buf, len);
#else
	r = timeoutwrite(timeout, fd, buf, len);
#endif
	if (r <= 0)
		_exit(1);
	return r;
}

void
flush()
{
	substdio_flush(&ssout);
}

void
flush_io()
{
	ssin.p = 0;
	substdio_flush(&ssout);
}

void
out(char *s)
{
	substdio_puts(&ssout, s);
}

void
die_nohelofqdn(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" non-FQDN HELO: ");
	logerr(arg);
	logerrf("\n");
	out("451 unable to accept non-FQDN HELO (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_localhelo(char *l, char *lip, char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" invalid HELO greeting: HELO <");
	logerr(arg);
	logerr("> for local ");
	logerr(l);
	logerr(", ");
	logerr(lip);
	logerrf("\n");
	out("451 invalid HELO greeting for local (#4.3.0)\r\n");
	flush();
}

void
err_badhelo(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" Invalid HELO greeting: HELO <");
	logerr(arg1);
	logerr("> FQDN <");
	logerr(arg2);
	logerrf(">\n");
	out("553 sorry, your HELO/EHLO greeting is in my badhelo list (#5.7.1)\r\n");
	flush();
#ifdef QUITASAP
	_exit(1);
#endif
}

void
die_lcmd()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" command t00 long!\n");
	out("553 sorry, the given command is t00 long! (#5.5.2)\r\n");
	flush();
	_exit(1);
}

void
die_read(char *str, char *err)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" read error");
	if (str) {
		logerr(": ");
		logerr(str);
	}
	if (errno) {
		logerr(": ");
		logerr(error_str(errno));
	}
	if (err) {
		logerr(": ");
		logerr(err);
	}
	logerrf("\n");
	out("451 Requested action aborted: read error (#4.4.2)\r\n");
	flush();
	_exit(1);
}

void
die_alarm()
{
	out("451 Requested action aborted: timeout (#4.4.2)\r\n");
	flush();
	_exit(1);
}

void
die_regex()
{
	out("451 Requested action aborted: regex compilation failed (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_nomem()
{
	out("451 Requested action aborted: out of memory (#4.3.0)\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerrf(" out of memory\n");
	_exit(1);
}

void
die_custom(char *arg)
{
	out("451 ");
	out(arg);
	out(" (#4.3.0)\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" ");
	logerr(arg);
	logerrf("\n");
	_exit(1);
}

void
die_control()
{
	out("451 Requested action aborted: unable to read controls (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_ipme()
{
	out("451 Requested action aborted: unable to figure out my IP addresses (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_plugin(char *arg1, char *arg2, char *arg3, char *arg4)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(": ");
	out("451 ");
	if (arg1) {
		logerr(arg1);
		out(arg1);
	}
	if (arg2) {
		logerr(arg2);
		out(arg2);
	}
	if (arg3) {
		logerr(arg3);
		out(arg3);
	}
	if (arg4) {
		logerr(arg4);
		out(arg4);
	}
	logerrf("\n");
	out(" (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_logfilter()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" unable create temporary files: ");
	logerr(error_str(errno));
	logerrf("\n");
	out("451 Requested action aborted: unable to create temporary files (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_addressmatch(char *errstr, char *fn)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" address_match: ");
	logerr(fn);
	logerr(": ");
	logerr(errstr);
	logerrf("\n");
	out("451 Requested action aborted: local system failure (#4.3.0)\r\n");
	flush();
}

void
straynewline()
{
	out("451 Requested action aborted: Bare LF received. (#4.6.0)\r\n");
	flush();
	_exit(1);
}

int
addrallowed(char *rcpt)
{
	int             r;

	if ((r = rcpthosts(rcpt, str_len(rcpt), 0)) == -1)
		die_control();
#ifdef TLS
	if (r == 0 && tls_verify())
		r = -2;
#endif
	return r;
}

void
log_fifo(char *arg1, char *arg2, unsigned long size, stralloc *line)
{
	int             logfifo, match;
	char           *fifo_name;
	struct stat     statbuf;
	static char     fifobuf[256], inbuf[1024];
	static substdio logfifo_in;
	static substdio logfifo_out;

	if (!env_get("SPAMFILTER"))
		return;
	fifo_name = env_get("LOGFILTER");
	if (!fifo_name || !*fifo_name)
		return;
	if (*fifo_name != '/')
		return;
	if ((logfifo = open(fifo_name, O_NDELAY | O_WRONLY)) == -1) {
		if (errno == ENXIO)
			return;
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerr(" fifo ");
		logerr(fifo_name);
		logerr(": ");
		logerr(error_str(errno));
		logerrf("\n");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		flush();
		_exit(1);
	}
	/*-
	 * write the SMTP transaction line to LOGFILTER fifo. All lines written
	 * to this fifo will be read by the qmail-cat spamlogger service
	 */
	substdio_fdbuf(&logfifo_out, write, logfifo, fifobuf, sizeof (fifobuf));
	if (substdio_puts(&logfifo_out, "qmail-smtpd: ") == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, "pid ") == -1) {
		close(logfifo);
		return;
	}
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (substdio_puts(&logfifo_out, strnum) == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, " MAIL from <") == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, arg1) == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, "> RCPT <") == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, arg2) == -1) {
		close(logfifo);
		return;
	}
	if (substdio_puts(&logfifo_out, "> Size: ") == -1) {
		close(logfifo);
		return;
	}
	strnum[fmt_ulong(strnum, msg_size)] = 0;
	if (substdio_puts(&logfifo_out, strnum) == -1) {
		close(logfifo);
		return;
	}
	/*-
	 * Read X-Bogosity line from bogofilter on logfd. logfd would have already
	 * been opened before qmail_open() by create_logfilter() function
	 */
	if (!fstat(logfd, &statbuf) && statbuf.st_size > 0 && !lseek(logfd, 0, SEEK_SET)) {
		if (substdio_puts(&logfifo_out, " ") == -1) {
			close(logfifo);
			close(logfd);
			return;
		}
		substdio_fdbuf(&logfifo_in, read, logfd, inbuf, sizeof (inbuf));
		if (getln(&logfifo_in, line, &match, '\n') == -1) {
			logerr("qmail-smtpd: read error: ");
			logerr(error_str(errno));
			logerrf("\n");
			close(logfd);
			return;
		}
		close(logfd);
		if (!stralloc_0(line))
			die_nomem();
		if (line->len) {
			if (substdio_puts(&logfifo_out, line->s) == -1) {
				logerr("qmail-smtpd: write error: ");
				logerr(error_str(errno));
				logerrf("\n");
			}
		}
	}
	if (substdio_puts(&logfifo_out, "\n") == -1) {
		logerr("qmail-smtpd: write error: ");
		logerr(error_str(errno));
		logerrf("\n");
	}
	if (substdio_flush(&logfifo_out) == -1) {
		close(logfifo);
		return;
	}
	close(logfifo);
	return;
}

void
log_trans(char *r_ip, char *mfrom, char *recipients, int rcpt_len, char *authuser, int notify)
{
	char           *ptr;
	int             idx;
	static stralloc tmpLine = { 0 };

	tmpLine.len = 0;
	for (ptr = recipients + 1, idx = 0; idx < rcpt_len; idx++) {
		if (!recipients[idx]) {
			/*- write data to spamlogger and get X-Bogosity line in tmpLine */
			if (!notify)
				log_fifo(mfrom, ptr, msg_size, &tmpLine);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(r_ip);
			if (!notify) {
				logerr(" HELO <");
				logerr(helohost.s);
				logerr("> ");
			} else
				logerr(" NOTIFY: ");
			logerr("MAIL from <");
			logerr(mfrom);
			logerr("> RCPT <");
			logerr(ptr);
			if (!notify) {
				logerr("> AUTH <");
				if (authuser && *authuser) {
					logerr(authuser);
					logerr(": AUTH ");
					logerr(get_authmethod(authd));
				}
				if (addrallowed(ptr)) {
					if (authuser && *authuser)
						logerr(": ");
					logerr("local-rcpt");
				} else
				if (!authuser || !*authuser)
					logerr("auth-ip/pop");
			}
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
#ifdef TLS
			logerr(" TLS=");
			logerr(ssl ? SSL_get_version(ssl) : "No");
#endif
			if (!notify && tmpLine.len) {
				logerr(" ");
				logerr(tmpLine.s);
			}
			logerr("\n");
			ptr = recipients + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
err_queue(char *r_ip, char *mfrom, char *recipients, int rcpt_len, char *authuser, char *qqx, int permanent, unsigned long qp)
{
	char           *ptr;
	int             idx;
	char            size[FMT_ULONG];
	static stralloc tmpLine = { 0 };

	tmpLine.len = 0;
	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	size[fmt_ulong(size, msg_size)] = 0;
	for (ptr = recipients + 1, idx = 0; idx < rcpt_len; idx++) {
		if (!recipients[idx]) {
			/*- write data to spamlogger */
			log_fifo(mfrom, ptr, msg_size, &tmpLine);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(r_ip);
			logerr(" ");
			logerr(qqx);
			if (permanent)
				logerr(" (permanent): ");
			else
				logerr(" (temporary): ");
			logerr("HELO <");
			logerr(helohost.s);
			logerr("> MAIL from <");
			logerr(mfrom);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> AUTH <");
			if (authuser && *authuser) {
				logerr(authuser);
				logerr(": AUTH ");
				logerr(get_authmethod(authd));
			}
			if (addrallowed(ptr)) {
				if (authuser && *authuser)
					logerr(": ");
				logerr("local-rcpt");
			} else
			if (!authuser || !*authuser)
				logerr("auth-ip/pop");
			logerr("> Size: ");
			logerr(size);
			if (tmpLine.len) {
				logerr(" ");
				logerr(tmpLine.s); /*- X-Bogosity line */
			}
#ifdef TLS
			logerr(" TLS=");
			logerr(ssl ? SSL_get_version(ssl) : "No");
#endif
			logerr(" qp ");
			logerr(accept_buf);
			logerr("\n");
			ptr = recipients + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
msg_notify()
{
	unsigned long   qp;
	char           *qqx;
	char            buf[DATE822FMT];
	struct datetime dt;

	if (qmail_open(&qqt) == -1) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerrf(" qqt failure");
		return;
	}
	qp = qmail_qp(&qqt); /*- pid of queue process */
	if (proto.len) {
		if (!stralloc_0(&proto))
			die_nomem();
		protocol = proto.s;
	}
	datetime_tai(&dt, now());
	received(&qqt, (char *) protocol, local, remoteip,
			str_diff(remotehost, "unknown") ? remotehost : 0, remoteinfo, fakehelo);
	strnum[fmt_ulong(strnum, msg_size)] = 0;
	qmail_puts(&qqt, "X-size-Notification: ");
	qmail_puts(&qqt, "size=");
	qmail_puts(&qqt, strnum);
	qmail_puts(&qqt, ",");
	qmail_put(&qqt, buf, date822fmt(buf, &dt));
	qmail_puts(&qqt, "To: do-not-reply\nFrom: ");
	qmail_put(&qqt, mailfrom.s, mailfrom.len);
	qmail_puts(&qqt, "\nSubject: Notification Message size ");
	qmail_puts(&qqt, strnum);
	qmail_puts(&qqt, " exceeds data limit\n");
	qmail_puts(&qqt, "Date: ");
	qmail_put(&qqt, buf, date822fmt(buf, &dt));
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);
	qqx = qmail_close(&qqt);
	if (!*qqx) { /*- mail is now in queue */
		log_trans(remoteip, mailfrom.s, rcptto.s, rcptto.len, 0, 1);
		return;
	}
	err_queue(remoteip, mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0, qqx + 1, *qqx == 'D', qp);
}

void
err_smf()
{
	out("451 Requested action aborted: DNS temporary failure (#4.4.3)\r\n");
	flush();
}

void
err_size(char *rip, char *mfrom, char *rcpt, int len)
{
	int             idx;
	char           *ptr;

	out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n");
	flush();
	if (env_get("DATABYTES_NOTIFY"))
		msg_notify();
	for (ptr = rcpt + 1, idx = 0; idx < len; idx++) {
		if (!rcpt[idx]) {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(rip);
			logerr(" data size exceeded: MAIL from <");
			logerr(mfrom);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
			logerr("\n");
			ptr = rcpt + idx + 2;
		}
	}
}

void
err_hops()
{
	out("554 too many hops, this message is looping (#5.4.6)\r\n");
	flush();
}

void
err_hmf(char *arg1, char *arg2, int arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg3)
		logerr(" Non-existing DNS_MX: MAIL ");
	else
		logerr(" Non-existing DNS_MX: HELO ");
	logerr(arg2);
	logerrf("\n");
	if (arg3)
		out("553 Bad sender's system address (#5.1.8)\r\n");
	else
		out("553 sorry, helo domain must exist (#5.1.8)\r\n");
	flush();
}

void
err_nogateway(char *arg1, char *arg2, char *arg3, int flag)
{
	char           *x;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RELAY client: MAIL from <");
	logerr(arg2);
	if (arg3 && *arg3) {
		logerr("> RCPT <");
		logerr(arg3);
	}
	logerr(">");
	if (authd) {
		logerr(", Auth <");
		logerr(remoteinfo);
		logerr(">");
		x = env_get("MASQUERADE");
		if (x && *x) {
			logerr(", MASQUERADE <");
			logerr(x);
			logerr(">");
		}
	}
	logerrf("\n");
	if (flag)
		out("553 sorry, this MTA does not accept masquerading/forging ");
	else
		out("553 sorry, that domain isn't allowed to be relayed thru this MTA without authentication ");
	if (authd) {
		out(", auth <");
		out(remoteinfo);
		out("> ");
	}
#ifdef TLS
	if (ssl)
		tls_nogateway();
#endif
	out("#5.7.1\r\n");
	flush();
}

void
err_badbounce()
{
	out("553 sorry, bounce messages should have a single envelope recipient (#5.7.1)\r\n");
	flush();
}

void
err_bmf(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid SENDER address: MAIL from <");
	logerr(arg2);
	logerrf(">\n");
	out("553 sorry, your envelope sender has been denied (#5.7.1)\r\n");
	flush();
}

#ifdef USE_SPF
void
err_spf()
{
	int             i, j;

	for (i = 0; i < spfbarfmsg.len; i = j + 1) {
		j = byte_chr(spfbarfmsg.s + i, spfbarfmsg.len - i, '\n') + i;
		if (j < spfbarfmsg.len) {
			out("550-");
			spfbarfmsg.s[j] = 0;
			out(spfbarfmsg.s);
			spfbarfmsg.s[j] = '\n';
			out("\r\n");
		} else {
			out("550 ");
			out(spfbarfmsg.s);
			out(" (#5.7.1)\r\n");
		}
	}
	flush();
}
#endif

void
err_hostaccess(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid SENDER host IP address: MAIL from <");
	logerr(arg2);
	logerrf(">\n");
	out("553 sorry, your host has been denied (#5.7.1)\r\n");
	flush();
}

void
log_virus(char *arg1, char *arg2, char *arg3, char *arg4, int len, int blackhole)
{
	int             idx;
	char           *ptr;

	for (ptr = arg4 + 1, idx = 0; idx < len; idx++) {
		if (!arg4[idx]) {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(arg1);
			logerr(" virus/banned content: ");
			logerr(arg2);
			logerr(": MAIL from <");
			logerr(arg3);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
			logerr("\n");
			ptr = arg4 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
	if (!blackhole) {
		out("552-we don't accept email with the below content (#5.3.4)\r\n");
		out("552 Further Information: ");
		out(arg2);
		out("\r\n");
		flush();
	}
}

void
err_acl(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	out("553 sorry, sites access list denies transaction (#5.7.1)\r\n");
	flush();
	return;
}

void
err_rcp(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	out("553 sorry, your envelope recipient has been denied (#5.7.1)\r\n");
	flush();
	return;
}

void
smtp_badip(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" BAD IP client\n");
	sleep(5);
	out("421 sorry, your IP (");
	out(arg);
	out(") is temporarily denied (#4.7.1)\r\n");
	flush();
	return;
}

void
smtp_badhost(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" BAD HOST ");
	logerr(remotehost);
	logerrf("\n");
	sleep(5);
	out("553 sorry, your host (");
	out(remotehost);
	out(") has been denied (#5.7.1)\r\n");
	flush();
	return;
}

void
smtp_relayreject(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" OPEN RELAY client\n");
	sleep(5);
	out("553 No mail accepted from an open relay (");
	out(remoteip);
	out("); check your server configs (#5.7.1)\r\n");
	flush();
	return;
}

void
smtp_paranoid(char *arg)
{
	char           *ptr;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" PTR (reverse DNS) record points to wrong hostname\n");
	sleep(5);
	ptr = env_get("TCPPARANOID");
	out("553 sorry, your IP address (");
	out(arg);
	if (ptr && *ptr) {
		out(") PTR (reverse DNS) record points to wrong hostname ");
		out(ptr);
		out(" (#5.7.1)\r\n");
	} else
		out(") PTR (reverse DNS) record points to wrong hostname (#5.7.1)\r\n");
	flush();
	return;
}

void
smtp_ptr(char *arg)
{
	char           *ptr;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" unable to obain PTR (reverse DNS) record\n");
	sleep(5);
	ptr = env_get("REQPTR");
	out("553 ");
	if (*ptr) {
		out(ptr);
		out(": from ");
		out(arg);
		out(": (#5.7.1)\r\n");
	} else {
		out(" Sorry, no PTR (reverse DNS) record for (");
		out(arg);
		out(") (#5.7.1)\r\n");
	}
	flush();
	return;
}

void
log_rules(char *arg1, char *arg2, char *arg3, int arg4, int arg5)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(arg5 == 0 ? " Setting EnvRule No " : " Setting DomainQueue Rule No ");
	strnum[fmt_ulong(strnum, arg4)] = 0;
	logerr(strnum);
	logerr(": MAIL from <");
	logerr(arg2);
	if (authd) {
		logerr("> AUTH ");
		logerr(get_authmethod(authd));
		logerr(" <");
		logerr(arg3);
	}
	logerrf(">\n");
}

void
err_relay()
{
	out("550 we don't relay (#5.7.1)\r\n");
	flush();
}

void
err_unimpl(char *arg)
{
	if (!case_diffs(arg, "unimplemented"))
		out("502 unimplemented (#5.5.1)\r\n");
	else
	if (!case_diffs(arg, "help"))
		out("502 disabled by the lord in her infinite wisdom (#5.5.1)\r\n");
	else {
		out("502 command ");
		out(arg);
		out(" not recognized (#5.5.2)\r\n");
	}
	flush();
}

void
err_syntax()
{
	out("555 syntax error in address (#5.1.3)\r\n");
	flush();
}

void
err_wantmail()
{
	out("503 MAIL first (#5.5.1)\r\n");
	flush();
}

void
err_wantrcpt()
{
	out("503 RCPT first (#5.5.1)\r\n");
	flush();
}

void
err_bhf(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Blackholed SENDER address: MAIL ");
	logerr(arg2);
	logerrf("\n");
	if (!env_put("NULLQUEUE=1"))
		die_nomem();
}

void
err_bhrcp(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Blackholed RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	if (!env_put("NULLQUEUE=1"))
		die_nomem();
}

void
err_maps(char *m, char *ip, char *from)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(ip);
	logerr(" Blackholed SENDER address: MAIL ");
	logerr(from);
	logerr(" Reason ");
	logerr(m);
	logerrf("\n");
	out("553 ");
	out(m);
	out(" (#5.7.1)\r\n");
	flush();
	_exit(1);
}

void
err_mrc(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Too many RECIPIENTS: MAIL <");
	logerr(arg2);
	logerr("> Last RCPT <");
	logerr(arg3);
	logerrf(">\n");
	out("557 sorry, too many recipients (#5.7.1)\r\n");
	flush();
}

void
smtp_noop(char *arg)
{
	if (arg && *arg) {
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		flush();
		return;
	}
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	out("250 ok\r\n");
	flush();
	return;
}

void
smtp_vrfy(char *arg)
{
	if (no_vrfy) {
		err_unimpl("unimplimented");
		return;
	}
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	out("252 Cannot VRFY user, but will accept message and attempt delivery (#2.7.0)\r\n");
	flush();
	return;
}

void
err_qqt(char *arg)
{
	out("451 Requested action aborted: qqt failure (#4.3.0)\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" qqt failure\n");
	return;
}

int
err_child()
{
	out("451 Requested action aborted: problem with child and I can't auth (#4.3.0)\r\n");
	flush();
	return -1;
}

void
err_library(char *arg)
{
	out("451 Requested action aborted: problem loading virtual domain library (#4.3.0)\r\n");
	flush();
	if (arg) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerr(" ");
		logerr(arg);
		logerrf("\n");
	}
	return;
}

int
err_fork()
{
	out("451 Requested action aborted: child won't start and I can't auth (#4.3.0)\r\n");
	flush();
	return -1;
}

int
err_pipe()
{
	out("451 Requested action aborted: unable to open pipe and I can't auth (#4.3.0)\r\n");
	flush();
	return -1;
}

int
err_write()
{
	out("451 Requested action aborted: unable to write pipe and I can't auth (#4.3.0)\r\n");
	flush();
	return -1;
}

void
err_authfailure(char *r_ip, char *authuser, int ret)
{
	static char     retstr[FMT_ULONG];
	int             i;

	strnum[fmt_ulong(retstr, ret > 0 ? ret : 0 - ret)] = 0;
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(r_ip);
	if (authuser) {
		logerr(" AUTH ");
		logerr(authuser);
	}
	logerr(" status=[");
	if (ret < 0)
		logerr("-");
	logerr(retstr);
	logerr("]");
	if (authmethod.len) {
		logerr(" AUTH ");
		i = authmethod.s[0];
		logerr(get_authmethod(i));
	} else
		logerr(" AUTH Unknown ");
#ifdef TLS
	logerr(" TLS=");
	logerr(ssl ? SSL_get_version(ssl) : "No");
#endif
	logerrf(" auth failure\n");
}

void
err_authinsecure(char *r_ip, int ret)
{
	static char     retstr[FMT_ULONG];

	strnum[fmt_ulong(retstr, ret > 0 ? ret : 0 - ret)] = 0;
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(r_ip);
	logerr(" status=[");
	if (ret < 0)
		logerr("-");
	logerr(retstr);
	logerr("]");
#ifdef TLS
	logerr(" TLS=");
	logerr(ssl ? SSL_get_version(ssl) : "No");
#endif
	logerrf(" auth failure\n");
}

void
err_authd()
{
	out("503 you're already authenticated (#5.5.0)\r\n");
	flush();
}

void
err_authrequired()
{
	out("530 authentication required (#5.7.1)\r\n");
	flush();
}

void
err_transaction(char *arg)
{
	out("503 no ");
	out(arg);
	out(" during mail transaction (#5.5.0)\r\n");
	flush();
}

int
err_noauth()
{
	out("504 auth type unimplemented (#5.5.1)\r\n");
	flush();
	return -1;
}

int
err_noauthallowed()
{
	out("530 auth type prohibited without TLS (#5.7.0)\r\n");
	flush();
	return -2;
}

int
err_authabrt()
{
	out("501 auth exchange cancelled (#5.0.0)\r\n");
	flush();
	return -1;
}

int
err_input()
{
	out("501 malformed auth input (#5.5.4)\r\n");
	flush();
	return -1;
}

void
err_mailbox(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg1);
	logerr("> RCPT <");
	logerr(arg2);
	logerr("> state <");
	logerr(arg3);
	logerrf(">\n");
	out("550 sorry, ");
	out(arg1);
	out(" mailbox <");
	out(arg2);
	out("> ");
	out(arg3);
	out("\r\n");
	flush();
	return;
}

void
err_rcpt_errcount(char *arg1, int count)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" Too many Invalid RECIPIENTS (");
	strnum[fmt_ulong(strnum, count)] = 0;
	logerr(strnum);
	logerr("): MAIL from <");
	logerr(arg1);
	logerrf(">\n");
	out("421 too many invalid addresses, goodbye (#4.7.1)\r\n");
	flush();
	return;
}

void
err_greytimeout()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerrf(" Timeout (no response from greylisting server)\n");
	out("451 greylist temporary failure - Timeout (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_grey_tmpfail(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" greylisting temporary failure: ");
	if (arg) {
		logerr(arg);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	out("451 greylist temporary failure (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_grey()
{
	char           *arg, *ptr;
	int             idx;

	arg = rcptto.s;
	for (ptr = arg + 1, idx = 0; idx < rcptto.len; idx++) {
		if (!arg[idx]) {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" HELO <");
			logerr(helohost.s);
			logerr("> MAIL from <");
			logerr(mailfrom.s);
			logerr("> RCPT <");
			logerr(ptr);
			logerrf(">\n");
			ptr = arg + idx + 2;
		}
	}
	logerr("greylist ");
	logerr(remoteip);
	logerr(" <");
	logerr(mailfrom.s);
	logerr("> to <");
	logerr(arg + 1);
	logerr(">");
	if (rcptcount > 1)
		logerr("...");			/* > 1 address sent for greylist check */
	logerr("\n");
	out("450 try again later (#4.3.0)\r\n");
	flush();
	return;
}

int             flagblackhole;
stralloc        Desc = { 0 };

int
sigscheck(stralloc *line, char **desc, int in_header)
{
	int             i, j, k, len, pos1, pos2, header_check, body_check;
	char           *ptr;

	*desc = "unknown";
	for (i = j = 0; i < sigs.len; i++) {
		header_check = body_check = flagblackhole = 0;
		if (!sigs.s[i]) {
			for (k = i, len = 0; sigs.s[k] != ':' && k > j; k--)
				len++;
			pos1 = pos2 = 0;
			if (sigs.s[k] == ':') {
				sigs.s[k] = 0;
				pos2 = k;
				for (ptr = sigs.s + k + 1; *ptr && isspace(*ptr); ptr++);
				if (!str_diffn(ptr, "-headerblackhole", 16))
					header_check = flagblackhole = 1;
				else
				if (!str_diffn(ptr, "-header", 7))
					header_check = 1;
				else
				if (!str_diffn(ptr, "-bodyblackhole", 14))
					body_check = flagblackhole = 1;
				else
				if (!str_diffn(ptr, "-body", 5))
					body_check = 1;
				if (header_check || flagblackhole || body_check) {
					for (; sigs.s[k] != ':' && k > j; k--)
						len++;
					if (sigs.s[k] == ':') {
						pos1 = k;
						sigs.s[k] = 0;
						if (!stralloc_copys(&Desc, sigs.s + pos1 + 1))
							die_nomem();
					} else {
						if (!stralloc_copys(&Desc, sigs.s + j))
							die_nomem();
						len -= (pos2 - k);
					}
				} else {
					if (!stralloc_copys(&Desc, sigs.s + pos2 + 1))
						die_nomem();
				}
			} else {
				if (!stralloc_copys(&Desc, sigs.s + j))
					die_nomem();
				len = 0; /*- handle signatures without comments */
			}
			if (pos1)
				sigs.s[pos1] = ':';
			if (pos2)
				sigs.s[pos2] = ':';
			if ((body_check && in_header) || (header_check && !in_header)) {
				j = i + 1;
				continue;
			}
			if ((i - j - len) < line->len && !str_diffn(line->s, sigs.s + j, i - j - len)) {
				if (!stralloc_0(&Desc))
					die_nomem();
				*desc = Desc.s;
				return 1;
			}
			j = i + 1;
		}		  /*- if (!sigs.s[i]) */
	} /*- for (;;) */
	return 0;
}

/*
 * This function returns
 *  0: If user not found
 * >0: if user is found
 */
int
check_recipient_cdb(char *rcpt)
{
	int             r;

	r = recipients(rcpt, str_len(rcpt));
	switch (r)
	{
	case -1:
		die_control();
		break;
	case -2:
		die_nomem();
		break;
	case 10:/*- recipient cdb does not exist */
		return 0;
	case -3:
	case 111:
		out("451 unable to check recipients (#4.3.2)\r\n");
		flush();
		logerr("qmail-smtpd: ");
		logerrpid();
		logerrf("recipients database error\n");
		_exit(1);
		/*- Not Reached */
	}
	return r;
}

/*
 * This function returns
 *  1: If user not found
 *  0: if user is found
 */
int
check_recipient_pwd(char *rcpt, int len)
{
	int             fd, match, i;
	char            inbuf[4096];
	static stralloc line = { 0 };
	substdio        pwss;

	if ((fd = open_read("/etc/passwd")) == -1) {
		out("451 Requested action aborted: unable to read passwd database (#4.3.0)\r\n");
		logerr("qmail-smtpd: ");
		logerrpid();
		logerrf("passwd database error\n");
		flush();
		_exit(1);
	}
	substdio_fdbuf(&pwss, read, fd, inbuf, sizeof (inbuf));
	for (;;) {
		if (getln(&pwss, &line, &match, '\n') == -1) {
			close(fd);
			die_read("/etc/passwd", 0);
		}
		if (!line.len) {
			close(fd);
			return 1;
		}
		i = str_chr(line.s, ':');
		if (line.s[i]) {
			line.s[i] = 0;
			if (!str_diffn(line.s, rcpt, len)) {
				close(fd);
				return 0;
			}
		}
	}
	close(fd);
	return 1;
}

char           *
load_virtual()
{
	char           *ptr;

	if (!hasvirtual) {
		err_library("libindimail.so not loaded");
		return ((char *) NULL);
	}
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir))
				die_nomem();
			if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/"))
				die_nomem();
			if (!stralloc_catb(&libfn, "libindimail", 11) ||
					!stralloc_0(&libfn))
				die_nomem();
		}
		return libfn.s;
	}
	return ptr;
}

/*
 * This function returns
 *  0: User is fine
 *  1: User is not present
 *  2: User is Inactive
 *  3: User is overquota
 * -1: System Error
 */
int
check_recipient_sql(char *rcpt, int len)
{
	char           *ptr, *errstr;
	void           *(*inquery) (char, char *, char *);

	if (!(ptr = load_virtual()))
		return -1;
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &errstr))) {
		err_library(errstr);
		return -1;
	}
	if ((ptr = (*inquery) (USER_QUERY, rcpt, 0))) {
		if (*ptr == 4)	   /*- allow aliases */
			return (0);
		return (*ptr);
	}
	out("451 Requested action aborted: temporary database error (#4.3.2)\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerrf("sql database error\n");
	_exit(1);
	/*- Not Reached */
	return (0);
}

int
dnscheck(char *address, int len, int paranoid)
{
	ipalloc         ia = { 0 };
	unsigned int    random;
	int             j;

	if (str_equal(address, "#@[]") || !len)
		return (0);
	if (nodnschecksok) {
		if (constmap(&mapnodnschecks, address, len))
			return 0;
		if ((j = byte_rchr(address, len, '@')) < (len - 1)) {
			if (constmap(&mapnodnschecks, address + j, len - j))
				return 0;
		}
	}
	random = now() + (getpid() << 16);
	if ((j = byte_rchr(address, len, '@')) < (len - 1)) {
		if (!stralloc_copys(&sa, address + j + 1))
			die_nomem();
		dns_init(0);
		if ((j = dns_mxip(&ia, &sa, random)) < 0)
			return j;
	} else
	if (paranoid) {
		if (!stralloc_copys(&sa, address))
			die_nomem();
		dns_init(0);
		if ((j = dns_mxip(&ia, &sa, random)) < 0)
			return j;
	} else
		return (DNS_HARD);
	return (0);
}

void
log_etrn(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg3) {
		logerr(" ");
		logerr(arg3);
	}
	if (arg2) {
		logerr(" ETRN ");
		logerr(arg2);
	}
	logerrf("\n");
}

void
log_atrn(char *arg1, char *arg2, char *arg3, char *arg4)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg2) {
		logerr(" ");
		logerr(arg2);
	}
	if (arg3) {
		logerr(" ATRN ");
		logerr(arg3);
	}
	if (arg4) {
		logerr(": ");
		logerr(arg4);
	}
	logerrf("\n");
}

void
sigterm()
{
	smtp_greet("421 ");
	out(" Service not available, closing tranmission channel (#4.3.2)\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" going down on SIGTERM");
	logerrf("\n");
	_exit(1);
}

void
esmtp_print()
{
	char           *ptr;
	char            buf[DATE822FMT];
	int             i;
	struct datetime dt;

	substdio_puts(&ssout, " (NO UCE) ESMTP IndiMail ");
	for (ptr = (revision + 11); *ptr; ptr++) {
		if (*ptr == ' ') {
			out(" ");
			break;
		}
		substdio_put(&ssout, ptr, 1);
	}
	datetime_tai(&dt, now());
	i = date822fmt(buf, &dt);
	buf[i - 1] = 0;
	out(buf);
	flush();
	return;
}

void
smtp_greet(char *code)
{
	int             i, j, esmtp;

	if (code[3] != ' ') {
		substdio_puts(&ssout, code);
		substdio_puts(&ssout, greeting.s);
		return;
	}
	if (code[0] == '2' && code[1] == '2' && code[2] == '0')
		esmtp = 1;
	else
		esmtp = 0;
	for (i = 0, j = 0; i < greeting.len - 1; i++) {
		if (greeting.s[i] == '\0') {
			substdio_put(&ssout, code, 3);
			substdio_puts(&ssout, "-");
			substdio_put(&ssout, greeting.s + j, i - j);
			if (esmtp) {
				esmtp_print();
				esmtp = 0;
			}
			substdio_puts(&ssout, "\r\n");
			j = i + 1;
		}
	}
	substdio_puts(&ssout, code);
	substdio_put(&ssout, greeting.s + j, greeting.len - 1 - j);
	if (esmtp) {
		esmtp_print();
		esmtp = 0;
	}
}

void
smtp_help(char *arg)
{
	char           *ptr;

	if (no_help) {
		err_unimpl("help");
		return;
	}
	ptr = revision + 11;
	if (*ptr) {
		out("214-This is IndiMail SMTP Version ");
		for (; *ptr; ptr++) {
			if (*ptr == ' ')
				break;
			substdio_put(&ssout, ptr, 1);
		}
	}
	out("\r\n");
	out("214-This server supports the following commands:\r\n");
	switch (smtp_port)
	{
	case ODMR_PORT:/*- RFC 2645 */
		if (hasvirtual)
			out("214 HELO EHLO AUTH ATRN HELP QUIT\r\n");
		else { /*- since we don't have ATRN mechanism, behave like any other non-special port */
			out("214 HELO EHLO RSET NOOP MAIL RCPT DATA ");
			if (hostname && *hostname && childargs && *childargs)
				out("AUTH ");
			out(no_vrfy ? "ETRN HELP QUIT\r\n" : "VRFY ETRN HELP QUIT\r\n");
		}
		break;
	case SUBM_PORT:/*- RFC 2476 */
		out("214 HELO EHLO RSET NOOP MAIL RCPT DATA ");
		if (hostname && *hostname && childargs && *childargs)
			out("AUTH ");
		out(no_vrfy ? "HELP QUIT\r\n" : "VRFY HELP QUIT\r\n");
		break;
	default:
		out("214 HELO EHLO RSET NOOP MAIL RCPT DATA ");
		if (hostname && *hostname && childargs && *childargs)
			out("AUTH ");
		out(no_vrfy ? "ETRN HELP QUIT\r\n" : "VRFY ETRN HELP QUIT\r\n");
		break;
	}
	flush();
	return;
}

void
smtp_quit(char *arg)
{
#ifdef SMTP_PLUGIN
	int             i;
#endif

	smtp_greet("221 ");
	out(" closing connection\r\n");
	flush();
	if (phandle)
		closeLibrary(&phandle);
#ifdef SMTP_PLUGIN
	for (i = 0; plughandle && i < plugin_count; i++) {
		if (plughandle[i])
			dlclose(plughandle[i]);
	}
#endif
	_exit(0);
}

int
badipcheck(char *arg)
{
	/*- badip */
	if (!stralloc_copys(&ipaddr, arg) ||
			!stralloc_0(&ipaddr))
		die_nomem();
	switch (address_match((badipfn && *badipfn) ? badipfn : "badip", &ipaddr, briok ? &bri : 0, briok ? &mapbri : 0, 0, &errStr))
	{
	case 1:
		return (1);
	case 0:
		return (0);
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr, "badip");
		return (-1);
	}
	return (0);
}

/*
 * $TCPREMOTEHOST check against regular expressions depending on $RELAYCLIENT:
 * domain-based blacklist/right-hand-side blackhole list (RHSBL)
 *
 * Author: Jörg Backschues
 * switch (address_match(0, &addr, brhok ? &brh : 0, brhok ? &mapbrh : 0, 0, &errStr))
 */
int
badhostcheck()
{
	int             i = 0, j = 0, x = 0, negate = 0;
	static stralloc curregex = { 0 };

	curregex.len = 0;
	while (j < brh.len) {
		i = j;
		while ((brh.s[i] != '\0') && (i < brh.len))
			i++;
		if (brh.s[j] == '!') {
			negate = 1;
			j++;
		}
		if (!stralloc_copyb(&curregex, brh.s + j, (i - j)) ||
				!stralloc_0(&curregex))
			die_nomem();
		x = matchregex(remotehost, curregex.s, 0);
		if ((negate) && (x == 0))
			return 1;
		if (!(negate) && (x > 0))
			return 1;
		j = i + 1;
		negate = 0;
	}
	return 0;
}

void
dohelo(char *arg)
{
	int             i;

	seenhelo = 0;
	if (!stralloc_copys(&helohost, arg) ||
			!stralloc_0(&helohost))
		die_nomem();
	if (!relayclient) { /*- turn on helo check if user not authenticated */
		if (env_get("ENFORCE_FQDN_HELO")) {
			i = str_chr(arg, '.');
			if (!arg[i])
				die_nohelofqdn(arg);
		}
	}
	/*- badhelo */
	if (dohelocheck) {
		if (!case_diffs(localip, remoteip) && case_diffs(local, helohost.s) && case_diffs(localip, helohost.s))
			err_localhelo(local, localip, arg);
		switch (address_match
				((badhelofn
				  && *badhelofn) ? badhelofn : "badhelo", &helohost, badhelook ? &badhelo : 0, badhelook ? &maphelo : 0, 0,
				 &errStr))
		{
		case 1:
			err_badhelo(helohost.s, remotehost);
			return;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, "badhelo");
			return;
		}
	}
	if ((fakehelo = case_diffs(remotehost, helohost.s) ? helohost.s : 0)) {
		if (dohelocheck && !nodnscheck) {
			switch (dnscheck(helohost.s, helohost.len - 1, 1))
			{
			case DNS_HARD:
				err_hmf(remoteip, arg, 0);
				return;
			case DNS_SOFT:
				err_smf();
				return;
			case DNS_MEM:
				die_nomem();
			}
		}
		seenhelo = 1;
		return;
	}
	seenhelo = 1;
}

void
greetdelay_check(int delay)
{
	int             r;

	if (delay > 0) {
		sleep(delay);
		return;
	} else
		delay = -delay;
	if ((r = timeoutread(delay, 0, ssinbuf, sizeof ssinbuf)) == -1) {
		if (errno == error_timeout)
			return;				/* Timeout ==> No early talking */
	}
	if (r <= 0) {
		if (!r)
			errno = 0;
		die_read(!r ? "client dropped connection" : 0, 0);
	}
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerrf(" SMTP Protocol violation - Early Talking\n");
	out("554 SMTP protocol violation. Polite people say hello after the server greets them (#5.7.1)\r\n");
	flush();
	_exit(1);
}


/*
 * This function gets called after envrules
 * Set any environment variables again using envrules
 */
void
databytes_setup()
{
	char           *x;

	if (!(x = env_get("DATABYTES"))) {
		if (control_readulong(&databytes, "databytes") == -1)
			die_control();
	} else
		scan_ulong(x, &databytes);
}

void
open_control_once(int *open_flag, int *open_flagp, char **fn, char **fn_p,
	char *envstr, char *envstr_p, char *cfn, char *cfn_p,
	stralloc *sfn, struct constmap *mapvar, stralloc *sfn_p)
{
	char           *x;

	if (open_flag && envstr && (x = env_get(envstr))) {
		if (fn && *fn && *x && !str_diff(x, *fn))
			return;
		*open_flag = 0;
		if (fn && *fn)
			*fn = 0;
	}
	if (open_flagp && envstr_p && (x = env_get(envstr_p))) {
		if (fn_p && *fn_p && !str_diff(x, *fn_p))
			return;
		*open_flagp = 0;
		if (fn_p && *fn_p)
			*fn_p = 0;
	}
	/*- open the control file e.g. badrcptto */
	if ((open_flag && !*open_flag && fn && !(*fn))) {
		x = (envstr ? env_get(envstr) : 0);
		if ((*open_flag = control_readfile(sfn, *fn = (x && *x ? x : cfn), 0)) == -1)
			die_control();
		if (*open_flag && mapvar && !constmap_init(mapvar, sfn->s, sfn->len, 0))
			die_nomem();
	}
	/*- open the control file having patterns e.g. badrcptpatterns */
	if ((open_flagp && !*open_flagp && fn_p && !(*fn_p))) {
		x = (envstr_p ? env_get(envstr_p) : 0);
		if ((*open_flagp = control_readfile(sfn_p, *fn_p = (x && *x ? x : cfn_p), 0)) == -1)
			die_control();
	}
}

void
open_control_once_int(int *val, char **fn, char *envstr, char *cfn, int neg_allowed)
{
	char           *x;

	if (envstr && (x = env_get(envstr)))
		scan_int(x, val);
	else {
		if (fn && !(*fn) && control_readint(val, cfn) == -1)
			die_control();
		if (fn && cfn)
			*fn = cfn;
		if (!neg_allowed) {
			if (*val < 0)
				*val = 0;
		}
	}
	return;
}

void
open_control_files()
{
	char           *x;

	/*- BADMAILFROM */
	open_control_once(&bmfok, &bmpok, &bmfFn, &bmfFnp,
		"BADMAILFROM", "BADMAILPATTERNS", "badmailfrom", "badmailpatterns", &bmf, &mapbmf, &bmp);
	/*- BLACKHOLE Sender Patch - include Control file */
	open_control_once(&bhfok, &bhpok, &bhsndFn, &bhsndFnp,
		"BLACKHOLEDSENDER", "BLACKHOLEDPATTERNS", "blackholedsender", "blackholedpatterns", &bhf, &mapbhf, &bhp);
	/*- BLACKHOLE RECIPIENT Patch - include Control file */
	open_control_once(&bhrcpok, &bhbrpok, &bhrcpFn, &bhrcpFnp,
		"BLACKHOLERCPT", "BLACKHOLERCPTPATTERNS", "blackholercpt", "blackholercptpatterns", &bhrcp, &mapbhrcp, &bhbrp);
	/*- BADRECIPIENT Patch - include Control file */
	open_control_once(&rcpok, &brpok, &rcpFn, &rcpFnp,
		"BADRCPTTO", "BADRCPTPATTERNS", "badrcptto", "badrcptpatterns", &rcp, &maprcp, &brp);
	/*- goodrcptto */
	open_control_once(&chkgrcptok, &chkgrcptokp, &grcptFn, &grcptFnp,
		"GOODRCPTTO", "GOODRCPTPATTERNS", "goodrcptto", "goodrcptpatterns", &grcpt, &mapgrcpt, &grcptp);
	/*- Spam Ignore Patch - include Control file */
	if (env_get("SPAMFILTER"))
		open_control_once(&spfok, &sppok, &spfFn, &spfFnp,
			"SPAMIGNORE", "SPAMIGNOREPATTERNS", "spamignore", "spamignorepatterns", &spf, &mapspf, &spp);
	/*-
	 * DNSCHECK Patch - include Control file
	 * Look up "MAIL from:" addresses to skip for DNS check in control/nodnscheck.
	 */
	if (!(nodnscheck = env_get("NODNSCHECK")))
		open_control_once(&nodnschecksok, 0, &nodnsFn, 0, 0, 0, "nodnscheck", 0, &nodnschecks, &mapnodnschecks, 0);
	/*-
	 * Enable badip if
	 * BADIPCHECK is defined (default control file badip)
	 * or
	 * BADIP (control file defined by BADIP env variable)
	 * is defined
	 */
	if ((dobadipcheck = (env_get("BADIPCHECK") ? "" : env_get("BADIP"))))
		open_control_once(&briok, 0, &badipfn, 0, "BADIP", 0, "badip", 0, &bri, &mapbri, 0);
	/*-
	 * Enable badhost if
	 * BADHOSTCHECK is defined (default control file badhost)
	 * or
	 * BADHOST (control file defined by BADHOST env variable)
	 * is defined
	 */
	if ((dobadhostcheck = (env_get("BADHOSTCHECK") ? "" : env_get("BADHOST"))))
		open_control_once(&brhok, 0, &badhostfn, 0, "BADHOST", 0, "badhost", 0, &brh, &mapbrh, 0);
	/*-
	 * Enable badhelo if
	 * BADHELOCHECK is defined (default control file badhelo)
	 * or
	 * BADHELO (control file defined by BADHELO env variable)
	 * is defined
	 */
	if ((dohelocheck = (env_get("BADHELOCHECK") ? "" : env_get("BADHELO"))))
		open_control_once(&badhelook, 0, &badhelofn, 0, "BADHELO", 0, "badhelo", 0, &badhelo, &maphelo, 0);
#ifdef BATV
	open_control_once(&batvok, 0, &batvfn, 0, "SIGNKEY", 0, "signkey", 0, &signkey, 0, 0);
	if (batvok) {
		signkey.len--;
		if (!nosign.len) {
			switch (control_readfile(&nosign, "nosignhosts", 0))
			{
			case -1:
				die_control();
			case 1:
				if (!constmap_init(&mapnosign, nosign.s, nosign.len, 0))
					die_nomem();
				break;
			}
		}
		open_control_once_int(&signkeystale, &signkeystaleFn, "SIGNKEYSTALE", "signkeystale", 0);
	}
#endif
	open_control_once(&acclistok, 0, &accFn, 0, "ACCESSLIST", 0, "accesslist", 0, &acclist, 0, 0);
	if ((x = env_get("BODYCHECK"))) {
		open_control_once(&bodyok, 0, &bodyFn, 0, (x && *x) ? x : "BODYCHECK", 0, "bodycheck", 0, &body, 0, 0);
		bodyok_orig = bodyok;
	}
#ifdef USE_SPF
	open_control_once_int((int *) &spfbehavior, &spfbehaviorFn, "SPFBEHAVIOR", "spfbehavior", 0);
	open_control_once_int((int *) &spfipv6, &spfipv6Fn, "SPFIPV6", "spfipv6", 0);
#endif
	/*- TARPIT Patch - include Control Files */
	open_control_once_int(&tarpitcount, &tarpitcountFn, "TARPITCOUNT", "tarpitcount", 0);
	open_control_once_int(&tarpitdelay, &tarpitdelayFn, "TARPITDELAY", "tarpitdelay", 0);
	/*- MAXRECPIENTS - include Control Files */
	open_control_once_int(&maxrcptcount, &maxrcptcountFn, "MAXRECIPIENTS", "maxrecipients", 0);
	if ((x = env_get("VIRUSCHECK"))) {
		unsigned long   u;
		if (!*x)
			x = "1";
		scan_ulong(x, &u);
		switch (u)
		{
		case 1:	/*- Virus Scanner (Internal) */
		case 2:	/*- Virus Scanner (Internal + External) */
		case 3:	/*- Virus Scanner (Internal) + Bad Attachment Scan */
		case 4:	/*- Virus Scanner (Internal + External) + Bad Attachment Scan */
			open_control_once(&sigsok, 0, &sigsFn, 0, "SIGNATURES", 0, "signatures", 0, &sigs, 0, 0);
			sigsok_orig = sigsok;
			break;
		case 5:	/*- Virus Scanner (External) + Bad Attachment Scan*/
		case 6:	/*- Virus Scanner (External) */
		case 7:	/*- Bad Attachment scan */
			break;
		}
	}
	open_control_once_int(&greetdelay, &greetdelayFn, "GREETDELAY", "greetdelay", 0);
	open_control_once_int(&qregex, &qregexFn, "QREGEX", "qregex", 0);
	if (qregex && !env_get("QREGEX") && !env_put("QREGEX=1"))
		die_nomem();
	return;
}

#ifdef HASLIBGSASL
void
log_gsasl_version()
{
	logerr("qmail-smtpd: ");
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logerr("pid ");
	logerr(strnum);
	logerr(" gsasl header version=");
	logerr(GSASL_VERSION);
	logerr(" library version=");
	logerr(gsasl_check_version(NULL));
	logerrf("\n");
}
#endif

void
smtp_init(int force_flag)
{
	static int      flag;
#ifdef HASLIBGSASL
	int             r;
#endif

	if (!force_flag && flag)
		return;
	if (!flag)
		flag++;
	if (control_init() == -1)
		die_control();
	if (control_readfile(&greeting, "smtpgreeting", 1) != 1)
		die_control();
	if ((liphostok = control_rldef(&liphost, "localiphost", 1, (char *) 0)) == -1)
		die_control();
	if (control_readint(&timeout, "timeoutsmtpd") == -1)
		die_control();
	if (timeout <= 0)
		timeout = 1;
	if (control_readint(&maxhops, "maxhops") == -1)
		die_control();
	if (maxhops <= 0)
		maxhops = MAXHOPS;
	/*- buffer limit for commands */
	if (control_readint(&ctl_maxcmdlen, "maxcmdlen") == -1)
		die_control();
	if (ctl_maxcmdlen < 0)
		ctl_maxcmdlen = 0;
	databytes_setup();
	if (rcpthosts_init() == -1)
		die_control();
	if (recipients_init() == -1)
		die_control();
	if (!relayclient) {
		if ((relayclientsok = control_readfile(&relayclients, "relayclients", 0)) == -1)
			die_control();
		if (relayclientsok && !constmap_init(&maprelayclients, relayclients.s, relayclients.len, 0))
			die_nomem();
	}
	if (!relayclient) {
		if ((relaydomainsok = control_readfile(&relaydomains, "relaydomains", 0)) == -1)
			die_control();
		if (relaydomainsok && !constmap_init(&maprelaydomains, relaydomains.s, relaydomains.len, 0))
			die_nomem();
	}
	/*- RELAYMAILFROM Patch - include Control File */
	if ((rmfok = control_readfile(&rmf, "relaymailfrom", 0)) == -1)
		die_control();
	if (rmfok && !constmap_init(&maprmf, rmf.s, rmf.len, 0))
		die_nomem();
	if ((chkrcptok = control_readfile(&chkrcpt, "chkrcptdomains", 0)) == -1)
		die_control();
	if (chkrcptok && !constmap_init(&mapchkrcpt, chkrcpt.s, chkrcpt.len, 0))
		die_nomem();
	if ((chkdomok = control_readfile(&chkdom, "authdomains", 0)) == -1)
		die_control();
	if (chkdomok && !constmap_init(&mapchkdom, chkdom.s, chkdom.len, 0))
		die_nomem();
	if (control_readfile(&locals, "locals", 1) != 1)
		die_control();
	if (!constmap_init(&maplocals, locals.s, locals.len, 0))
		die_nomem();
#ifdef USE_SPF
	if (control_readline(&spflocal, "spfrules") == -1)
		die_control();
	if (spflocal.len && !stralloc_0(&spflocal))
		die_nomem();
	if (control_readline(&spfguess, "spfguess") == -1)
		die_control();
	if (spfguess.len && !stralloc_0(&spfguess))
		die_nomem();
	if (control_rldef(&spfexp, "spfexp", 0, SPF_DEFEXP) == -1)
		die_control();
	if (!stralloc_0(&spfexp))
		die_nomem();
#endif
	/*- initialize all variables */
	bmfok = bmpok = bhfok = bhpok = bhrcpok = bhbrpok = rcpok = brpok = 0;
	chkgrcptok = chkgrcptokp = spfok = sppok = nodnschecksok = 0;
	briok = brhok = badhelook = acclistok = bodyok = 0;
	tarpitcount = tarpitdelay = maxrcptcount = sigsok = greetdelay = qregex = 0;
	bmfFn = bmfFnp = bhsndFn = bhsndFnp = bhrcpFn = bhrcpFnp = rcpFn = rcpFnp = grcptFn = grcptFnp = 0;
	spfFn = spfFnp = bmfFn = badipfn = badhostfn = badhelofn = accFn = bodyFn = 0;
	tarpitcountFn = tarpitdelayFn = maxrcptcountFn = sigsFn = greetdelayFn = qregexFn = 0;
	proto.len = 0;
#ifdef BATV
	batvok = 0;
	signkeystale = 7;
	batvfn = signkeystaleFn = 0;
#endif
#ifdef USE_SPF
	spfbehavior = spfipv6 = 0;
	spfbehaviorFn = spfipv6Fn = 0;
#endif
	open_control_files();
#ifdef HASLIBGSASL
	if ((r = gsasl_init(&gsasl_ctx)) < 0) {
		logerr("gsasl_init: ");
		logerr(gsasl_strerror(r));
		logerrf("\r\n");
		_exit(111);
	}
	log_gsasl_version();
#endif
	return;
}

static void
setup()
{
	unsigned int    i, len;
	char           *x;

	if (!stralloc_copys(&Revision, revision + 11) ||
			!stralloc_0(&Revision))
		die_nomem();
	for (x = Revision.s; *x && *x != ' '; x++);
	if (*x == ' ')
		*x = 0;
	smtp_init(0);
#ifdef IPV6
	if (!(remoteip4 = env_get("TCPREMOTEIP")))
		remoteip4 = "unknown";
	if (!(remoteip = env_get("TCP6REMOTEIP")) && !(remoteip = remoteip4))
		remoteip = "unknown";
	if (!(localip = env_get("TCP6LOCALIP")))
		localip = env_get("TCPLOCALIP");
	if (!localip)
		localip = "unknown";
#else
	if (!(remoteip = env_get("TCPREMOTEIP")))
		remoteip = "unknown";
	if (!(localip = env_get("TCPLOCALIP")))
		localip = "unknown";
#endif
	if (!(local = env_get("TCPLOCALHOST")))
		local = hostname && *hostname ? hostname : "unknown";
	if (!(remotehost = env_get("TCPREMOTEHOST")))
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	relayclient = env_get("RELAYCLIENT");
	greyip = env_get("GREYIP");
	if (!greyip || !*greyip)
		greyip = (char *) 0; /*- Disable greylisting if GREYIP="" */
	/*- Attempt to look up the IP number in control/relayclients. */
	if (relayclientsok) {
		for (i = len = str_len(remoteip); i > 0; i--)
			if (((i == len) || (remoteip[i - 1] == '.')) && (relayclient = constmap(&maprelayclients, remoteip, i)))
				break;
	}
	/*- Attempt to look up the host name in control/relaydomains. */
	if (relaydomainsok) {
		for (i = 0, len = str_len(remotehost); i <= len; i++)
			if (((i == 0) || (i == len) || (remotehost[i] == '.'))
				&& (relayclient = constmap(&maprelaydomains, remotehost + i, len - i)))
				break;
	}
#ifdef TLS
	if (env_get("SMTPS")) {
		smtps = 1;
		tls_init();
	}
#endif
	dohelo(remotehost);
}

int
addrparse(char *arg)
{
	int             i, flagesc, flagquoted;
	char            ch, terminator;
	ip_addr         ip;

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
		die_nomem();
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) { /*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				die_nomem();
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
					die_nomem();
			}
		}
	}
	/*- could check for termination failure here, but why bother? */
	if (!stralloc_append(&addr, ""))
		die_nomem();
	if (liphostok) {
		if ((i = byte_rchr(addr.s, addr.len, '@')) < addr.len) {
			/*- if not, partner should go read rfc 821 */
			if (addr.s[i + 1] == '[') {
				if (!addr.s[i + 1 + ip4_scanbracket(addr.s + i + 1, &ip)]) {
					if (ipme_is(&ip)) {
						addr.len = i + 1;
						if (!stralloc_cat(&addr, &liphost) ||
								!stralloc_0(&addr))
							die_nomem();
					}
				}
			}
		}
	}
	if (addr.len > 900)
		return 0;
	return 1;
}

void
smtp_helo(char *arg)
{
	seenmail = 0;
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	smtp_greet("250 ");
	if (!arg || !*arg) {
		out(" [");
		out(remoteip);
		out("]");
	}
	out("\r\n");
	if (!arg || !*arg)
		dohelo(remotehost);
	else
		dohelo(arg);
	flush();
	return;
}

void
smtp_ehlo(char *arg)
{
	char            size_buf[FMT_ULONG]; /*- needed for SIZE CMD */

	seenmail = 0;
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	smtp_greet("250-");
	if (!arg || !*arg) {
		out(" [");
		out(remoteip);
		out("]");
	}
	out("\r\n");
	if (hostname && *hostname && childargs && *childargs) {
		char           *no_auth_login, *no_auth_plain, *no_cram_md5,
					   *no_cram_sha1, *no_cram_sha224, *no_cram_sha256,
					   *no_cram_sha384, *no_cram_sha512, *no_cram_ripemd,
					   *no_digest_md5;
#ifdef HASLIBGSASL
		char           *no_scram_sha1, *no_scram_sha256, *no_scram_sha512;
#endif
		int             flags1, flags2;
#ifdef TLS
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
		no_scram_sha512 = env_get("DISABLE_SCRAM_SHA512");
		no_scram_sha1_plus = env_get("DISABLE_SCRAM_SHA1_PLUS");
		no_scram_sha256_plus = env_get("DISABLE_SCRAM_SHA256_PLUS");
		no_scram_sha512_plus = env_get("DISABLE_SCRAM_SHA512_PLUS");
#endif
#ifdef HASLIBGSASL
		flags1 = !no_auth_login && !no_auth_plain && !no_cram_md5 &&
			!no_cram_sha1 && !no_cram_sha256 && !no_cram_sha512 &&
			!no_cram_ripemd && !no_digest_md5 && !no_scram_sha1 &&
			!no_scram_sha256 && !no_scram_sha512 && !no_scram_sha1_plus &&
			!no_scram_sha256_plus && !no_scram_sha512_plus;
		flags2 = !no_auth_login || !no_auth_plain || !no_cram_md5 ||
			!no_cram_sha1 || !no_cram_sha256 || !no_cram_sha512 || !no_cram_ripemd
			|| !no_scram_sha1 || !no_scram_sha256 || !no_scram_sha512 ||
			!no_scram_sha1_plus || !no_scram_sha256_plus || !no_scram_sha512_plus;
#else
		flags1 = !no_auth_login && !no_auth_plain && !no_cram_md5 &&
			!no_cram_sha1 && !no_cram_sha256 && !no_cram_sha512 &&
			!no_cram_ripemd && !no_digest_md5;
		flags2 = !no_auth_login || !no_auth_plain || !no_cram_md5 ||
			!no_cram_sha1 || !no_cram_sha256 || !no_cram_sha512 || !no_cram_ripemd;
#endif
		if (flags1) { /*- all auth methods enabled */
#ifdef HASLIBGSASL
#if 0
			out("250-AUTH LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5 SCRAM-SHA-1 SCRAM-SHA-256 SCRAM-SHA-512");
#else
			out("250-AUTH LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5 SCRAM-SHA-1 SCRAM-SHA-256");
#endif
#ifdef TLS
			if (ssl)
#if 0
				out(" SCRAM-SHA-1-PLUS SCRAM-SHA-256-PLUS SCRAM-SHA-512-PLUS");
#else
				out(" SCRAM-SHA-1-PLUS SCRAM-SHA-256-PLUS");
#endif
#endif
			out("\r\n");
			if (old_client_bug) {
#if 0
				out("250-AUTH=LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5 SCRAM-SHA-1 SCRAM-SHA-256 SCRAM-SHA-512");
#else
				out("250-AUTH=LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5 SCRAM-SHA-1 SCRAM-SHA-256");
#endif
#ifdef TLS
				if (ssl)
#if 0
					out(" SCRAM-SHA-1-PLUS SCRAM-SHA-256-PLUS SCRAM-SHA-512-PLUS");
#else
					out(" SCRAM-SHA-1-PLUS SCRAM-SHA-256-PLUS");
#endif
#endif
				out("\r\n");
			}
#else
			out("250-AUTH LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5\r\n");
			if (old_client_bug)
				out("250-AUTH=LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA224 CRAM-SHA256 CRAM-SHA384 CRAM-SHA512 CRAM-RIPEMD DIGEST-MD5\r\n");
#endif /*- #ifdef HAVELIBGSASL */
		} else /*- few auth methods disabled */
		if (flags2) {
			int             flag = 0;

			out("250-AUTH");
			if (!no_auth_login)
				out(" LOGIN");
			if (!no_auth_plain)
				out(" PLAIN");
			if (!no_cram_md5)
				out(" CRAM-MD5");
			if (!no_cram_sha1)
				out(" CRAM-SHA1");
			if (!no_cram_sha224)
				out(" CRAM-SHA224");
			if (!no_cram_sha256)
				out(" CRAM-SHA256");
			if (!no_cram_sha384)
				out(" CRAM-SHA384");
			if (!no_cram_sha512)
				out(" CRAM-SHA512");
			if (!no_cram_ripemd)
				out(" CRAM-RIPEMD");
			if (!no_digest_md5)
				out(" DIGEST-MD5");
#ifdef HASLIBGSASL
			if (!no_scram_sha1)
				out(" SCRAM-SHA-1");
			if (!no_scram_sha1_plus)
				out(" SCRAM-SHA-1-PLUS");
			if (!no_scram_sha256)
				out(" SCRAM-SHA-256");
			if (!no_scram_sha256_plus)
				out(" SCRAM-SHA-256-PLUS");
#if 0
			if (!no_scram_sha512)
				out(" SCRAM-SHA-512");
			if (!no_scram_sha512_plus)
				out(" SCRAM-SHA-512-PLUS");
#endif
#endif
			out("\r\n");
			if (old_client_bug) {
				if (!no_auth_login) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("LOGIN");
				}
				if (!no_auth_plain) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("PLAIN");
				}
				if (!no_cram_md5) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-MD5");
				}
				if (!no_cram_sha1) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-SHA1");
				}
				if (!no_cram_sha224) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-SHA224");
				}
				if (!no_cram_sha256) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-SHA256");
				}
				if (!no_cram_sha384) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-SHA384");
				}
				if (!no_cram_sha512) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-SHA512");
				}
				if (!no_cram_ripemd) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("CRAM-RIPEMD");
				}
				if (!no_digest_md5) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("DIGEST-MD5");
				}
#ifdef HASLIBGSASL
				if (!no_scram_sha1) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-1");
				}
				if (!no_scram_sha1_plus) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-1-PLUS");
				}
				if (!no_scram_sha256) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-256");
				}
				if (!no_scram_sha256_plus) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-256-PLUS");
				}
#if 0
				if (!no_scram_sha512) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-512");
				}
				if (!no_scram_sha512_plus) {
					out(flag++ == 0 ? "250-AUTH=" : " ");
					out("SCRAM-SHA-512-PLUS");
				}
#endif
#endif
				out("\r\n");
			}
		}
	}
	if (hasvirtual) {
		if (smtp_port != ODMR_PORT) {
			out("250-PIPELINING\r\n");
			out("250-8BITMIME\r\n");
			if (databytes > 0) {
				size_buf[fmt_ulong(size_buf, (unsigned long) databytes)] = 0;
				out("250-SIZE ");
				out(size_buf);
				out("\r\n");
			}
			if (smtp_port != SUBM_PORT)
				out("250-ETRN\r\n");
		} else
			out("250-ATRN\r\n");
	} else {
		out("250-PIPELINING\r\n");
		out("250-8BITMIME\r\n");
		if (databytes > 0) {
			size_buf[fmt_ulong(size_buf, (unsigned long) databytes)] = 0;
			out("250-SIZE ");
			out(size_buf);
			out("\r\n");
		}
		if (smtp_port != SUBM_PORT)
			out("250-ETRN\r\n");
	}
#ifdef TLS
	if (!ssl && env_get("STARTTLS")) {
		stralloc        filename = { 0 };
		struct stat     st;

		if (!certdir) {
			if (!(certdir = env_get("CERTDIR")))
				certdir = auto_control;
		}
		if (!stralloc_copys(&filename, certdir) ||
				!stralloc_catb(&filename, "/", 1))
			die_nomem();
		servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
		if (!stralloc_cats(&filename, servercert) ||
				!stralloc_0(&filename))
			die_nomem();
		if (!stat(filename.s, &st))
			out("250-STARTTLS\r\n");
		alloc_free(filename.s);
	}
#endif
#ifdef SMTPUTF8
	if (env_get("SMTPUTF8")) {
		smtputf8_enable = 1;
		out("250-SMTPUTF8\r\n");
	}
#endif
	out("250 HELP\r\n");
	flush();
	if (!arg || !*arg)
		dohelo(remotehost);
	else
		dohelo(arg);
	return;
}

void
smtp_rset(char *arg)
{
	if (arg && *arg) {
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		flush();
		return;
	}
	seenmail = rcptto.len = mailfrom.len = addr.len = 0;
	out("250 flushed\r\n");
	flush();
	return;
}

#ifdef BATV
int
check_batv_sig()
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, md5pos, atpos, slpos;
	char            kdate[] = "0000";
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_MD_CTX     *mdctx;
	const EVP_MD   *md = 0;
	unsigned char   md5digest[EVP_MAX_MD_SIZE];
	unsigned int    md_len;
#else
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
#endif
	unsigned long   signday;

	if (addr.len >= (11 + 2 * BATVLEN) && stralloc_starts(&addr, "prvs=")) {
		atpos = str_rchr(addr.s, '@');
		addr.s[atpos] = 0;	/*- just for a moment */
		slpos = str_rchr(addr.s, '='); /*- prefer an = sign */
		addr.s[atpos] = '@';
		byte_copy(kdate, 4, addr.s + 5);
		md5pos = 9;
	} else
		return 0; /*- no BATV */
	if (kdate[0] != '0')
		return 0; /* not known format 0 */
	if (scan_ulong(kdate + 1, &signday) != 3)
		return 0;
	if ((unsigned) (daynumber - signday) > signkeystale)
		return 0; /*- stale bounce */
	addr.len--;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!(md = EVP_MD_fetch(NULL, "md5", NULL)))
		die_custom("qmail-smtpd: batv: unable to fetch digest implementation for MD5");
	if (!(mdctx = EVP_MD_CTX_new()))
		die_nomem();
	if (!EVP_DigestInit_ex(mdctx, md, NULL) ||
			!EVP_DigestUpdate(mdctx, kdate, 4) ||/*- date */
			!EVP_DigestUpdate(mdctx, addr.s + slpos + 1, addr.len - slpos - 1) ||
			!EVP_DigestUpdate(mdctx, signkey.s, signkey.len) ||
			!EVP_DigestFinal_ex(mdctx, md5digest, &md_len))
		die_custom("qmail-smtpd: batv: unable to hash md5 message digest");
	EVP_MD_free(md);
#else
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, addr.s + slpos + 1, addr.len - slpos - 1);
	MD5_Update(&md5, signkey.s, signkey.len);
	MD5_Final(md5digest, &md5);
#endif
	for (i = 0; i < BATVLEN; i++) {
		int             c, x;
		c = addr.s[md5pos + 2 * i];
		if (isdigit(c))
			x = c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x = 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x = 10 + c - 'A';
		else
			return 0; /*- bad signature */
		x <<= 4;
		c = addr.s[md5pos + 2 * i + 1];
		if (isdigit(c))
			x += c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x += 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x += 10 + c - 'A';
		else
			return 0; /*- bad signature */
		if (x != md5digest[i])
			return 0; /*- bad signature */
	}
	/*- peel off the signature */
	addr.len -= slpos;
	byte_copy(addr.s, addr.len, addr.s + slpos + 1);
	return 1;
}
#endif /*- #ifdef BATV */

int
pop_bef_smtp(char *mfrom)
{
	char           *ptr, *errstr;
	void           *(*inquery) (char, char *, char *);

	if (!(ptr = load_virtual()))
		return 1;
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &errstr))) {
		err_library(errstr);
		return 1;
	}

	if ((ptr = (*inquery) (RELAY_QUERY, mfrom, remoteip))) {
		if ((authenticated = *ptr)) /*- also set authenticated variable */
			relayclient = "";
		if (!env_put2("AUTHENTICATED", authenticated == 1 ? "1" : "0"))
			die_nomem();
	} else {
		out("451 Requested action aborted: temporary database error (#4.3.2)\r\n");
		flush();
		logerr("qmail-smtpd: ");
		logerrpid();
		logerrf("Database error\n");
		return (1);
	}
	return (0);
}

/*
 * Allow aliasdomains or DEFAULT_DOMAIN to be matched with a real
 * domain in the mailfrom or the authenticated username.
 */
int
domain_compare(char *dom1, char *dom2)
{
	char           *ptr, *tmpdom1, *tmpdom2, *errstr;
	void           *(*inquery) (char, char *, char *);

	if (!(ptr = load_virtual()))
		return -1;
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &errstr))) {
		err_library(errstr);
		return -1;
	}

	if (str_diff(dom1, dom2)) {
		if (!(tmpdom1 = (*inquery) (DOMAIN_QUERY, dom1, 0)) ||
				!(tmpdom2 = (*inquery) (DOMAIN_QUERY, dom2, 0))) {
			out("451 Requested action aborted: temporary database error (#4.3.2)\r\n");
			flush();
			logerr("qmail-smtpd: ");
			logerrpid();
			logerrf("Database error\n");
			return (-1);
		}
		if (str_diff(tmpdom1, tmpdom2)) {
			err_nogateway(remoteip, mailfrom.s, 0, 1);
			return (1);
		}
	}
	return (0);
}

int             flagsize = 0;
stralloc        mfparms = { 0 };

int
mailfrom_size(char *arg)
{
	long            r;
	unsigned long   sizebytes = 0;

	scan_ulong(arg, (unsigned long *) &r);
	sizebytes = r;
	msg_size = r;
	if (databytes > 0 && sizebytes > databytes)
		return 1;
	return 0;
}

void
mailfrom_auth(char *arg, int len)
{
	if (authd)
		return;
	if (!stralloc_copys(&user, ""))
		die_nomem();
	if (case_starts(arg, "<>")) {
		if (!stralloc_cats(&user, "unknown"))
			die_nomem();
	} else {
		while (len) {
			if (*arg == '+') {
				if (case_starts(arg, "+3D")) {
					arg = arg + 2;
					len = len - 2;
					if (!stralloc_cats(&user, "="))
						die_nomem();
				}
				if (case_starts(arg, "+2B")) {
					arg = arg + 2;
					len = len - 2;
					if (!stralloc_cats(&user, "+"))
						die_nomem();
				}
			} else
			if (!stralloc_catb(&user, arg, 1))
				die_nomem();
			arg++;
			len--;
		}
	}
	if (!stralloc_0(&user))
		die_nomem();
	if (!remoteinfo) {
		remoteinfo = user.s;
		if (!env_unset("TCPREMOTEINFO"))
			die_nomem();
		if (!env_put2("TCPREMOTEINFO", remoteinfo))
			die_nomem();
		if (!env_put2("AUTHINFO", remoteinfo))
			die_nomem();
	}
}

void
mailfrom_parms(char *arg)
{
	int             i;
	int             len;

	len = str_len(arg);
	mfparms.len = 0;
	i = byte_chr(arg, len, '>');
	if (i > 4 && i < len) {
		while (len) {
			arg++;
			len--;
			if (*arg == ' ' || *arg == '\0') {
#ifdef SMTPUTF8
				if (smtputf8_enable && case_starts(mfparms.s, "SMTPUTF8")) {
					smtputf8 = 1;
				} else
#endif
				if (case_starts(mfparms.s, "SIZE=")) {
					mfparms.s[mfparms.len] = 0;
					if (mailfrom_size(mfparms.s + 5)) {
						flagsize = 1;
						return;
					}
				} else
				if (case_starts(mfparms.s, "AUTH=")) {
					mfparms.s[mfparms.len] = 0;
					mailfrom_auth(mfparms.s + 5, mfparms.len - 5);
				}
				mfparms.len = 0;
			} else
			if (!stralloc_catb(&mfparms, arg, 1))
				die_nomem();
		}
	}
}

static int      f_envret = 0, d_envret = 0, a_envret = 0;

void
smtp_mail(char *arg)
{
	PASSWD         *pw;
	char           *x, *ptr;
	int             ret, i;
	int            *u_not_found, *i_inactive;
#ifdef SMTP_PLUGIN
	char           *mesg;
#endif
#ifdef USE_SPF
	int             r;
#endif
	void           *(*inquery) (char, char *, char *);

	/*-
	 * If this is the second session restore original environment.
	 * This is because envrules(), domainqueues could have modified
	 * the environmnent in the previous session
	 */
	restore_env();
	if (f_envret || d_envret || a_envret) {	/* reload control files if in an earlier session, envrules was used */
		open_control_files();
		f_envret = 0;
	}
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!seenhelo) {
		out("503 Polite people say hello first (#5.5.4)\r\n");
		flush();
		return;
	}
	if (!addrparse(arg)) {
		err_syntax();
		return;
	}
#ifdef SMTPUTF8
	if (!stralloc_catb(&proto, smtputf8 ? "UTF8ESMTP" : "ESMTP", smtputf8 ? 9 : 5))
		die_nomem();
#else
	if (!stralloc_catb(&proto, "ESMTP", 5))
		die_nomem();
#endif
#ifdef TLS
	if (ssl && !stralloc_append(&proto, "S"))
		die_nomem();
#endif
	if (authd && !stralloc_append(&proto, "A"))
		die_nomem();

#ifdef BATV
	if (batvok)
		(void) check_batv_sig(); /*- unwrap in case it's ours */
#endif
	switch ((f_envret = envrules(addr.s, "from.envrules", "FROMRULES", &errStr)))
	{
	case AM_MEMORY_ERR:
		die_nomem();
	case AM_FILE_ERR:
		die_control();
	case AM_REGEX_ERR:
		/*- flow through */
	case 0:
		break;
	default:
		if (f_envret > 0) {
			/*-
			 * No point in setting Following environment variables as they have been
			 * read in the function setup() which gets called only once
			 * SMTPS, RELAYCLIENT, TCPREMOTEINFO, TCPREMOTEHOST, TCPLOCALIP
			 * TCPLOCALHOST, TCPREMOTEIP
			 */
			databytes_setup(); /*- so that it is possible to set DATABYTES again using envrules */
			open_control_files();
			log_rules(remoteip, addr.s, authd ? remoteinfo : 0, f_envret, 0);
		}
		break;
	}
	if (!(ret = tablematch("hostaccess", remoteip, addr.s + str_chr(addr.s, '@')))) {
		err_hostaccess(remoteip, addr.s);
		return;
	} else
	if (ret == -1)
		die_control();
	else
	if (ret == -2)
		die_nomem();
	flagsize = 0;
	mailfrom_parms(arg);
	if ((requireauth = env_get("REQUIREAUTH")) && !authd) {
		err_authrequired();
		return;
	}
	/*- Terminate SMTP Session immediatly if BLACKHOLED Sender is seen */
	switch (address_match(bhsndFn, &addr, bhfok ? &bhf : 0, bhfok ? &mapbhf : 0, bhpok ? &bhp : 0, &errStr))
	{
	case 1:/*- flow through */
		err_bhf(remoteip, addr.s); /*- This sets NULLQUEUE */
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr, bhsndFn);
		return;
	}
	/*- badmailfrom, badmailpatterns */
	switch (address_match(bmfFn, &addr, bmfok ? &bmf : 0, bmfok ? &mapbmf : 0, bmpok ? &bmp : 0, &errStr))
	{
	case 1:
		err_bmf(remoteip, addr.s);
		return;
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr, bmfFn);
		return;
	}

	if (!nodnscheck) {
		switch (dnscheck(addr.s, addr.len - 1, 0))
		{
		case DNS_HARD:
			err_hmf(remoteip, arg, 1);
			return;
		case DNS_SOFT:
			err_smf();
			return;
		case DNS_MEM:
			die_nomem();
		}
	}
	if ((bouncemail = env_get("BOUNCEMAIL"))) {
		err_maps(bouncemail, remoteip, addr.s);
		return;
	}
	if (env_get("SPAMFILTER")) {
		/*- spamignore, spamignorepatterns */
		switch (address_match(spfFn, &addr, spfok ? &spf : 0, spfok ? &mapspf : 0, sppok ? &spp : 0, &errStr))
		{
		case 1:
			if (!env_unset("SPAMFILTER"))
				die_nomem();
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, spfFn);
			return;
		}
	}
	if (!stralloc_copys(&rcptto, "") ||
			!stralloc_copys(&mailfrom, addr.s) ||
			!stralloc_0(&mailfrom))
		die_nomem();
	if ((x = env_get("MAX_RCPT_ERRCOUNT")))
		scan_int(x, &max_rcpt_errcount);
	else
		max_rcpt_errcount = -1;
	rcptcount = rcpt_errcount = 0;
	/*- relaymailfrom */
	if (!relayclient) {
		switch (address_match("relaymailfrom", &mailfrom, rmfok ? &rmf : 0, rmfok ? &maprmf : 0, 0, &errStr))
		{
		case 1:
			relayclient = "";
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, "relaymailfrom");
			return;
		}
	}
	if (!hasvirtual)
		goto nohasvirtual;
	if (!(ptr = load_virtual()))
		return;
	else
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &x))) {
		err_library(x);
		return;
	}
	/*-
	 * closed user group mailing
	 * allow only sender domains listed in rcpthosts to
	 * send mails.
	 */
	if (env_get("CUGMAIL")) {
		if (!addrallowed(addr.s)) {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" Invalid SENDER address: MAIL from <");
			logerr(mailfrom.s);
			logerrf(">\n");
			out("553 authorization failure (#5.7.1)\r\n");
			flush();
			return;
		}
		if (!(pw = (*inquery) (PWD_QUERY, mailfrom.s, 0))) {
			if (!(u_not_found = (int *) getlibObject(ptr, &phandle, "userNotFound", &x))) {
				err_library(x);
				return;
			}
			if (*u_not_found) {
				/*- 
				 * Accept the mail as denial could be stupid
				 * like the vrfy command
				 */
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr(remoteip);
				logerr(" mail from invalid user <");
				logerr(mailfrom.s);
				logerrf(">\n");
				out("553 authorization failure (#5.7.1)\r\n");
				flush();
				sleep(5); /*- Prevent DOS */
				return;
			} else {
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr(remoteip);
				logerrf(" Database error\n");
				out("451 Requested action aborted: temporary database error (#4.3.2)\r\n");
				flush();
				return;
			}
		} else {
			if (!(i_inactive = (int *) getlibObject(ptr, &phandle, "is_inactive", &x))) {
				err_library(x);
				return;
			}
			if (*i_inactive || pw->pw_gid & NO_SMTP) {
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr(remoteip);
				logerr(" SMTP Access denied to <");
				logerr(mailfrom.s);
				logerr("> ");
				logerrf(*i_inactive ? "user inactive" : "No SMTP Flag");
				out("553 authorization failure (#5.7.1)\r\n");
				flush();
				return;
			}
		}
	} /*- if (env_get("CUGMAIL")) */
	/*-
	 * ANTISPOOFING 
	 * Delivery to local domains
	 * If the mailfrom is local and rcptto is local, do not allow
	 * receipt without authentication (unless MASQUERADE is set).
	 * (do not allow spoofing for local users)
	 */
	if (!relayclient && !authd && env_get("ANTISPOOFING") && addrallowed(mailfrom.s)) {
		if (pop_bef_smtp(mailfrom.s))	  /*- will set the variable authenticated */
			return;	/*- temp error */
		if (authenticated != 1) { /*- in case pop-bef-smtp also is negative */
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" unauthenticated local SENDER address: MAIL from <");
			logerr(mailfrom.s);
			logerrf(">\n");
			out("530 authentication required for local users (#5.7.1)\r\n");
			flush();
			return;
		}
	}
	x = env_get("MASQUERADE");
	if ((!x || (x && *x)) && authd) {
		int             at1, at2, iter_pass, flag;
		char           *dom1, *dom2, *allowed;

		if (mailfrom.s[at1 = str_chr(mailfrom.s, '@')]) {
			dom1 = mailfrom.s + at1 + 1;
			if (!addrallowed(mailfrom.s)) {
				err_nogateway(remoteip, mailfrom.s, 0, 1);
				return;
			}
			mailfrom.s[at1] = 0;
			for (flag = 0, iter_pass = 0;; iter_pass++) {
				if (x && *x) {
					allowed = iter_pass ? remoteinfo : x;
				} else {
					allowed = remoteinfo;
					iter_pass++;
				}
				if (allowed[at2 = str_chr(allowed, '@')]) {
					dom2 = allowed + at2 + 1;
					allowed[at2] = 0;
					if (str_diff(mailfrom.s, allowed)) {
						allowed[at2] = '@';
						flag = 1;
					}
					allowed[at2] = '@';
					/*- now compare domains */
					if (domain_compare(dom1, dom2) == 1)
						flag = 1;
				} else {
					if (str_diff(mailfrom.s, allowed))
						flag = 1;
					/*- now compare mailfrom domain with $DEFAULT_DOMAIN */
					if ((dom2 = env_get("DEFAULT_DOMAIN"))) {
						if (domain_compare(dom1, dom2) == 1)
							flag = 1;
					}
				}
				if (!flag) {
					mailfrom.s[at1] = '@';
					break;
				}
				if (iter_pass == 1) {
					mailfrom.s[at1] = '@';
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					break;
				}
			} /*- for (;;) */
		} else {
			if (x && *x) {
				if (str_diff(mailfrom.s, x) && str_diff(mailfrom.s, remoteinfo)) {
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					return;
				}
			} else {
				if (str_diff(mailfrom.s, remoteinfo)) {
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					return;
				}
			}
		}
	}
nohasvirtual:
#ifdef USE_SPF
	flagbarfspf = 0;
	if (spfbehavior && !relayclient) {
#ifdef IPV6
		switch (r = spfcheck(spfipv6 ? remoteip : remoteip4))
#else
		switch (r = spfcheck(remoteip))
#endif
		{
		case SPF_OK:
			env_put2("SPFRESULT","pass");
			break;
		case SPF_NONE:
			env_put2("SPFRESULT","none");
			break;
		case SPF_UNKNOWN:
			env_put2("SPFRESULT","unknown");
			break;
		case SPF_NEUTRAL:
			env_put2("SPFRESULT","neutral");
			break;
		case SPF_SOFTFAIL:
			env_put2("SPFRESULT","softfail");
			break;
		case SPF_FAIL:
			env_put2("SPFRESULT","fail");
			break;
		case SPF_ERROR:
			env_put2("SPFRESULT","error");
			break;
		}

		switch(r)
		{
		case SPF_NOMEM:
			die_nomem();
		case SPF_ERROR:
			if (spfbehavior < 2)
				break;
			out("451 SPF lookup failure (#4.3.0)\r\n");
			flush();
			return;
		case SPF_NONE:
		case SPF_UNKNOWN:
			if (spfbehavior < 6)
				break;
		case SPF_NEUTRAL:
			if (spfbehavior < 5)
				break;
		case SPF_SOFTFAIL:
			if (spfbehavior < 4)
				break;
		case SPF_FAIL:
			if (spfbehavior < 3)
				break;
			if (!spfexplanation(&spfbarfmsg))
				die_nomem();
			if (!stralloc_0(&spfbarfmsg))
				die_nomem();
			flagbarfspf = 1;
		}
	} else
		env_unset("SPFRESULT");
	if (flagbarfspf) {
		err_spf();
		return;
	}
#endif /*- #ifdef USE_SPF */
	/*- authdomains */
	if (chkdomok) {
		switch (address_match("authdomains", &mailfrom, chkdomok ? &chkdom : 0, chkdomok ? &mapchkdom : 0, 0, &errStr))
		{
		case 0:
			chkdomok = 0;
		case 1:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, "authdomains");
			return;
		}
	}
#ifdef SMTP_PLUGIN
	for (i = 0; i < plugin_count; i++) {
		if (!plug[i] || !plug[i]->mail_func)
			continue;
		if (plug[i]->mail_func(remoteip, addr.s, &mesg)) {
			out(mesg);
			flush();
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr("plugin(from)[");
			strnum[fmt_ulong(strnum, i)] = 0;
			logerr(strnum);
			logerr("]: ");
			logerr(mesg);
			logerrf("\n");
			return;
		}
	}
#endif
#if BATV
	if (batvok) {
		isbounce = 0;
		if (addr.len <= 1 /*- null term */
			|| stralloc_starts(&addr, "mailer-daemon@")
			|| stralloc_starts(&addr, "Mailer-Daemon@")
			|| stralloc_starts(&addr, "MAILER-DAEMON@")) {
			ret = str_rchr(addr.s, '@');
			if (!addr.s[ret] || !nosign.len || !constmap(&mapnosign, addr.s + ret + 1, addr.len - ret - 2))
				isbounce = 1;
		}
	}
#endif /*- BATV*/
#ifdef TLS
	if (env_get("FORCE_TLS")) {
		if (!ssl) {
			out("530 must issue STARTTLS first (#5.7.0)\r\n");
			flush();
			return;
		}
	}
#endif
	seenmail = 1;
	out("250 ok\r\n");
	flush();
	return;
}

void
smtp_rcpt(char *arg)
{
	int             allowed_rcpthosts = 0, isgoodrcpt = 0, result = 0, at, isLocal;
	char           *tmp;
#if BATV
	int             ws = -1;
#endif
#ifdef SMTP_PLUGIN
	char           *mesg;
	int             i;
#endif

	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!seenmail) {
		err_wantmail();
		return;
	}
	if (!addrparse(arg)) {
		err_syntax();
		return;
	}
#ifdef BATV
	if (batvok)
		ws = check_batv_sig(); /*- always strip sig, even if it's not a bounce */
#endif
	if (addrrelay()) {
		err_relay();
		return;
	}
	/*- goodrcptto, goodrcptpatterns */
	switch (address_match(grcptFn, &addr, chkgrcptok ? &grcpt : 0, chkgrcptok ? &mapgrcpt : 0, chkgrcptokp ? &grcptp : 0, &errStr))
	{
	case 1:
		isgoodrcpt = 4;
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr, grcptFn);
		return;
	}
	/*- RECIPIENT BAD check */
	if (isgoodrcpt != 4) { /*- not in goodrcptto, goodrcptpatterns */
		/*- badrcptto, badrcptpatterns */
		switch (address_match(rcpFn, &addr, rcpok ? &rcp : 0, rcpok ? &maprcp : 0, brpok ? &brp : 0, &errStr))
		{
		case 1:
			err_rcp(remoteip, mailfrom.s, addr.s);
			return;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, rcpFn);
			return;
		}
	}
	if (acclistok) {
		switch (mail_acl(&acclist, qregex, mailfrom.s, addr.s, 0))
		{
		case 1:
			err_acl(remoteip, mailfrom.s, addr.s);
			return;
		case 0:
			break;
		default:
			out("451 Requested action aborted: local system failure (#4.3.0)\r\n");
			flush();
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" accesslist: ");
			logerr(error_str(errno));
			logerrf("\n");
			return;
		}
	}
	/*- If AUTH_ALL is defined, allowed_rcpthosts = 0 */
	if (!chkdomok && !env_get("AUTH_ALL"))
		allowed_rcpthosts = addrallowed(addr.s);
	if (relayclient) {
		--addr.len;
		if (!stralloc_cats(&addr, relayclient) ||
				!stralloc_0(&addr))
			die_nomem();
	} else /*- RELAYCLIENT not set */
	if (!allowed_rcpthosts) { /*- not in rcpthosts - delivery to remote domains */
		if (hasvirtual) {
			if (env_get("CHECKRELAY") && pop_bef_smtp(mailfrom.s))
				return;
			if (authenticated != 1) {
				err_nogateway(remoteip, mailfrom.s, addr.s, 0);
				return;
			}
		} else {
			err_nogateway(remoteip, mailfrom.s, addr.s, 0);
			return;
		}
	} /*- if (!allowed_rcpthosts) */
	/*-
	 * If rcptto is local, check status of recipients
	 * (do not allow mail to be sent to invalid users)
	 */
	if (allowed_rcpthosts == 1 && (tmp = env_get("CHECKRECIPIENT"))) {
		/*- chkrcptdomains */
		switch (address_match("chkrcptdomains", &addr, chkrcptok ? &chkrcpt : 0, chkrcptok ? &mapchkrcpt : 0, 0, &errStr))
		{
		case 1: /*- in chkrcptdomains */
			chkrcptok = 0;
			break;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, "chkrcptdomains");
			return;
		}
		if (!chkrcptok) {
			if (isgoodrcpt != 4) { /*- not in goodrcptto, goodrcptpatterns */
				if (hasvirtual) {
					if (*tmp)
						scan_int(tmp, &isgoodrcpt);
					else
						isgoodrcpt = 3;	/*- default is cdb */
				} else
					isgoodrcpt = 3;	/*- default is cdb */
			}
			switch (isgoodrcpt)
			{
			case 1:	/* reject if user not in sql db */
				if ((at = byte_rchr(addr.s, addr.len - 1, '@')) < addr.len - 1)
					isLocal = (constmap(&maplocals, addr.s + at + 1, addr.len - at - 2) ? 1 : 0);
				else
					isLocal = 1;
				result = (isLocal ? check_recipient_pwd : check_recipient_sql) (addr.s, at);
				break;
			case 2:	/* reject if user not in both recipient.cdb and sql db */
				if ((at = byte_rchr(addr.s, addr.len - 1, '@')) < addr.len - 1)
					isLocal = (constmap(&maplocals, addr.s + at + 1, addr.len - at - 2) ? 1 : 0);
				else
					isLocal = 1;
				if ((result = !check_recipient_cdb(addr.s)))
					result = (isLocal ? check_recipient_pwd : check_recipient_sql) (addr.s, at);
				break;
			case 4:
				result = 0;
				break;
			case 3:	/* reject if user not in recipient */
			default:
				result = (!check_recipient_cdb(addr.s) ? 1 : 0);
				break;
			}
			if (result > 0) {
				rcpt_errcount++;
				sleep(5); /*- Prevent DOS */
			}
			switch (result)
			{
			case 1:
				err_mailbox(mailfrom.s, addr.s, "is absent on this domain (#5.1.1)");
				return;
			case 2:
				err_mailbox(mailfrom.s, addr.s, "is inactive on this domain (#5.2.1)");
				return;
			case 3:
				err_mailbox(mailfrom.s, addr.s, "is overquota on this domain (#5.2.2)");
				return;
			}
		}
	}
	if (max_rcpt_errcount != -1 && rcpt_errcount >= max_rcpt_errcount) {
		err_rcpt_errcount(mailfrom.s, rcpt_errcount);
		return;
	}
	/*- RECIPIENT BLACKHOLE */
	if (isgoodrcpt != 4) { /*- not in goodrcptto, goodrcptpatterns */
		/*- blackholedrcpt, blackholedrcptpatterns */
		switch (address_match(bhrcpFn, &addr, bhrcpok ? &bhrcp : 0, bhrcpok ? &mapbhrcp : 0, bhbrpok ? &bhbrp : 0, &errStr))
		{
		case 1:
			err_bhrcp(remoteip, mailfrom.s, addr.s);
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr, "bhrcpFn");
			return;
		}
	}
#if BATV
	/*- if wasn't signed and was bounce, sideline it */
	if (batvok && !relayclient && !ws && isbounce) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr("bad bounce: at ");
		logerr(remoteip);
		logerr(" to ");
		logerr(addr.s);
		logerrf("\n");
		out("553 Not our message (#5.7.1)\r\n");
		flush();
		return;
	}
#endif
#ifdef SMTP_PLUGIN
	for (i = 0; i < plugin_count; i++) {
		if (!plug[i] || !plug[i]->rcpt_func)
			continue;
		if (plug[i]->rcpt_func(remoteip, mailfrom.s, addr.s, &mesg)) {
			out(mesg);
			flush();
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr("plugin(rcpt)[");
			strnum[fmt_ulong(strnum, i)] = 0;
			logerr(strnum);
			logerr("]: ");
			logerr(mesg);
			logerrf("\n");
			return;
		}
	}
#endif
	/*- Check on max. number of RCPTS */
	if (maxrcptcount > 0 && (rcptcount + 1) > maxrcptcount) {
		err_mrc(remoteip, mailfrom.s, arg);
		return;
	}
	if (tarpitcount && (rcptcount + 1) >= tarpitcount)
		while (sleep(tarpitdelay));	/*- TARPIT Delay */
	if (mailfrom.len == 1 && (rcptcount + 1) > 1) {
		err_badbounce();
		return;
	}
	/*- domain based queue */
	switch ((d_envret = domainqueue(addr.s, "domainqueue", "DOMAINQUEUE", &errStr)))
	{
	case AM_MEMORY_ERR:
		die_nomem();
	case AM_FILE_ERR:
		die_control();
	case AM_REGEX_ERR:
		/*- flow through */
	case 0:
		break;
	default:
		if (d_envret > 0)
			log_rules(remoteip, addr.s, authd ? remoteinfo : 0, d_envret, 1);
	}
	rcptcount++;
	if (!stralloc_cats(&rcptto, "T") ||
			!stralloc_cats(&rcptto, addr.s) ||
			!stralloc_0(&rcptto))
		die_nomem();
	out("250 ok\r\n");
	flush();
	return;
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	flush();
#ifdef TLS
	if (ssl && fd == ssl_rfd)
		r = ssl_timeoutread(timeout, ssl_rfd, ssl_wfd, ssl, buf, len);
	else
		r = timeoutread(timeout, fd, buf, len);
#else
	r = timeoutread(timeout, fd, buf, len);
#endif
	if (r == -1) {
		if (errno == error_timeout)
			die_alarm();
	}
	if (r < 0)
#ifdef TLS
		ssl ? die_read("ssl_timeoutread", ssl_error()) : die_read("timeoutread", 0);
#else
		die_read("timeoutread", 0);
#endif
	return r;
}

/*-
 * =0 after boundary is found in body,
 * until blank line
 */
int             linespastheader;
int             flagexecutable, flagbody, flagqsbmf, boundary_start;
char            linetype;

stralloc        pline = { 0 };
stralloc        line = { 0 };
stralloc        content = { 0 };
stralloc        boundary = { 0 };

/*-
 * put() puts characters into the queue.  We remember those characters
 * and form them into a line.  When we get a newline, we examine the
 * line.  If we're currently in a header (0 linespastheader), we look
 * for Content-Type.  If we're at the newline that ends a header, we
 * look to see if the content is multipart.  If it is, then we push
 * the current boundary, remember the boundary, otherwise we set the
 * boundary to the empty string.  Set the linespastheader to 1.  When
 * linespastheader is 1, and the boundary is empty, scan the line for
 * signatures.  If the boundary is non-empty, look for a match against
 * the boundary.  If it matches and is followed by two dashes, pop the
 * boundary, otherwise set linespastheader to 0.
 */

void
put(char *ch)
{
	char           *cp, *cpstart, *cpafter;
	unsigned int    len;

	if (!sigsok && !bodyok) {
		if (BytesToOverflow && !--BytesToOverflow)
			qmail_fail(&qqt);
		msg_size++;
		qmail_put(&qqt, ch, 1);
		return;
	}
	if (line.len < 1024 && !stralloc_catb(&line, ch, 1))
		die_nomem();
	if (*ch == '\n' && (sigsok || bodyok)) {
		if (linespastheader == 0) {
			if (line.len > 1 && isspace(line.s[0])) {
				if (!stralloc_cat(&pline, &line))
					die_nomem();
			} else
			if (!stralloc_copy(&pline, &line))
				die_nomem();
		}
		if (bodycheck(&body, &pline, &content_desc, linespastheader == 0) == 1) {
			flagbody = 1;
			/*- turn off virus/content-filtering on first match */
			sigsok = bodyok = 0;
			qmail_fail(&qqt);
		}
		if (linespastheader == 0) {
			if (line.len == 1) {
				linespastheader = 1;
				if (flagqsbmf) {
					flagqsbmf = 0;
					linespastheader = 0;
				}
				if (content.len) { /*- MIME header */
					cp = content.s;
					len = content.len;
					while (len && (*cp == ' ' || *cp == '\t')) {
						++cp;
						--len;
					}
					cpstart = cp;
					if (len && *cp == '"') { /*- might be commented */
						++cp;
						--len;
						cpstart = cp;
						while (len && *cp != '"') {
							++cp;
							--len;
						}
					} else {
						while (len && *cp != ' ' && *cp != '\t' && *cp != ';') {
							++cp;
							--len;
						}
					}
					if (!case_diffb(cpstart, cp - cpstart, "message/rfc822"))
						linespastheader = 0;
					cpafter = content.s + content.len;
					while ((cp += byte_chr(cp, cpafter - cp, ';')) != cpafter) {
						++cp;
						while (cp < cpafter && (*cp == ' ' || *cp == '\t'))
							++cp;
						if (case_startb(cp, cpafter - cp, "boundary=")) {
							cp += 9;	/*- after boundary= */
							if (cp < cpafter && *cp == '"') {
								++cp;
								cpstart = cp;
								while (cp < cpafter && *cp != '"')
									++cp;
							} else {
								cpstart = cp;
								while (cp < cpafter && *cp != ';' && *cp != ' ' && *cp != '\t')
									++cp;
							}
							/*-
							 * push the current boundary. 
							 * Append a null and remember start. 
							 */
							if (!stralloc_0(&boundary))
								die_nomem();
							boundary_start = boundary.len;
							if (!stralloc_cats(&boundary, "--") ||
									!stralloc_catb(&boundary, cpstart, cp - cpstart))
								die_nomem();
							break;
						}
					}
				}
			} else { /*- non-blank header line */
				if ((*line.s == ' ' || *line.s == '\t')) {
					switch (linetype)
					{
					case 'C':
						if (!stralloc_catb(&content, line.s, line.len - 1))
							die_nomem();
						break;
					default:
						break;
					}
				} else {
					if (case_startb(line.s, line.len, "content-type:")) {
						if (!stralloc_copyb(&content, line.s + 13, line.len - 14))
							die_nomem();
						linetype = 'C';
					} else
						linetype = ' ';
				}
			}
		} else { /*- non-header line */
			if (boundary.len - boundary_start && *line.s == '-' && line.len > (boundary.len - boundary_start) && !str_diffn(line.s, boundary.s + boundary_start, boundary.len - boundary_start)) {
				/*- matches a boundary */
				if (line.len > boundary.len - boundary_start + 2 && line.s[boundary.len - boundary_start + 0] == '-'
					&& line.s[boundary.len - boundary_start + 1] == '-') { /*- XXXX - pop the boundary here */
					if (boundary_start)
						boundary.len = boundary_start - 1;
					boundary_start = boundary.len;
					while (boundary_start--)
						if (!boundary.s[boundary_start])
							break;
					boundary_start++;
					linespastheader = 2;
				} else
					linespastheader = 0;
			} else
			if (linespastheader == 1) { /*- first line -- match a signature?  */
				if (/*- mailfrom.s[0] == '\0' && */
					   str_start(line.s, "Hi. This is the "))
					flagqsbmf = 1;
				else /*- mailfrom.s[0] == '\0' && */
				if ( str_start(line.s, "This message was created automatically by mail delivery software"))
					flagqsbmf = 1;
				else
				if (sigscheck(&line, &virus_desc, linespastheader == 0)) {
					flagexecutable = 1;
					/*- turn off virus/content-filtering on first match */
					sigsok = bodyok = 0;
					qmail_fail(&qqt);
				}
				linespastheader = 2;
			}
			if (flagqsbmf && str_start(line.s, "---"))
				linespastheader = 0;
		}
		line.len = 0;
	}
	if (BytesToOverflow && !--BytesToOverflow)
		qmail_fail(&qqt);
	msg_size++;
	qmail_put(&qqt, ch, 1);
}

int
blast(int *hops)
{
	char            ch;
	int             state, err;
	int             flaginheader;
	unsigned int    pos;		/*- number of bytes since most recent \n, if fih */
	int             flagmaybew;	/*- 1 if this line might match RETURN-RECEIPT, if fih */
	int             flagmaybex;	/*- 1 if this line might match RECEIVED, if fih */
	int             flagmaybey;	/*- 1 if this line might match \r\n, if fih */
	int             flagmaybez;	/*- 1 if this line might match DELIVERED, if fih */
	int             seencr;

	state = 1;
	*hops = 0;
	flaginheader = 1;
	dsn = pos = 0;
	flagmaybew = flagmaybex = flagmaybey = flagmaybez = 1;
	msg_size = 0;
	seencr = 0;	/*- qmail-smtpd-newline patch */
	for (;;) {
		if ((err = substdio_get(&ssin, &ch, 1)) <= 0)
			return (1);
		if (ch == '\n') {
			if (seencr == 0) {
				substdio_seek(&ssin, -1);
				ch = '\r';
			}
		}
		if (ch == '\r')
			seencr = 1;
		else
			seencr = 0;
		if (flaginheader) {
			if (pos > 8 && pos < 14) {
				if (ch != "return-receipt"[pos] && ch != "RETURN-RECEIPT"[pos])
					flagmaybew = 0;
				if (flagmaybew && pos == 13)
					dsn = 1;
			}
			if (pos < 9) {
				if (ch != "delivered"[pos] && ch != "DELIVERED"[pos])
					flagmaybez = 0;
				if (flagmaybez && pos == 8)
					++ * hops;
				if (pos < 8 && ch != "received"[pos] && ch != "RECEIVED"[pos])
					flagmaybex = 0;
				if (flagmaybex && pos == 7)
					++ * hops;
				if (pos < 2 && ch != "\r\n"[pos])
					flagmaybey = 0;
				if (flagmaybey && pos == 1)
					flaginheader = 0;
			}
			/*-
			 * We are interested only in return-receipt, delivered, received
			 * headers. So no point in checking beyond pos = 13
			 */
			if (pos < 14)
				++pos;
			if (ch == '\n') {
				pos = 0;
				flagmaybew = flagmaybex = flagmaybey = flagmaybez = 1;
			}
		}
		switch (state)
		{
		case 0:
			if (ch == '\n')
				straynewline();
			if (ch == '\r') {
				state = 4;
				continue;
			}
			break;
		case 1: /*- \r\n */
			if (ch == '\n')
				straynewline();
			if (ch == '.') {
				state = 2;
				continue;
			}
			if (ch == '\r') {
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 2: /*- \r\n + . */
			if (ch == '\n')
				straynewline();
			if (ch == '\r') {
				state = 3;
				continue;
			}
			state = 0;
			break;
		case 3: /*- \r\n + .\r */
			if (ch == '\n')
				return (0);
			put(".");
			put("\r");
			if (ch == '\r') {
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 4: /*- + \r */
			if (ch == '\n') {
				state = 1;
				break;
			}
			if (ch != '\r') {
				put("\r");
				state = 0;
			}
		}
		put(&ch);
	} /*- for (;;) */
	return (0);
}

#ifdef USE_SPF
stralloc        rcvd_spf = { 0 };
void
spfreceived()
{

	if (!spfbehavior || relayclient)
		return;
	if (!stralloc_copys(&rcvd_spf, "Received-SPF: ") ||
			!spfinfo(&sa) ||
			!stralloc_cat(&rcvd_spf, &sa) ||
			!stralloc_append(&rcvd_spf, "\n"))
		die_nomem();
	if (BytesToOverflow) {
		BytesToOverflow -= rcvd_spf.len;
		if (BytesToOverflow <= 0)
			qmail_fail(&qqt);
	}
	qmail_put(&qqt, rcvd_spf.s, rcvd_spf.len);
}
#endif

void
acceptmessage(unsigned long qp)
{
	datetime_sec    when;

	when = now();
	out("250 ok ");
	accept_buf[fmt_ulong(accept_buf, (unsigned long) when)] = 0;
	out(accept_buf);
	out(" qp ");
	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	out(accept_buf);
	out("\r\n");
	flush();
	return;
}

static void
create_logfilter()
{
	int             fd;
	char           *tmpdir, *x;
	static stralloc tmpFile = { 0 };

	if (env_get("LOGFILTER")) {
		if (!(tmpdir = env_get("TMPDIR")))
			tmpdir = "/tmp";
		if (!stralloc_copys(&tmpFile, tmpdir) ||
				!stralloc_cats(&tmpFile, "/smtpFilterXXX") ||
				!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())) ||
				!stralloc_0(&tmpFile))
			die_nomem();
		if ((x = env_get("LOGFD")))
			scan_int(x, &logfd);
		if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
			die_logfilter();
		if (unlink(tmpFile.s))
			die_logfilter();
		if (dup2(fd, logfd) == -1)
			die_logfilter();
		if (fd != logfd)
			close(fd);
	}
	return;
}

void
smtp_data(char *arg)
{
	int             hops;
	unsigned long   qp;
	char           *qqx;
#ifdef SMTP_PLUGIN
	int             i;
	char           *mesg;
#endif

#ifdef HAS_MYSQL
	if (hasvirtual)
		sqlmatch_close_db();
#endif
	if (arg && *arg) {
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		flush();
		return;
	}
	if (!seenmail) {
		err_wantmail();
		return;
	}
	if (!rcptto.len || !rcptcount) {
		err_wantrcpt();
		return;
	}
	if (greyip && !relayclient) {
		switch (greylist(greyip, remoteip, mailfrom.s, rcptto.s, rcptto.len, err_greytimeout, err_grey_tmpfail))
		{
		case 1:/*- success */
			break;
		case 0:
			err_grey();
			/*- flow through */
		case -1:
			return;
		case -2:
			die_nomem();
		}
	}
	seenmail = 0;
	/*- Return error if incoming SMTP msg exceeds DATABYTES */
	if (flagsize) {
		err_size(remoteip, mailfrom.s, rcptto.s, rcptto.len);
		return;
	}
	if (databytes > 0)
		BytesToOverflow = databytes + 1;
	if (sigsok) {
		boundary.len = 0;
		boundary_start = 0;
		content.len = 0;
		linespastheader = 0;
		flagexecutable = 0;
		flagqsbmf = 0;
		linetype = ' ';
		virus_desc = "";
	}
	create_logfilter();
	if (relayclient || authd) {
		switch ((a_envret = envrules("", "auth.envrules", "AUTHRULES", &errStr)))
		{
		case AM_MEMORY_ERR:
			die_nomem();
		case AM_FILE_ERR:
			die_control();
		case AM_REGEX_ERR:
			/*- flow through */
		case 0:
			break;
		default:
			if (a_envret > 0) {
				log_rules(remoteip, "authorized", authd ? remoteinfo : 0, a_envret, 0);
			}
			break;
		}
	}
	if (qmail_open(&qqt) == -1) {
		err_qqt(remoteip);
		return;
	}
	qp = qmail_qp(&qqt); /*- pid of queue process */
	out("354 go ahead\r\n");
	if (proto.len) {
		if (!stralloc_0(&proto))
			die_nomem();
		protocol = proto.s;
	}
	received(&qqt, (char *) protocol, local, remoteip,
			str_diff(remotehost, "unknown") ? remotehost : 0, remoteinfo, fakehelo);
#ifdef USE_SPF
	spfreceived();
#endif
	/*- write the body */
	if (!blast(&hops)) {
		hops = (hops >= maxhops);
		if (hops)
			qmail_fail(&qqt);
#ifdef SMTP_PLUGIN
		for (i = 0; i < plugin_count; i++) {
			if (!plug[i] || !plug[i]->data_func)
				continue;
			if (plug[i]->data_func(local, remoteip, remotehost, remoteinfo, &mesg)) {
				out(mesg);
				flush();
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr("plugin(data)[");
				strnum[fmt_ulong(strnum, i)] = 0;
				logerr(strnum);
				logerr("]: ");
				logerr(mesg);
				logerrf("\n");
				return;
			}
		}
#endif
	} else
		qmail_fail(&qqt);
	/*- write the envelope */
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);
	/*-
	 * RFC 1047, In case we are accepting the mail
	 * we should immediately respond
	 * back to client as early as possible to
	 * avoid message duplication
	 */
	qqx = qmail_close(&qqt);
	if (!*qqx) { /*- mail is now in queue */
		acceptmessage(qp);
		log_trans(remoteip, mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0, 0);
		return;
	}
	/*- you will reach here if qmail_fail() was called or if qmail_close returns error */
	if (flagexecutable || flagbody) {
		sigsok = sigsok_orig;
		bodyok = bodyok_orig;
		if (flagexecutable) {
			log_virus(remoteip, virus_desc, mailfrom.s, rcptto.s, rcptto.len, flagblackhole);
			flagexecutable = flagblackhole = 0;
			return;
		}
		if (flagbody) {
			log_virus(remoteip, content_desc, mailfrom.s, rcptto.s, rcptto.len, flagblackhole);
			flagbody = flagblackhole = 0;
			return;
		}
	}
	if (hops) {
		err_hops();
		return;
	}
	if (databytes > 0 && !BytesToOverflow) {
		err_size(remoteip, mailfrom.s, rcptto.s, rcptto.len);
		return;
	}
	if (*qqx == 'D')
		out("554 ");
	else
		out("451 ");
	out(qqx + 1);
	out("\r\n");
	flush();
	err_queue(remoteip, mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0, qqx + 1, *qqx == 'D', qp);
	return;
}

int
authgetl(void)
{
	int             i;

	if (!stralloc_copys(&authin, ""))
		die_nomem();
	for (;;) {
		if (!stralloc_readyplus(&authin, 1))
			die_nomem(); /*- XXX */
		i = substdio_get(&ssin, authin.s + authin.len, 1);
		if (i != 1) {
			if (!i)
				errno = 0;
			die_read("client dropped connection", 0);
		}
		if (authin.s[authin.len] == '\n')
			break;
		++authin.len;
	}
	if (authin.len > 0 && authin.s[authin.len - 1] == '\r')
		--authin.len;
	authin.s[authin.len] = 0;
	if (*authin.s == '*' && *(authin.s + 1) == 0)
		return err_authabrt();
#if 0
	if (authin.len == 0)
		return err_input();
#endif
	return (authin.len);
}

int
authenticate(int method)
{
	int             child, wstat, i, n = 0;
	int             pi[2], po[2] = { -1, -1};
#ifdef TLS
	int             pe[2];
	struct substdio pwd_in;
	char            pwd_in_buf[1024];
#endif
	char            respbuf[1024];

	if (!stralloc_0(&user) ||
			!stralloc_0(&pass) ||
			!stralloc_0(&resp))
		die_nomem();
	if (pipe(pi) == -1) /*- password input data */
		return err_pipe();
	if (pipe(po) == -1) /*- digest md5 */
		return err_pipe();
#ifdef TLS
	if (ssl && pipe(pe) == -1) /*- smtp message pipe */
		return err_pipe();
#endif
	switch (child = fork())
	{
	case -1:
		return err_fork();
	case 0:
		close(pi[1]);
		close(po[0]);
#ifdef TLS
		close(pe[0]);
#endif
		if (pi[0] != 3) {
			if (dup2(pi[0], 3) == -1)
				return err_write();
			close(pi[0]);
		}
		if (po[1] != 6) {
			if (dup2(po[1], 6) == -1)
				return err_write();
			close(po[1]);
		}
#ifdef TLS
		if (ssl && pe[1] != 1) {
			if (dup2(pe[1], 1) == -1)
				return err_write();
			close(pe[1]);
		}
#endif
		sig_pipedefault();
		execv(*childargs, childargs);
		_exit(1);
	}
	close(pi[0]);
	close(po[1]);
#ifdef TLS
	if (ssl)
		close(pe[1]);
#endif
	substdio_fdbuf(&ssup, safewrite, pi[1], upbuf, sizeof upbuf);
	if (substdio_put(&ssup, user.s, user.len) == -1)
		return err_write();
	if (substdio_put(&ssup, pass.s, pass.len) == -1)
		return err_write();
	if (substdio_put(&ssup, resp.s, resp.len) == -1)
		return err_write();
	strnum[0] = method;
	strnum[1] = 0;
	if (!stralloc_copyb(&authmethod, strnum, 2))
		die_nomem();
	if (substdio_put(&ssup, authmethod.s, authmethod.len) == -1)
		return err_write();
	if (substdio_flush(&ssup) == -1)
		return err_write();
	close(pi[1]);
	byte_zero(pass.s, pass.len);
	byte_zero(upbuf, sizeof upbuf);
	if (method == AUTH_DIGEST_MD5) {
		if ((n = saferead(po[0], respbuf, 33)) == -1)
			die_read("err reading digest md5 response", 0);
		respbuf[n] = 0;
		close(po[0]);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	if ((i = wait_exitcode(wstat))) {
#ifdef TLS
		if (ssl) { /*- don't let plain text from exec'd programs to interfere with ssl */
			substdio_fdbuf(&pwd_in, saferead, pe[0], pwd_in_buf, sizeof(pwd_in_buf));
			if (substdio_copy(&ssout, &pwd_in) == -2)
				die_read("error reading checkpassword pipe", 0);
		}
#endif
		/* 
		 * i == 3 - account doesn't have RELAY permissions.
		 * pw_gid is set with NO_RELAY
		 * This is returned by vchkpass auth program
		 */
		return (i == 111 ? -1 : i);
	}
	if (method == AUTH_DIGEST_MD5) {
		if (!n)
			return (1);
		if (!stralloc_copys(&slop, "rspauth=") ||
				!stralloc_catb(&slop, respbuf, n))
			die_nomem();
		slop.s[slop.len] = 0;
		if (b64encode(&slop, &resp) < 0)
			die_nomem();
		if (!stralloc_0(&resp))
			die_nomem();
		out("334 ");
		out(resp.s);
		out("\r\n");
		flush();
		/*- digest-md5 requires a special okay response ...  */
		if ((n = saferead(0, respbuf, 512)) == -1)
			die_read("failed to get OK from client", 0);
		if (n)
			respbuf[n] = 0;
		return (0);
	} else
		return (i);
}

static int
auth_login(char *arg)
{
	int             r;

#ifdef TLS
	if (secure_auth && !ssl)
		return err_noauthallowed();
#endif
	if (*arg) {
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &user)) == 1)
			return err_input();
	} else {
		out("334 VXNlcm5hbWU6\r\n");
		flush(); /*- Username: */
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &user)) == 1)
			return err_input();
	}
	if (r == -1)
		die_nomem();
	out("334 UGFzc3dvcmQ6\r\n");
	flush(); /*- Password: */
	if (authgetl() < 0)
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &pass)) == 1)
		return err_input();
	if (r == -1)
		die_nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate(AUTH_LOGIN);
	if (!r || r == 3)
		authd = AUTH_LOGIN;
	return (r);
}

static int
auth_plain(char *arg)
{
	int             r, id = 0;

#ifdef TLS
	if (secure_auth && !ssl)
		return err_noauthallowed();
#endif
	if (*arg) {
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &slop)) == 1)
			return err_input();
	} else {
		out("334 \r\n");
		flush();
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
			return err_input();
	}
	if (r == -1 || !stralloc_0(&slop))
		die_nomem();
	while (slop.s[id])
		id++; /*- ignore authorize-id */
	if (slop.len > id + 1)
		if (!stralloc_copys(&user, slop.s + id + 1))
			die_nomem();
	if (slop.len > id + user.len + 2)
		if (!stralloc_copys(&pass, slop.s + id + user.len + 2))
			die_nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate(AUTH_PLAIN);
	if (!r || r == 3)
		authd = AUTH_PLAIN;
	return (r);
}

static int
auth_cram(int method)
{
	int             i, r;
	char           *s;
	char            unique[FMT_ULONG + FMT_ULONG + 3];

	s = unique;
	s += fmt_uint(s, getpid());
	*s++ = '.';
	s += fmt_ulong(s, (unsigned long) now());
	*s++ = '@';
	*s++ = 0;

	/*- generate challenge */
	if (!stralloc_copys(&pass, "<") ||
			!stralloc_cats(&pass, unique) ||
			!stralloc_cats(&pass, hostname) ||
			!stralloc_cats(&pass, ">") ||
			b64encode(&pass, &slop) < 0 ||
			!stralloc_0(&slop))
		die_nomem();

	out("334 "); /*- "334 mychallenge \r\n" */
	out(slop.s);
	out("\r\n");
	flush();
	if (authgetl() < 0) /*- got response */
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (r == -1 || !stralloc_0(&slop))
		die_nomem();
	i = str_chr(slop.s, ' ');
	s = slop.s + i;
	while (*s == ' ')
		++s;
	slop.s[i] = 0;
	if (!stralloc_copys(&user, slop.s)) /*- userid */
		die_nomem();
	if (!stralloc_copys(&resp, s)) /*- digest */
		die_nomem();
	if (!user.len || !resp.len)
		return err_input();
	r = authenticate(method);
	if (!r || r == 3)
		authd = method;
	return (r);
}

#ifdef HASLIBGSASL
static PASSWD  *gsasl_pw;
static stralloc scram_method;
static int      gs_callback_err;

PASSWD         *
get_scram_record(char *u, int *mech, int *iter, char **salt, char **stored_key,
		char **server_key, char **hexsaltpw, char **cleartxt, char **saltedpw)
{
	int             i;
	char           *ptr, *err;
	int            *u_not_found, *i_inactive;
	void           *(*inquery) (char, char *, char *);

	gsasl_pw = (PASSWD *) NULL;
	if (!hasvirtual)
		return ((PASSWD *) NULL);
	if (!(ptr = load_virtual()))
		return ((PASSWD *) NULL);
	else
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &err))) {
		err_library(err);
		return ((PASSWD *) NULL);
	}
	if (!stralloc_copys(&user, u) || !stralloc_0(&user))
		die_nomem();
	user.len--;
	if (!(gsasl_pw = (*inquery) (PWD_QUERY, u, 0))) {
		if (!(u_not_found = (int *) getlibObject(ptr, &phandle, "userNotFound", &err))) {
			err_library(err);
			return ((PASSWD *) NULL);
		}
		if (*u_not_found) {
			/*- 
			 * Accept the mail as denial could be stupid
			 * like the vrfy command
			 */
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" mail from invalid user <");
			logerr(u);
			logerrf(">\n");
			out("553 authorization failure (#5.7.1)\r\n");
			flush();
			sleep(5); /*- Prevent DOS */
			return ((PASSWD *) NULL);
		} else {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerrf(" Database error\n");
			out("451 Requested action aborted: temporary database error (#4.3.2)\r\n");
			flush();
			return ((PASSWD *) NULL);
		}
	} else {
		if (!(i_inactive = (int *) getlibObject(ptr, &phandle, "is_inactive", &err))) {
			err_library(err);
			gsasl_pw = (PASSWD *) NULL;
			return ((PASSWD *) NULL);
		}
		if (*i_inactive || gsasl_pw->pw_gid & NO_SMTP) {
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" SMTP Access denied to <");
			logerr(u);
			logerr("> ");
			logerrf(*i_inactive ? "user inactive" : "No SMTP Flag");
			out("553 authorization failure (#5.7.1)\r\n");
			flush();
			gsasl_pw = (PASSWD *) NULL;
			return ((PASSWD *) NULL);
		}
	}
	if (str_diffn(gsasl_pw->pw_passwd, scram_method.s, scram_method.len)) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerr(" SCRAM AUTH Method not supported for user ");
		logerr(scram_method.s);
		i = str_chr(gsasl_pw->pw_passwd, '}');
		if (gsasl_pw->pw_passwd[i]) {
			logerr(" != ");
			substdio_put(&sserr, gsasl_pw->pw_passwd, i + 1);
		}
		logerrf("\n");
		out("553 authorization failure (#5.7.1)\r\n");
		flush();
		gsasl_pw = (PASSWD *) NULL;
		return ((PASSWD *) NULL);
	} else
		*mech = 0;
	i = get_scram_secrets(gsasl_pw->pw_passwd, mech, iter, salt, stored_key, server_key, hexsaltpw, cleartxt, saltedpw);
	if (i != 6 && i != 8) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerr(" Unable to get secrets for <");
		logerr(u);
		logerrf(">\n");
		out("553 authorization failure (#5.7.1)\r\n");
		flush();
		gsasl_pw = (PASSWD *) NULL;
		return ((PASSWD *) NULL);
	}
	return gsasl_pw;
}

static int
is_scram_method(int mech)
{
	switch (mech)
	{
	case AUTH_SCRAM_SHA1:
	case AUTH_SCRAM_SHA256:
	case AUTH_SCRAM_SHA1_PLUS:
	case AUTH_SCRAM_SHA256_PLUS:
		return 1;
	default:
		return 0;
	}
}

static int
get_user_details(Gsasl_session *sctx, char **u, int *mech, int *iter,
		char **salt, char **stored_key, char **server_key,
		char **hexsaltpw, char **cleartxt, char **saltedpw)
{
	int             rc;

	if (!*u && !(*u = gsasl_property_fast(sctx, GSASL_AUTHID))) {
		logerr("gsasl_property_fast: unable to get GSASL_AUTHID\n");
		return GSASL_NO_CALLBACK;
	}
	if (!(gsasl_pw = get_scram_record(*u, mech, iter, salt, stored_key,
					server_key, hexsaltpw, cleartxt, saltedpw)))
		return gs_callback_err = GSASL_NO_CALLBACK;
	if (!is_scram_method(*mech)) {
		strnum[fmt_int(strnum, *mech)] = 0;
		logerr("unknown SCRAM AUTH method [");
		logerr(strnum);
		logerr("]\n");
		return GSASL_NO_CALLBACK;
	}
	strnum[fmt_int(strnum, *iter)] = 0;
#if GSASL_VERSION_MAJOR > 1
	if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_ITER, strnum)) != GSASL_OK) {
		logerr("gsasl_property_set: GSASL_SCRAM_ITER: ");
		logerr(gsasl_strerror(rc));
		logerrf("\n");
		return GSASL_NO_CALLBACK;
	}
#else
	gsasl_property_set(sctx, GSASL_SCRAM_ITER, strnum);
	rc = GSASL_OK;
#endif
	return rc;
}

void
err_scram(char *err_code1, char *err_code2, char *mesg, char *str)
{
	out(err_code1);
	out(" ");
	out(mesg);
	out(" (#");
	out(err_code2);
	out(")\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" ");
	logerr(mesg);
	if (str) {
		logerr(" [");
		logerr(str);
		logerr("]");
	}
	logerrf("\n");
}

#ifdef TLS
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1
char           *
get_finish_message(cb_type type)
{
	char            tls_finish_buf[EVP_MAX_MD_SIZE];
	static stralloc in = {0}, res = {0};
	int             i, tls_finish_len;

	if (!ssl) /*- we should never be here */
		return ((char *) NULL);
	switch (type)
	{
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1
	case tls_unique: /*- RFC 5929 */
		/*-
		 * SSL_get_peer_finished() does not offer a way to know the exact length
		 * of a TLS finish message beforehand, so we use EVP_MAX_MD_SIZE
		 * and return failure if the message does not fit (this will however
		 * never happen)
		 */
		tls_finish_len = SSL_get_peer_finished(ssl, tls_finish_buf, EVP_MAX_MD_SIZE);
		if (tls_finish_len > EVP_MAX_MD_SIZE)
			return ((char *) NULL);
		break;
#endif
#if GSASL_VERSION_NUMBER >= 0x020002
	case tls_exporter: /*- RFC 9266 tls-exporter length = 32 */
		tls_finish_len = 32;
		if ((i = SSL_export_keying_material(ssl, (unsigned char *) tls_finish_buf,
						tls_finish_len, "EXPORTER-Channel-Binding", 24, 0, 0, 1)) != 1) {
			err_scram("410", "4.3.5", "Temporary channel binding failure", "SSL_export_keyring_material failed");
			return ((char *) NULL);
		}
		break;
#endif
	default:
		if (type == tls_unique || type == tls_exporter) {
			if (!stralloc_copyb(&res, "channel binding type=", 21) ||
					!stralloc_cats(&res, type == tls_unique ? "tls-unique" : "tls-exporter") ||
					!stralloc_catb(&res, " not supported", 14) ||
					!stralloc_0(&res))
				die_nomem();
		} else {
			i = strnum[fmt_int(strnum, type)] = 0;
			if (!stralloc_copyb(&res, "channel binding type=", 21) ||
					!stralloc_catb(&res, strnum, i) ||
					!stralloc_catb(&res, " not supported", 14) ||
					!stralloc_0(&res))
				die_nomem();
		}
		err_scram("410", "4.3.5", res.s, 0);
		return ((char *) NULL);
	}
	/*
	 * Save the TLS finish message expected to be found, useful for
	 * authentication checks related to channel binding.
	 */
	if (!stralloc_copyb(&in, tls_finish_buf, tls_finish_len) ||
			b64encode(&in, &res) != 0 || !stralloc_0(&res))
		die_nomem();
	return res.s;
}
#endif
#endif

static int
gs_callback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop)
{
	int             rc = GSASL_NO_CALLBACK;
	static int      mech, iter = 4096;
	static char    *u, *salt, *stored_key, *server_key, *hexsaltpw,
				   *cleartxt, *saltedpw;
	static int      i = -1;
#ifdef TLS
	static char    *p;
#endif

	gs_callback_err = 0;
	switch (prop)
	{
	case GSASL_AUTHID:
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		} else
		if (gsasl_pw && u)
			rc = GSASL_OK;
		break;
	case GSASL_AUTHZID: /*- ignored */
		break;
	case GSASL_PASSWORD:
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_PASSWORD, cleartxt)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_PASSWORD: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_PASSWORD, cleartxt);
		rc = GSASL_OK;
#endif
		break;
	case GSASL_SCRAM_SALT:
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_SALT, salt)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SCRAM_SALT: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SCRAM_SALT, salt);
		rc = GSASL_OK;
#endif
		break;
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	case GSASL_SCRAM_SERVERKEY:
		if (env_get("GSASL_PASSWORD"))
			break;
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_SERVERKEY, server_key)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SCRAM_SERVERKEY: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SCRAM_SERVERKEY, server_key);
		rc = GSASL_OK;
#endif
		break;
	case GSASL_SCRAM_STOREDKEY:
		if (env_get("GSASL_PASSWORD"))
			break;
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_STOREDKEY, stored_key)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SCRAM_STOREDKEY: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SCRAM_STOREDKEY, stored_key);
		rc = GSASL_OK;
#endif
		break;
#endif
	case GSASL_SCRAM_SALTED_PASSWORD:
		if (i == -1) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_SALTED_PASSWORD, hexsaltpw)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SCRAM_SALTED_PASSWORD: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SCRAM_SALTED_PASSWORD, hexsaltpw);
		rc = GSASL_OK;
#endif
		break;
	/*- These are for GSSAPI/GS2 only.  */
	case GSASL_SERVICE:
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SERVICE, "smtp")) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SERVICE: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SERVICE, "smtp");
		rc = GSASL_OK;
#endif
		break;
	case GSASL_HOSTNAME:
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_HOSTNAME, hostname)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_HOSTNAME: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_HOSTNAME, hostname);
		rc = GSASL_OK;
#endif
		break;
	case GSASL_SCRAM_ITER:
		if (i == -1 && u) {
			if ((rc = get_user_details(sctx, &u, &mech, &iter, &salt, &stored_key,
							&server_key, &hexsaltpw, &cleartxt, &saltedpw)) != GSASL_OK)
				return gs_callback_err = GSASL_NO_CALLBACK;
			i = 0;
		}
		strnum[fmt_int(strnum, iter)] = 0;
#if GSASL_VERSION_MAJOR > 1
		if ((rc = gsasl_property_set(sctx, GSASL_SCRAM_ITER, strnum)) != GSASL_OK) {
			logerr("gsasl_property_set: GSASL_SCRAM_ITER: ");
			logerr(gsasl_strerror(rc));
			logerrf("\n");
		}
#else
		gsasl_property_set(sctx, GSASL_SCRAM_ITER, strnum);
		rc = GSASL_OK;
#endif
		break;
	case GSASL_VALIDATE_GSSAPI:
		return gs_callback_err = GSASL_OK;
#ifdef TLS
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 4 || GSASL_VERSION_MAJOR > 1
	case GSASL_CB_TLS_UNIQUE:
		if ((p = get_finish_message(tls_unique))) {
#if GSASL_VERSION_MAJOR > 1
			if ((rc = gsasl_property_set(sctx, GSASL_CB_TLS_UNIQUE, p)) != GSASL_OK) {
				logerr("gsasl_property_set: GSASL_CB_TLS_UNIQUE: ");
				logerr(gsasl_strerror(rc));
				logerrf("\n");
			}
#else
			gsasl_property_set(sctx, GSASL_CB_TLS_UNIQUE, p);
			rc = GSASL_OK;
#endif
		} else {
			logerrf("unable to get finish message for GSASL_CB_TLS_UNIQUE\n");
			return gs_callback_err = GSASL_NO_CALLBACK;
		}
		break;
#endif
#if GSASL_VERSION_NUMBER >= 0x020002
	case GSASL_CB_TLS_EXPORTER:
		if ((p = get_finish_message(tls_exporter))) {
			if ((rc = gsasl_property_set(sctx, GSASL_CB_TLS_EXPORTER, p)) != GSASL_OK) {
				logerr("gsasl_property_set: GSASL_CB_TLS_EXPORTER: ");
				logerr(gsasl_strerror(rc));
				logerrf("\n");
			}
		} else {
			logerrf("unable to get finish message for GSASL_CB_TLS_EXPORTER\n");
			return gs_callback_err = GSASL_NO_CALLBACK;
		}
		break;
#endif
#endif
	default:
		strnum[fmt_int(strnum, prop)] = 0;
		logerr("unknown callback [");
		logerr(strnum);
		logerrf("]\n");
		break;
	}
	return gs_callback_err = rc;
}

static int
gsasl_server_auth(Gsasl_session *session)
{
	char           *p;
	int             r, i = 0;

	/*
	 * The ordering and the type of checks in the following loop has to
	 * be adapted for each protocol depending on its SASL properties.
	 * SMTP is normally a "server-first" SASL protocol, but if
	 * INITIAL_CHALLENGE is supplied by the client it turns into a
	 * client-first SASL protocol.  This implementation do not support
	 * piggy-backing of the terminating server response.  See RFC 2554
	 * and RFC 4422 for terminology.  That profile results in the
	 * following loop structure.  Ask on the help-gsasl list if you are
	 * uncertain.  
	 */
	do {
		if (gs_callback_err)
			return 1;
		else
		if (i++ > 0 && !gsasl_pw)
			return 1;
		r = gsasl_step64(session, authin.s, &p);
		if (r == GSASL_NEEDS_MORE || (r == GSASL_OK && p && *p)) {
			out("334 ");
			out(p);
			out("\r\n");
			flush();
			gsasl_free(p);
			if (authgetl() < 0) /*- got response */
				return 1;
		}
	} while (r == GSASL_NEEDS_MORE);

	if (r != GSASL_OK) {
		logerr("gsasl_step64: ");
		logerr(gsasl_strerror(r));
		logerrf("\n");
		return 1;
	}
	return 0;
}

/*- RFC 5802, RFC 7677, RFC 5056, RFC 5929, RFC 9266 */
static int
auth_scram(int method)
{
	int             r = -1, i, cb_required, cb_disabled;
	Gsasl_session  *session = NULL;
	char           *p;

	cb_required = cb_disabled = 0;
	gsasl_callback_set(gsasl_ctx, gs_callback);
	user.len = 0;
  
	strnum[0] = method;
	strnum[1] = 0;
	if (!stralloc_copyb(&authmethod, strnum, 2))
		die_nomem();
	switch(method)
	{
	case AUTH_SCRAM_SHA1:
		if (no_scram_sha1_plus)
			cb_disabled = 1;
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-1}", 13) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-1", &session);
		break;
	case AUTH_SCRAM_SHA1_PLUS:
		cb_required = 1;
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-1}", 13) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-1-PLUS", &session);
		break;
	case AUTH_SCRAM_SHA256:
		if (no_scram_sha256_plus)
			cb_disabled = 1;
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-256}", 15) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-256", &session);
		break;
	case AUTH_SCRAM_SHA256_PLUS:
		cb_required = 1;
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-256}", 15) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-256-PLUS", &session);
		break;
#if 0
	case AUTH_SCRAM_SHA512:
		if (no_scram_sha512_plus)
			cb_disabled = 1;
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-512}", 15) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-512", &session);
		break;
	case AUTH_SCRAM_SHA512_PLUS:
		if (!stralloc_copyb(&scram_method, "{SCRAM-SHA-512}", 15) ||
				!stralloc_0(&scram_method))
			die_nomem();
		scram_method.len--;
		r = gsasl_server_start(gsasl_ctx, "SCRAM-SHA-512-PLUS", &session);
		break;
#endif
	} /*- switch(method) */
	if (r != GSASL_OK) {
		logerr("gsasl_server_start: ");
		logerr(gsasl_strerror(r));
		logerrf("\n");
		_exit(111);
	}
	out("334 \r\n");
	flush();

	if (authgetl() < 0) /*- got response */
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (r == -1 || !stralloc_0(&slop))
		die_nomem();
	slop.len--;
	/*
	 * Read gs2-cbind-flag. Server handles its value as described in RFC 5802,
	 * section 6 dealing with channel binding.
	 *
	 * client first message must start with n, y or p
	 * e.g. from RFC documentation
	 * C: n,,n=user,r=fyko+d2lbbFgONRv9qkxdawL
	 * S: r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j,s=QSXCR+Q6sek8bf92, i=4096
	 * C: c=biws,r=fyko+d2lbbFgONRv9qkxdawL3rfcNHYJY1ZVvWVs7j, p=v0X8v3Bz2T0CJGbJQyF0X+HI4Ts=
	 * S: v=rmF9pqV8S7suAoZWja4dJRkFsKQ=
	 */
	switch(slop.s[0])
	{
	case 'n':
		if (cb_required) {
			/*-
			 * Client does not support channel binding. Since selected auth mechanism
			 * requires cb, we do support this option.
			 */
			err_scram("535", "5.7.4", "client doesn't support channel binding, but selected mechanism does", 0);
		}
		/*- check if the syntax is correct by checking for comma */
		if (slop.s[1] != ',') {
			err_scram("535", "5.7.1", "535 malformed SCRAM message", slop.s);
			return 1;
		}
		break;
	case 'y':
#ifdef TLS
		if (ssl && !cb_disabled) {
			/*-
			 * this is a downgrade attack
			 * Client supports channel binding and thinks that the server
			 * does not. Server must fail authentication if it supports channel binding,
			 * which is the case if a connection is using TLS.
			 */
			err_scram("535", "5.7.4", "client supports channel binding, but thinks server doesn't", 0);
			return 1;
		}
#endif
		if (slop.s[1] != ',') {
			err_scram("535", "5.7.1", "535 malformed SCRAM message", slop.s);
			return 1;
		}
		break;
	case 'p':
#ifdef TLS
		if (!ssl || (ssl && cb_disabled)) {
			/*
			 * Client requires channel binding but we don't support it.
			 *
			 * RFC 5802 specifies a particular error code,
			 * e=server-does-support-channel-binding, for this.  But it can
			 * only be sent in the server-final message, and we don't want to
			 * go through the motions of the authentication, knowing it will
			 * fail, just to send that error message.
			 * The client requires channel binding.  Channel binding type
			 * follows, e.g., "p=tls-unique".
			 */
			err_scram("535", "5.7.4", "client requires channel binding, but server doesn't support", slop.s + 2);
			return 1;
		}
#else
		err_scram("535", "5.7.4", "client requires channel binding, but server doesn't support", slop.s + 2);
		return 1;
#endif
		if (slop.s[1] != '=') {
			err_scram("535", "5.7.1", "535 malformed SCRAM message", slop.s);
			return 1;
		}
		for (p = slop.s + 2;*p && *p != ','; p++);
		i = 0;
		if (*p == ',') {
			*p = '\0';
			i = 1;
		}
		/* channel name is slop.s + 2 */
		if (str_diffn(slop.s + 2, "tls-unique", 11) && str_diffn(slop.s + 2, "tls-exporter", 13)) {
			err_scram("535", "5.7.6", "unexpected SCRAM channel-binding type", slop.s + 2);
			return 1;
		}
		if (i) /*- replace the comma */
			*p = ',';
		break;
	default:
		err_scram("535", "5.7.1", "535 malformed SCRAM message", slop.s);
		return 1;
	}
	/*-
	 * Read channel-binding.  We don't support channel binding for instance running
	 * without TLS, so it's expected to always be "biws" in this case,
	 * which is "n,,", base64-encoded. "biws" is used for Non-SSL connection attempts,
	 * and for SSL connections the client has to provide channel binding value. cD10bHM
	 */
	r = gsasl_server_auth(session);
	gsasl_finish(session);
	gsasl_done(gsasl_ctx);
	if (!r && gsasl_pw->pw_gid & NO_RELAY)
		r = 3;
	if (!r || r == 3)
		authd = method;
	return (r);
}
#endif

static int
auth_cram_md5()
{
	return (auth_cram(AUTH_CRAM_MD5));
}

static int
auth_cram_sha1()
{
	return (auth_cram(AUTH_CRAM_SHA1));
}

static int
auth_cram_sha224()
{
	return (auth_cram(AUTH_CRAM_SHA224));
}

static int
auth_cram_sha256()
{
	return (auth_cram(AUTH_CRAM_SHA256));
}

static int
auth_cram_sha384()
{
	return (auth_cram(AUTH_CRAM_SHA384));
}

static int
auth_cram_sha512()
{
	return (auth_cram(AUTH_CRAM_SHA512));
}

static int
auth_cram_ripemd()
{
	return (auth_cram(AUTH_CRAM_RIPEMD));
}

#ifdef HASLIBGSASL
static int
auth_scram_sha1()
{
	return (auth_scram(AUTH_SCRAM_SHA1));
}

static int
auth_scram_sha1_plus()
{
	return (auth_scram(AUTH_SCRAM_SHA1_PLUS));
}

static int
auth_scram_sha256()
{
	return (auth_scram(AUTH_SCRAM_SHA256));
}

static int
auth_scram_sha256_plus()
{
	return (auth_scram(AUTH_SCRAM_SHA256_PLUS));
}

#if 0
static int
auth_scram_sha512()
{
	return (auth_scram(AUTH_SCRAM_SHA512));
}

static int
auth_scram_sha512_plus()
{
	return (auth_scram(AUTH_SCRAM_SHA512_PLUS));
}
#endif
#endif /*- #ifdef HASLIBGSASL */

/*- parse digest response */
unsigned int
scan_response(stralloc *dst, stralloc *src, const char *search)
{
	char           *x = src->s;
	int             i, len;
	unsigned int    slen;

	slen = str_len((char *) search);
	if (!stralloc_copys(dst, ""))
		die_nomem();
	for (i = 0; src->len > i + slen; i += str_chr(x + i, ',') + 1) {
		char           *s = x + i;
		if (case_diffb(s, slen, (char *) search) == 0) {
			s += slen;			/* skip name */
			if (*s++ != '=')
				return 0;		/* has to be here! */
			if (*s == '"') {	/* var="value" */
				s++;
				len = str_chr(s, '"');
				if (!len)
					return 0;
				if (!stralloc_catb(dst, s, len))
					die_nomem();
			} else {			/* var=value */
				len = str_chr(s, ',');
				if (!len)
					str_len(s);	/* should be the end */
				if (!stralloc_catb(dst, s, len))
					die_nomem();
			}
			return dst->len;
		}
	}
	return 0;
}

/*
 * RFC 2831
 *
 * sets all 3 variables: user\0pass\0resp\0
 */
char            hextab[] = "0123456789abcdef";

static int
auth_digest_md5()
{
	unsigned char   unique[FMT_ULONG + FMT_ULONG + 3];
	unsigned char   digest[20], encrypted[41];
	unsigned char  *s, *x = encrypted;
	int             i, r, len = 0; /*- qop = 1; */
	static stralloc tmp = { 0 }, nonce = {0};

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
	hmac_sha1(unique, len, unique + 3, len - 3, digest);	/* should be enough :) */
	for (i = 0; i < 20; i++) {
		*x = hextab[digest[i] / 16];
		++x;
		*x = hextab[digest[i] % 16];
		++x;
	}
	*x = 0;
	if (!stralloc_copys(&tmp, (char *) encrypted) ||
			b64encode(&tmp, &nonce) != 0 ||
			!stralloc_cats(&slop, "realm=\"") ||
			!stralloc_cats(&slop, hostname) ||
			!stralloc_cats(&slop, "\",nonce=\"") ||
			!stralloc_cat(&slop, &nonce) ||
			!stralloc_cats(&slop, "\",qop=\"auth\"") ||
			!stralloc_cats(&slop, ",algorithm=md5-sess") ||
			b64encode(&slop, &tmp) != 0)
		die_nomem();
	out("334 ");
	if (substdio_put(&ssout, tmp.s, tmp.len) == -1) {
		flush();
		return err_write();
	}
	out("\r\n");
	flush();

	/*- get digest-response */
	if (authgetl() < 0)
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (!stralloc_0(&slop))
		die_nomem();

	/*- scan slop for all required fields, fill resp for later auth.  */
	if (scan_response(&user, &slop, "username") == 0)
		return (err_input());
	if (scan_response(&tmp, &slop, "digest-uri") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "digest-uri=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- check nc field */
	if (scan_response(&tmp, &slop, "nc") == 0)
		return (err_input());
	if (tmp.len != 8)
		return (err_input());
	if (case_diffb("00000001", 8, tmp.s) != 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nnc=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- check nonce */
	if (scan_response(&tmp, &slop, "nonce") == 0)
		return (err_input());
	if (tmp.len != nonce.len)
		return (err_input());
	if (case_diffb(nonce.s, tmp.len, tmp.s) != 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nnonce=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- check cnonce */
	if (scan_response(&tmp, &slop, "cnonce") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\ncnonce=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- check qop */
	if (scan_response(&tmp, &slop, "qop") == 0)
		return (err_input());
	switch (tmp.len)
	{
	case 4:	/*- qop=1; */
		if (case_diffb("auth", 4, tmp.s) != 0)
			return (err_input());
		break;
	case 8:	/*- qop=2; */
		if (case_diffb("auth-int", 8, tmp.s) != 0)
			return (err_input());
		break;
	case 9:	/*- qop=3; */
		if (case_diffb("auth-conf", 9, tmp.s) != 0)
			return (err_input());
		break;
	default:
		return (err_input());
	}
	if (!stralloc_cats(&resp, "\nqop=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- xxx: todo / check realm against control/realms or so ?!  */
	if (scan_response(&tmp, &slop, "realm") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nrealm=") ||
			!stralloc_cat(&resp, &tmp))
		die_nomem();

	/*- check response */
	if (scan_response(&pass, &slop, "response") == 0)
		return (err_input());
	if (pass.len != 32)
		return (err_input());

	/*
	 * user=username
	 * pass=response (md5 hash = 32)
	 * resp=authfile (with all required vars for the checkpassword utility)
	 *      -> nc,qop,realm,nonce,cnonce,digesturi
	 *
	 *             a1 = md5(user:realm:pass) : nonce : cnonce
	 * qop=auth:   a2 = 'AUTHENTICATE' : digesturi
	 * qop=auth-*: a2 = 'AUTHENTICATE' : digesturi : '00000000000000000000000000000000'
	 * resp = md5(a1) + nonce + nc + conce + qop + md5(a2)
	 */
	if (!user.len || !pass.len || !resp.len)
		return (err_input());
	r = authenticate(AUTH_DIGEST_MD5);
	if (!r || r == 3)
		authd = AUTH_DIGEST_MD5;
	return (r);
}

void
smtp_auth(char *arg)
{
	int             i, j;
	char           *cmd = arg;

	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		flush();
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!hostname || !*hostname || !childargs || !*childargs) {
		out("503 auth not available (#5.3.3)\r\n");
		flush();
		return;
	}
	if (authd) {
		err_authd();
		return;
	}

	if (seenmail) {
		err_transaction("auth");
		return;
	}
	if (!stralloc_copys(&user, "") ||
			!stralloc_copys(&pass, "") ||
			!stralloc_copys(&resp, ""))
		die_nomem();
	i = str_chr(cmd, ' ');
	arg = cmd + i;
	while (*arg == ' ')
		++arg;
	cmd[i] = 0;
	for (i = 0; authcmds[i].text; ++i) {
		if (case_equals(authcmds[i].text, cmd))
			break;
	}
	switch ((j = authcmds[i].fun(arg)))
	{
	case 0:
		relayclient = "";
	case 3:/*- relayclient is not set, relaying is denied */
		remoteinfo = user.s;
		if (!env_unset("TCPREMOTEINFO"))
			die_nomem();
		if (!env_put2("TCPREMOTEINFO", remoteinfo))
			die_nomem();
		if (!env_put2("AUTHINFO", remoteinfo))
			die_nomem();
		out("235 ok, go ahead (#2.0.0)\r\n");
		flush();
		break;
	case 1:/*- auth fail */
	case 2:/*- misuse */
		err_authfailure(remoteip, user.len ? user.s : 0, j);
		sleep(5);
		out("553 authorization failure (#5.7.1)\r\n");
		flush();
		break;
	case -1:
		err_authfailure(remoteip, user.len ? user.s : 0, j);
		out("454 temporary authentication failure (#4.3.0)\r\n");
		flush();
		break;
	case -2: /*- returned by err_noauthallowed() when SECURE_AUTH is set and TLS isn't used */
		err_authinsecure(remoteip, j);
		break;
	default:
		err_child();
		break;
	}
	return;
}

void
smtp_etrn(char *arg)
{
	int             status, i;
	char            tmpbuf[1024], err_buff[1024], status_buf[FMT_ULONG]; /*- needed for SIZE CMD */

	if (!*arg) {
		err_syntax();
		return;
	}
	if (!seenhelo) {
		out("503 Polite people say hello first (#5.5.4)\r\n");
		flush();
		return;
	}
	if (seenmail) {
		err_transaction("ETRN");
		return;
	}
	if (!isalnum((int) *arg))
		arg++;
	if (!valid_hostname(arg)) {
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		flush();
		return;
	}
	if (!nodnscheck) {
		i = fmt_str(tmpbuf, "@");
		i += fmt_strn(tmpbuf + i, arg, 1022);
		if (i > 1023)
			die_nomem();
		tmpbuf[i] = 0;
		switch (dnscheck(tmpbuf, i, 1))
		{
		case DNS_HARD:
			err_hmf(remoteip, tmpbuf, 1);
			return;
		case DNS_SOFT:
			err_smf();
			return;
		case DNS_MEM:
			die_nomem();
		}
	}
	/*-
	 * XXX The implementation borrows heavily from the code that implements
	 * UCE restrictions. These typically return 450 or 550 when a request is
	 * rejected. RFC 1985 requires that 459 be sent when the server refuses
	 * to perform the request.
	 */
	switch ((status = etrn_queue(arg, remoteip)))
	{
	case 0:
		log_etrn(remoteip, arg, 0);
		out("250 OK, queueing for node <");
		out(arg);
		out("> started\r\n");
		flush();
		return;
	case -1:
		log_etrn(remoteip, arg, "ETRN Error");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		flush();
		return;
	case -2:
		log_etrn(remoteip, arg, "ETRN Rejected");
		out("553 <");
		out(arg);
		out(">: etrn service unavailable (#5.7.1)\r\n");
		flush();
		return;
	case -3:
		out("250 OK, No message waiting for node <");
		out(arg);
		out(">\r\n");
		flush();
		return;
	case -4:
		out("252 OK, pending message for node <");
		out(arg);
		out("> started\r\n");
		flush();
		return;
	default:
		status_buf[fmt_ulong(status_buf, (unsigned long) status)] = 0;
		if (status > 0) {
			out("253 OK, <");
			out(status_buf);
			out("> pending message for node <");
			out(arg);
			out("> started\r\n");
			flush();
			return;
		}
		i = fmt_str(err_buff, "unable to talk to fast flush service status <");
		i += fmt_ulong(err_buff + i, (unsigned long) status);
		if (i > 1023)
			die_nomem();
		i += fmt_str(err_buff + i, ">");
		if (i > 1023)
			die_nomem();
		err_buff[i] = 0;
		log_etrn(remoteip, arg, err_buff);
		out("451 Unable to queue messages, status <");
		out(status_buf);
		out("> (#4.3.0)\r\n");
		flush();
		return;
	}
	return;
}

void
smtp_atrn(char *arg)
{
	char           *ptr, *cptr, *domain_ptr, *user_tmp, *domain_tmp, *errstr;
	int             i, end_flag, status, Reject = 0, Accept = 0;
	char            err_buff[1024], status_buf[FMT_ULONG]; /*- needed for SIZE CMD */
	static stralloc a_user = {0}, domain = {0};
	void            (*iclose) (void);
	char           *(*show_atrn_map) (char **, char **);
	int             (*atrn_access) (char *, char *);
	int             (*parse_email) (char *, stralloc *, stralloc *);

	if (!authd) {
		err_authrequired();
		return;
	}
	if (!seenhelo) {
		out("503 Polite people say hello first (#5.5.4)\r\n");
		flush();
		return;
	}
	if (seenmail) {
		err_transaction("ATRN");
		return;
	}
	if (!(ptr = load_virtual()))
		return;
	if (!(iclose = getlibObject(ptr, &phandle, "iclose", &errstr))) {
		err_library(errstr);
		return;
	} else
	if (!(show_atrn_map = getlibObject(ptr, &phandle, "show_atrn_map", &errstr))) {
		err_library(errstr);
		return;
	} else
	if (!(atrn_access = getlibObject(ptr, &phandle, "atrn_access", &errstr))) {
		err_library(errstr);
		return;
	}
	domBuf.len = 0;
	for (; *arg && !isalnum((int) *arg); arg++);
	if (*arg)
		domain_ptr = arg;
	else {
		if (!(parse_email = getlibObject(ptr, &phandle, "parse_email", &errstr))) {
			err_library(errstr);
			return;
		}
		(*parse_email) (remoteinfo, &a_user, &domain);
		for (user_tmp = a_user.s, domain_tmp = domain.s, end_flag = 0;;) {
			if (!(ptr = (*show_atrn_map) (&user_tmp, &domain_tmp)))
				break;
			if (end_flag) {
				if (!stralloc_cats(&domBuf, " ")) {
					(*iclose) ();
					die_nomem();
				}
			}
			if (!stralloc_cats(&domBuf, ptr)) {
				(*iclose) ();
				die_nomem();
			}
			end_flag = 1;
		}
		if (!stralloc_0(&domBuf)) {
			(*iclose) ();
			die_nomem();
		}
		domain_ptr = domBuf.s;
	}
	for (cptr = domain_ptr;; cptr++) {
		if (*cptr == ' ' || *cptr == ',' || !*cptr) {
			if (*cptr) {
				end_flag = 0;
				*cptr = 0;
			} else
				end_flag = 1;
			if (!valid_hostname(arg)) {
				out("501 invalid parameter syntax (#5.3.2)\r\n");
				flush();
				return;
			}
			if ((*atrn_access) (remoteinfo, domain_ptr)) {
				Reject = 1;
				break;
			} else
				Accept = 1;
			if (end_flag)
				break;
			else
				*cptr = ' ';
			domain_ptr = cptr + 1;
		}
	}
	(*iclose) ();
	if (Reject) {
		log_atrn(remoteip, remoteinfo, domain_ptr, "ATRN Rejected");
		if (Accept)
			out("450 atrn service unavailable (#5.7.1)\r\n");
		else
			out("553 atrn service unavailable (#5.7.1)\r\n");
		flush();
		return;
	}
	switch ((status = atrn_queue(arg, remoteip)))
	{
	case 0:
		log_atrn(remoteip, remoteinfo, arg, 0);
		out("QUIT\r\n");
		flush();
		_exit(0);
	case -1:
		log_atrn(remoteip, remoteinfo, arg, "ATRN Error");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		flush();
		return;
	case -2:
		log_atrn(remoteip, remoteinfo, arg, "ATRN Rejected");
		out("553 <");
		out(arg);
		out(">: atrn service unavailable (#5.7.1)\r\n");
		flush();
		return;
	case -3:
		out("453 No message waiting for node(s) <");
		out(arg);
		out(">\r\n");
		flush();
		return;
	case -4:
		out("451 Unable to queue messages (#4.3.0)\r\n");
		flush();
		return;
	default:
		status_buf[fmt_ulong(status_buf, (unsigned long) status)] = 0;
		if (status > 0) {
			i = fmt_str(err_buff, "unable to talk to fast flush service status <");
			i += fmt_ulong(err_buff + i, (unsigned long) status);
			if (i > 1023)
				die_nomem();
			i += fmt_str(err_buff + i, ">");
			if (i > 1023)
				die_nomem();
			err_buff[i] = 0;
			log_atrn(remoteip, remoteinfo, arg, err_buff);
			out("451 Unable to queue messages, status <");
			out(status_buf);
			out("> (#4.3.0)\r\n");
			flush();
		}
		return;
	}
	return;
}

#ifdef TLS
int             ssl_verified = 0;
const char     *ssl_verify_err = 0;

void
smtp_tls(char *arg)
{
	if (ssl)
		err_unimpl("unimplimented");
	else
	if (*arg) {
		out("501 Syntax error (no parameters allowed) (#5.5.4)\r\n");
		flush();
	} else {
		tls_init();
		/*- have to discard the pre-STARTTLS HELO/EHLO argument, if any */
		dohelo(remotehost);
	}
}

#if OPENSSL_VERSION_NUMBER < 0x30000000L
RSA            *
tmp_rsa_cb(SSL *ssl_p, int export, int keylen)
{
	stralloc        filename = { 0 };
	char           *pems[] = {"8192", "4096", "2048", "1024", "512", 0};
	char          **ptr;
	int             i, j;
	FILE           *in;
	RSA            *rsa;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	BIGNUM         *bn;
	RSA            *key;
#endif

	if (!export)
		keylen = 512;
	if (!certdir) {
		if (!(certdir = env_get("CERTDIR")))
			certdir = auto_control;
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/rsa", 4))
		die_nomem();
	j = filename.len;
	for (ptr = pems; *ptr; ptr++) {
		scan_int(*ptr, &i);
		if (keylen != i)
			continue;
		if (!stralloc_cats(&filename, *ptr) ||
				!stralloc_catb(&filename, ".pem", 4) ||
				!stralloc_0(&filename))
			die_nomem();
		filename.len = j;
		if (!(in = fopen(filename.s, "r"))) {
			if (errno != error_noent)
				die_custom("error reading rsa private key");
			continue;
		}
		if (!(rsa = PEM_read_RSAPrivateKey(in, NULL, NULL, NULL)))
			die_custom("error reading rsa private key");
		fclose(in);
		alloc_free(filename.s);
		return rsa;
	}
	alloc_free(filename.s);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!(bn = BN_new()))
		die_nomem();
	if (!BN_set_word(bn, 65537))
		die_custom("error setting BIGNUM");
	if (!(key = RSA_new()))
		die_nomem();
	if (!RSA_generate_key_ex(key, keylen, bn, NULL))
		die_custom("error generating RSA keys");
	BN_free(bn);
	return (key);
#else
	return RSA_generate_key(keylen, RSA_F4, NULL, NULL);
#endif
}

DH             *
tmp_dh_cb(SSL *ssl_p, int export, int keylen)
{
	stralloc        filename = { 0 };
	char           *pems[] = {"8192", "4096", "2048", "1024", "512", 0};
	char          **ptr;
	int             i, j;
	FILE           *in;
	DH             *dh;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	DH             *client_key;
#endif

	if (!export)
		keylen = 1024;
	if (!certdir) {
		if (!(certdir = env_get("CERTDIR")))
			certdir = auto_control;
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/dh", 3))
		die_nomem();
	j = filename.len;
	for (ptr = pems; *ptr; ptr++) {
		scan_int(*ptr, &i);
		if (keylen != i)
			continue;
		if (!stralloc_cats(&filename, *ptr) ||
				!stralloc_catb(&filename, ".pem", 4) ||
				!stralloc_0(&filename))
			die_nomem();
		filename.len = j;
		if (!(in = fopen(filename.s, "r"))) {
			if (errno != error_noent)
				die_custom("error reading dh parameters");
			continue;
		}
		if (!(dh = PEM_read_DHparams(in, NULL, NULL, NULL)))
			die_custom("error reading dh parameters");
		fclose(in);
		alloc_free(filename.s);
		return dh;
	}
	alloc_free(filename.s);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!(client_key = DH_new()))
		die_nomem();
	if (!DH_generate_parameters_ex(client_key, keylen, DH_GENERATOR_2, NULL))
		die_custom("error generating DH parameters");
	return (client_key);
#else
	return DH_generate_parameters(keylen, DH_GENERATOR_2, NULL, NULL);
#endif
}
#else /*- #if OPENSSL_VERSION_NUMBER < 0x30000000L */
EVP_PKEY       *
get_rsakey(int export, int keylen)
{
	stralloc        filename = { 0 };
	char           *pems[] = {"8192", "4096", "2048", "1024", "512", 0};
	char          **ptr;
	BIO            *in;
	int             i, j;
	EVP_PKEY       *rsa_key;

	if (!export)
		keylen = 512;
	if (!certdir) {
		if (!(certdir = env_get("CERTDIR")))
			certdir = auto_control;
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/rsa", 4))
		die_nomem();
	j = filename.len;
	for (ptr = pems; *ptr; ptr++) {
		scan_int(*ptr, &i);
		if (keylen != i)
			continue;
		if (!stralloc_cats(&filename, *ptr) ||
				!stralloc_catb(&filename, ".pem", 4) ||
				!stralloc_0(&filename))
			die_nomem();
		filename.len = j;
		if (access(filename.s, F_OK)) {
			if (errno != error_noent)
				die_custom("error reading rsa private key");
			continue;
		}
		if ((in = BIO_new(BIO_s_file()))) {
			BIO_read_filename(in, filename.s);
			alloc_free(filename.s);
			if (!(rsa_key = PEM_read_bio_PrivateKey(in, NULL, 0, NULL)))
				die_custom("error reading rsa private key");
			BIO_free(in);
			return rsa_key;
		} else
			die_nomem();
	}
	alloc_free(filename.s);
	return ((EVP_PKEY *) NULL);
}

EVP_PKEY       *
get_dhkey(int export, int keylen)
{
	stralloc        filename = { 0 };
	char           *pems[] = {"8192", "4096", "2048", "1024", "512", 0};
	char          **ptr;
	int             i, j;
	BIO            *in;
	EVP_PKEY       *dh_key;

	if (!export)
		keylen = 1024;
	if (!certdir) {
		if (!(certdir = env_get("CERTDIR")))
			certdir = auto_control;
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/dh", 3))
		die_nomem();
	j = filename.len;
	for (ptr = pems; *ptr; ptr++) {
		scan_int(*ptr, &i);
		if (keylen == i)
			continue;
		if (!stralloc_cats(&filename, *ptr) ||
				!stralloc_catb(&filename, ".pem", 4) ||
				!stralloc_0(&filename))
			die_nomem();
		filename.len = j;
		if (access(filename.s, F_OK)) {
			if (errno != error_noent)
				die_custom("error reading dh parameters");
			continue;
		}
		if ((in = BIO_new(BIO_s_file()))) {
			if (!BIO_read_filename(in, filename.s))
				die_custom("error reading dhparams file");
			alloc_free(filename.s);
			if (!(dh_key = PEM_read_bio_Parameters_ex(in, NULL, NULL, NULL)))
				die_custom("error reading dh parameters");
			BIO_free(in);
			return dh_key;
		} else
			die_nomem();
	}
	return ((EVP_PKEY *) NULL);
}
#endif /*- #if OPENSSL_VERSION_NUMBER < 0x30000000L */

/*
 * don't want to fail handshake if cert isn't verifiable 
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX * ctx)
{
	return 1;
}

void
tls_nogateway()
{
	/*- there may be cases when relayclient is set */
	if (relayclient)
		return;
	out("; no valid cert for gateway");
	if (ssl_verify_err) {
		out(": ");
		out((char *) ssl_verify_err);
	}
	out(" ");
	flush();
	return;
}

void
tls_err(const char *err_code1, const char *err_code2, const char *s)
{
	const char     *err;

	out(err_code1);
	out(" ");
	out(s);
	out(" (#");
	out(err_code2);
	out(")\r\n");
	flush();
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" ");
	logerr(s);
	err = ssl_error();
	if (err) {
		logerr(": ");
		logerr(err);
	}
	logerrf("\n");
	return;
}

int
tls_verify()
{
	stralloc        clients = { 0 }, filename = { 0 };
	struct constmap mapclients;

	if (!ssl || relayclient || ssl_verified)
		return 0;
	ssl_verified = 1;	/*- don't do this twice */
	/*-
	 * request client cert to see if it can be verified by one of our CAs
	 * and the associated email address matches an entry in tlsclients 
	 */
	switch (control_readfile(&clients, "tlsclients", 0))
	{
	case 1:
		if (!constmap_init(&mapclients, clients.s, clients.len, 0))
			die_nomem();
		/*-
		 * if clientca.pem contains all the standard root certificates, a
		 * 0.9.6b client might fail with SSL_R_EXCESSIVE_MESSAGE_SIZE;
		 * it is probably due to 0.9.6b supporting only 8k key exchange
		 * data while the 0.9.6c release increases that limit to 100k 
		 */
		if (!certdir) {
			if (!(certdir = env_get("CERTDIR")))
				certdir = auto_control;
		}
		if (!stralloc_copys(&filename, certdir) ||
				!stralloc_catb(&filename, "/", 1))
			die_nomem();
		clientca = ((clientca = env_get("CLIENTCA")) ? clientca : "clientca.pem");
		if (!stralloc_cats(&filename, clientca) ||
				!stralloc_0(&filename))
			die_nomem();
		STACK_OF(X509_NAME) *sk = SSL_load_client_CA_file(filename.s);
		alloc_free(filename.s);
		constmap_free(&mapclients);
		alloc_free(clients.s);
		if (sk) {
			SSL_set_client_CA_list(ssl, sk);
			SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, NULL);
			break;
		}
	case 0:
		alloc_free(clients.s);
		return 0;
	case -1:
		die_control();
	}

	if (ssl_timeoutrehandshake(timeout, ssl_rfd, ssl_wfd, ssl) <= 0) {
		tls_err("454", "4.3.0", "rehandshake failed");
		_exit(1);
	}

	do { /*- one iteration */
		X509           *peercert;
		X509_NAME      *subj;
		stralloc        email = { 0 };

		int             n = SSL_get_verify_result(ssl);
		if (n != X509_V_OK) {
			ssl_verify_err = X509_verify_cert_error_string(n);
			break;
		}
		if (!(peercert = SSL_get_peer_certificate(ssl)))
			break;
		subj = X509_get_subject_name(peercert);
		if ((n = X509_NAME_get_index_by_NID(subj, NID_pkcs9_emailAddress, -1)) >= 0) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			X509_NAME_ENTRY *t;
			t = X509_NAME_get_entry(subj, n);
			ASN1_STRING    *s = X509_NAME_ENTRY_get_data(t);
#else
			const ASN1_STRING *s = X509_NAME_get_entry(subj, n)->value;
#endif
			if (s) {
				email.len = s->length;
				email.s = (char *) s->data;
			}
		}
		if (email.len <= 0)
			ssl_verify_err = "contains no email address";
		else
		if (!constmap(&mapclients, email.s, email.len))
			ssl_verify_err = "email address not in my list of tlsclients";
		else { /*- add the cert email to the proto if it helped allow relaying */
			if (!stralloc_cats(&proto, "\n  (cert ") || /*- continuation line */
					!stralloc_catb(&proto, email.s, email.len) ||
					!stralloc_cats(&proto, ")"))
				die_nomem();
			authenticated = 1;
			if (!env_put2("AUTHENTICATED", "1"))
				die_nomem();
			relayclient = "";
		}
		X509_free(peercert);
	} while (0);
	constmap_free(&mapclients);
	/*- we are not going to need this anymore: free the memory */
	SSL_set_client_CA_list(ssl, NULL);
	SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
	return relayclient ? 1 : 0;
}

SSL_CTX        *
set_tls_method()
{
	SSL_CTX        *ctx;
	stralloc        ssl_option = { 0 };
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	int             method = 4;	/*- (1 unused) 2 = SSLv23, 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2 */
#else
	int             method = 0;	/*- (1,2 unused) 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2, 7=TLSv1.3 */
#endif

	if (control_rldef(&ssl_option, "tlsservermethod", 0, "TLSv1_2") != 1)
		die_control();
	if (!stralloc_0(&ssl_option))
		die_nomem();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (str_equal(ssl_option.s, "SSLv23"))
		method = 2;
	else
	if (str_equal(ssl_option.s, "SSLv3"))
		method = 3;
	else
#endif
	if (str_equal(ssl_option.s, "TLSv1"))
		method = 4;
	else
	if (str_equal(ssl_option.s, "TLSv1_1"))
		method = 5;
	else
	if (str_equal(ssl_option.s, "TLSv1_2"))
		method = 6;
	else
	if (str_equal(ssl_option.s, "TLSv1_3"))
		method = 7;
	else {
		tls_err("454", "4.3.0", "Invalid TLS method configured");
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		logerrf("Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2\n");
#else
		logerrf("Supported methods: TLSv1, TLSv1_1, TLSv1_2 TLSv1_3\n");
#endif
		return ((SSL_CTX *) NULL);
	}
	SSL_library_init();
	/*- a new SSL context with the bare minimum of options */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (method == 2 && !(ctx = SSL_CTX_new(SSLv23_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize SSLv23 ctx");
		return ((SSL_CTX *) NULL);
	} else
	if (method == 3 && !(ctx = SSL_CTX_new(SSLv3_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize SSLv3 ctx");
		return ((SSL_CTX *) NULL);
	} else
#if defined(TLSV1_SERVER_METHOD) || defined(TLS1_VERSION)
	if (method == 4 && !(ctx = SSL_CTX_new(TLSv1_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize TLSv1 ctx");
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 4) {
		tls_err("454", "4.3.0", "TLS not available: TLSv1_server_method not available");
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_1_SERVER_METHOD) || defined(TLS1_1_VERSION)
	if (method == 5 && !(ctx = SSL_CTX_new(TLSv1_1_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize TLSv1_1 ctx");
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 5) {
		tls_err("454", "4.3.0", "TLS not available: TLSv1_1_server_method not available");
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_2_SERVER_METHOD) || defined(TLS1_2_VERSION)
	if (method == 6 && !(ctx = SSL_CTX_new(TLSv1_2_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize TLSv1_2 ctx");
		return ((SSL_CTX *) NULL) ;
	} else
#else
	if (method == 6) {
		tls_err("454", "4.3.0", "TLS not available: TLSv1_2_server_method not available");
		return ((SSL_CTX *) NULL) ;
	}
#endif
#else /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	if (!(ctx = SSL_CTX_new(TLS_server_method()))) {
		tls_err("454", "4.3.0", "TLS not available: unable to initialize TLS ctx");
		return ((SSL_CTX *) NULL) ;
	}
	/*-
	 * Currently supported versions are SSL3_VERSION, TLS1_VERSION, TLS1_1_VERSION,
	 * TLS1_2_VERSION, TLS1_3_VERSION for TLS
	 */
	switch (method)
	{
	case 2:
		tls_err("454", "4.3.0", "TLS not available: SSLv23_server_method not available");
		return ((SSL_CTX *) NULL) ;
	case 3:
		SSL_CTX_set_min_proto_version(ctx, SSL3_VERSION);
		SSL_CTX_set_max_proto_version(ctx, SSL3_VERSION);
		break;
	case 4:
		SSL_CTX_set_min_proto_version(ctx, TLS1_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_VERSION);
		break;
	case 5:
		SSL_CTX_set_min_proto_version(ctx, TLS1_1_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_1_VERSION);
		break;
	case 6:
		SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
		SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
		break;
	}
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
#endif /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	return ctx;
}

void
log_ssl_version()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" ");
	logerr("ssl-version=");
	logerr(SSL_get_version(ssl));
	logerrf("\n");
	return;
}

void
tls_init()
{
	const char     *ciphers;
	SSL            *myssl;
	SSL_CTX        *ctx;
	X509_STORE     *store;
	X509_LOOKUP    *lookup;
	stralloc        saciphers = { 0 };
	stralloc        filename = { 0 };
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_PKEY       *dh_pkey;
	int             bits;
#endif

	if (!(ctx = set_tls_method()))
		return;
	if (!certdir) {
		if (!(certdir = env_get("CERTDIR")))
			certdir = auto_control;
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/", 1))
		die_nomem();
	servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
	if (!stralloc_cats(&filename, servercert) ||
			!stralloc_0(&filename))
		die_nomem();
	if (!SSL_CTX_use_certificate_chain_file(ctx, filename.s)) {
		alloc_free(filename.s);
		SSL_CTX_free(ctx);
		tls_err("454", "4.3.0", "certificate missing");
		_exit(1);
	}
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/", 1))
		die_nomem();
	clientca = ((clientca = env_get("CLIENTCA")) ? clientca : "clientca.pem");
	if (!stralloc_cats(&filename, clientca) ||
			!stralloc_0(&filename))
		die_nomem();
	SSL_CTX_load_verify_locations(ctx, filename.s, NULL);
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
	/*- crl checking */
	store = SSL_CTX_get_cert_store(ctx);
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/", 1))
		die_nomem();
	clientcrl = ((clientcrl = env_get("CLIENTCRL")) ? clientcrl : "clientcrl.pem");
	if (!stralloc_cats(&filename, clientcrl) ||
			!stralloc_0(&filename))
		die_nomem();
	if ((lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file()))
		&& (X509_load_crl_file(lookup, filename.s, X509_FILETYPE_PEM) == 1))
		X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
	/*- set the callback here; SSL_set_verify didn't work before 0.9.6c */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_cb);
	/*- a new SSL object, with the rest added to it directly to avoid copying */
	myssl = SSL_new(ctx);
	SSL_CTX_free(ctx);
	if (!myssl) {
		alloc_free(filename.s);
		tls_err("454", "4.3.0", "unable to initialize ssl");
		_exit(1);
	}
	/*- this will also check whether public and private keys match */
	if (!stralloc_copys(&filename, certdir) ||
			!stralloc_catb(&filename, "/", 1))
		die_nomem();
	servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
	if (!stralloc_cats(&filename, servercert) ||
			!stralloc_0(&filename))
		die_nomem();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!SSL_use_PrivateKey_file(myssl, filename.s, SSL_FILETYPE_PEM)) {
#else
	if (!SSL_use_RSAPrivateKey_file(myssl, filename.s, SSL_FILETYPE_PEM)) {
#endif
		SSL_free(myssl);
		alloc_free(filename.s);
		tls_err("454", "4.3.0", "no valid RSA private key");
		_exit(1);
	}
	alloc_free(filename.s);
	if (!(ciphers = env_get("TLS_CIPHER_LIST"))) {
		if (control_readfile(&saciphers, "tlsserverciphers", 0) == -1) {
			SSL_free(myssl);
			die_control();
		}
		if (saciphers.len) {
			int             i;
			/*- convert all '\0's except the last one to ':' */
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		}
	}
	if (!ciphers || !*ciphers)
		ciphers = "DEFAULT";
	SSL_set_cipher_list(myssl, ciphers);
	alloc_free(saciphers.s);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	getEnvConfigInt(&bits, "SSL_BITS", 2048);
	if (!get_rsakey(0, bits))
		EVP_RSA_gen(bits);
	if (!(dh_pkey = get_dhkey(0, bits)))
		SSL_set_dh_auto(myssl, 1);
	else
		SSL_set0_tmp_dh_pkey(myssl, dh_pkey);
#else
	SSL_set_tmp_rsa_callback(myssl, tmp_rsa_cb);
	SSL_set_tmp_dh_callback(myssl, tmp_dh_cb);
#endif
	SSL_set_rfd(myssl, ssl_rfd = substdio_fileno(&ssin));
	SSL_set_wfd(myssl, ssl_wfd = substdio_fileno(&ssout));
	if (!smtps) {
		out("220 ready for tls\r\n");
		flush();
	}
	if (ssl_timeoutaccept(timeout, ssl_rfd, ssl_wfd, myssl) <= 0) {
		/*- neither cleartext nor any other response here is part of a standard */
		tls_err("454", "4.3.0", "failed to accept TLS connection");
		while (SSL_shutdown(myssl) == 0)
			usleep(100);
		SSL_free(myssl);
		myssl = 0;
		_exit(1);
	}
	ssl = myssl;
	log_ssl_version();
	/*- populate the protocol string, used in Received */
	if (!stralloc_append(&proto, "(") ||
			!stralloc_cats(&proto, (char *) SSL_get_version(ssl)) ||
			!stralloc_catb(&proto, " ", 1) ||
			!stralloc_cats(&proto, (char *) SSL_CIPHER_get_name(SSL_get_current_cipher(ssl))) ||
			!stralloc_catb(&proto, " encrypted) ", 12))
		die_nomem();
	return;
}
#endif

struct commands smtpcommands[] = {
	{"rcpt", smtp_rcpt, 0},
	{"mail", smtp_mail, 0},
	{"data", smtp_data, flush},
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"rset", smtp_rset, 0},
	{"help", smtp_help, flush},
	{"noop", smtp_noop, flush},
	{"vrfy", smtp_vrfy, flush},
	{"etrn", smtp_etrn, flush},
	{"atrn", smtp_atrn, flush},
#ifdef TLS
	{"starttls", smtp_tls, flush_io},
#endif
	{0, err_unimpl, flush}
};

struct commands odmrcommands[] = {
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"help", smtp_help, flush},
	{"etrn", smtp_etrn, flush},
	{"atrn", smtp_atrn, flush},
	{0, err_unimpl, flush}
};

struct commands submcommands[] = {
	{"rcpt", smtp_rcpt, 0},
	{"mail", smtp_mail, 0},
	{"data", smtp_data, flush},
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"rset", smtp_rset, 0},
	{"help", smtp_help, flush},
	{"noop", smtp_noop, flush},
	{"vrfy", smtp_vrfy, flush},
#ifdef TLS
	{"starttls", smtp_tls, flush_io},
#endif
	{0, err_unimpl, flush}
};

#ifdef SMTP_PLUGIN
void
load_plugin(char *library, char *plugin_symb, int j)
{
	PLUGIN         *(*func) (void);
	char           *error;

#ifdef RTLD_DEEPBIND
	if (!(plughandle[j] = dlopen(library, RTLD_LAZY|RTLD_LOCAL|RTLD_NODELETE|RTLD_DEEPBIND)))
#else
	if (!(plughandle[j] = dlopen(library, RTLD_LAZY|RTLD_LOCAL|RTLD_NODELETE)))
#endif
		die_plugin("dlopen failed for ", library, ": ", dlerror());
	dlerror(); /*- man page told me to do this */
	*(void **) (&func) = dlsym(plughandle[j], plugin_symb);
	if ((error = dlerror()))
		die_plugin("dlsym ", plugin_symb, " failed: ", error);
	/*- execute the function */
	if (!(plug[j] = (*func) ()))	 /*- this function returns a pointer to PLUGIN */
		die_plugin("function ", plugin_symb, " failed", 0);
	return;
}
#endif

void
qmail_smtpd(int argc, char **argv, char **envp)
{
	char           *ptr, *errstr;
	struct commands *cmdptr;
	int             i;
#ifdef SMTP_PLUGIN
	int             j, len;
	char           *start_plugin, *plugin_symb, *plugindir;
	static stralloc plugin = { 0 };
#endif

	if (argc > 2) {
		hostname = argv[1];
		childargs = argv + 2;
	}
	old_client_bug = env_get("OLD_CLIENT") ? 1 : 0;
	no_help = env_get("DISABLE_HELP");
	no_vrfy = env_get("DISABLE_VRFY");
	setup_state = 0;
	if ((ptr = env_get("TCPLOCALPORT")))
		scan_int(ptr, &smtp_port);
	else
		smtp_port = -1;
	if (smtp_port == ODMR_PORT && (!hostname || !*hostname || !childargs || !*childargs)) {
		if (!env_put("SHUTDOWN=1"))
			die_nomem();
	}
	if (envp)
		environ = envp;
	sig_termcatch(sigterm);
	sig_pipeignore();
	/*- load virtual package library */
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir) ||
					(libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/")) ||
					!stralloc_catb(&libfn, "libindimail", 11) ||
					!stralloc_0(&libfn))
				die_nomem();
		}
		ptr = libfn.s;
	} else
		ptr = "VIRTUAL_PKG_LIB";
	loadLibrary(&phandle, ptr, &i, &errstr);
	if (i) {
		logerr("Error loading virtual package shared lib: ");
		logerr(errstr);
		logerrf("\n");
		out("451 Requested action aborted: unable to load shared library (#4.3.0)\r\n");
		flush();
		_exit(1);
	}
	hasvirtual = phandle ? 1 : 0;
	/*-
	 * setup calls databytes_setup(), which sets
	 * databytes
	 */
	setup(); /*- remoteip is set */
	if (ipme_init() != 1)
		die_ipme();
	if (greetdelay)
		greetdelay_check(greetdelay);
	if ((ptr = env_get("SHUTDOWN"))) {
		if (*ptr) {
			if (!isdigit(ptr[0]) || !isdigit(ptr[1]) || !isdigit(ptr[2])) {
				smtp_greet("421 ");
			} else {
				str_copyb(strnum, ptr, 4);
				strnum[4] = 0;
				smtp_greet(strnum);
				out(ptr + 3);
			}
		} else {
			smtp_greet("421 ");
			out(" SMTP service unavailable (#4.3.2)");
		}
		setup_state = 1;
	} else {
		if (dobadipcheck && badipcheck(remoteip)) {
			smtp_greet("421 ");
			out(" sorry, your IP (");
			out(remoteip);
			out(") is temporarily denied (#4.7.1)");
			setup_state = 6;
		} else
		if (dobadhostcheck && badhostcheck()) {
			smtp_greet("553 ");
			out(" sorry, your host (");
			out(remotehost);
			out(") has been denied (#5.7.1)");
			setup_state = 5;
		} else
		if (env_get("OPENRELAY")) {
			smtp_greet("553 ");
			out(" No mail accepted from an open relay (");
			out(remoteip);
			out("); check your server configs (#5.7.1)");
			setup_state = 2;
		} else
		if ((ptr = env_get("TCPPARANOID"))) {
			smtp_greet("553 ");
			out(" sorry, your IP address (");
			out(remoteip);
			if (*ptr) {
				out(") PTR (reverse DNS) record points to wrong hostname ");
				out(ptr);
				out(" (#5.7.1)");
			} else
				out(") PTR (reverse DNS) record points to wrong hostname (#5.7.1)");
			setup_state = 3;
		} else
		if ((ptr = env_get("REQPTR")) && str_equal(remotehost, "unknown")) {
			smtp_greet("553 ");
			if (*ptr) {
				out(" ");
				out(ptr);
				out(": from ");
				out(remoteip);
				out(": (#5.7.1)");
			} else {
				out(" Sorry, no PTR (reverse DNS) record for (");
				out(remoteip);
				out(") (#5.7.1)");
			}
			setup_state = 4;
		} else
			smtp_greet("220 ");
	}
	out("\r\n");
	flush();
	switch (smtp_port)
	{
	case ODMR_PORT:/*- RFC 2645 */
		cmdptr = odmrcommands;
		break;
	case SUBM_PORT:/*- RFC 2476 */
		cmdptr = submcommands;
		break;
	case SMTP_PORT:
	default:
		cmdptr = smtpcommands;
		break;
	}
#ifdef SMTP_PLUGIN
	if (env_get("DISABLE_PLUGIN"))
		goto command;
	if (!(plugin_symb = env_get("SMTP_PLUGIN_SYMB")))
		plugin_symb = "plugin_init";

	if (!(plugindir = env_get("PLUGINDIR"))) {
		if (!stralloc_copys(&plugin, auto_prefix) ||
				!stralloc_catb(&plugin, "/lib/indimail/plugins/", 22))
			die_nomem();
	} else {
		if (*plugindir != '/')
			die_plugin(plugindir, "plugindir must have an absolute path", 0, 0);
		if (!stralloc_copys(&plugin, plugindir) ||
				!stralloc_append(&plugin, "/"))
			die_nomem();
	}

	if (!(start_plugin = env_get("SMTP_PLUGIN")))
		start_plugin = "smtpd-plugin.so";
	if (!stralloc_cats(&plugin, start_plugin) ||
			!stralloc_0(&plugin))
		die_nomem();
	len = plugin.len;
	/*- figure out plugin count */
	for (i = plugin_count = 0;; plugin_count++) {
		if (!plugin_count) {
			if (access(plugin.s, R_OK)) {
				if (errno != error_noent)
					die_plugin("unable to access plugin: ", plugin.s, 0, 0);
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i++)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1) ||
						!stralloc_cats(&plugin, ".so") ||
						!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK)) {
					if (errno != error_noent)
						die_plugin("unable to access plugin: ", plugin.s, 0, 0);
					goto command;
				}
			}
		} else {
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i++)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1) ||
					!stralloc_cats(&plugin, ".so") ||
					!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK)) {
				if (errno != error_noent)
					die_plugin("unable to access plugin: ", plugin.s, 0, 0);
				break;
			}
		}
	}
	if (!(plughandle = (void **) alloc(sizeof (void *) * plugin_count)))
		die_nomem();
	if (!(plug = (PLUGIN **) alloc(sizeof (PLUGIN *) * plugin_count)))
		die_nomem();
	plugin.len = len - 4;
	if (!stralloc_cats(&plugin, ".so") ||
			!stralloc_0(&plugin))
		die_nomem();
	for (i = j = 0; i < plugin_count;) {
		if (!j) {
			if (access(plugin.s, R_OK)) { /*- smtpd-plugin.so */
				if (errno != error_noent)
					die_plugin("unable to access plugin: ", plugin.s, 0, 0);
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1) ||
						!stralloc_cats(&plugin, ".so") ||
						!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK)) {		/*- smtpd-plugin0.so */
					if (errno != error_noent)
						die_plugin("unable to access plugin: ", plugin.s, 0, 0);
					goto command;
				}
				load_plugin(plugin.s, plugin_symb, j++);
				i++;
			} else
				load_plugin(plugin.s, plugin_symb, j++);
		} else { /*- smtpd-plugin1.so, smtpd-plugin2.so, ... */
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1) ||
					!stralloc_cats(&plugin, ".so") ||
					!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK)) {
				if (errno != error_noent)
					die_plugin("unable to access plugin: ", plugin.s, 0, 0);
				break;
			}
			load_plugin(plugin.s, plugin_symb, j++);
			i++;
		}
	}
command:
#endif
#ifdef TLS
	secure_auth = env_get("SECURE_AUTH") ? 1 : 0;
#endif
	if (commands(&ssin, cmdptr) == 0)
		die_read("client dropped connection", 0);
	die_nomem();
}

/*- Rejection of relay probes. */
int
addrrelay()
{
	int             j;

	j = addr.len;
	while (--j >= 0)
		if (addr.s[j] == '@')
			break;
	if (j < 0)
		j = addr.len;
	while (--j >= 0) {
		if (addr.s[j] == '@')
			return 1;
		if (addr.s[j] == '%')
			return 1;
		if (addr.s[j] == '!')
			return 1;
	}
	return 0;
}

/*
 * $Log: smtpd.c,v $
 * Revision 1.269  2022-10-09 23:00:33+05:30  Cprogrammer
 * removed include wildmat.h
 *
 * Revision 1.268  2022-10-08 20:01:48+05:30  Cprogrammer
 * set SPFRESULT environment variable
 *
 * Revision 1.267  2022-10-07 18:09:39+05:30  Cprogrammer
 * fixed length of batv signkey
 *
 * Revision 1.266  2022-08-25 18:28:52+05:30  Cprogrammer
 * fetch hexsalted and clear text passwords
 *
 * Revision 1.265  2022-08-23 13:11:13+05:30  Cprogrammer
 * replaced authmethod_to_str() with get_authmethod() from libqmail
 *
 * Revision 1.264  2022-08-23 01:18:50+05:30  Cprogrammer
 * fixed crash when ssl_timeoutaccept() failed
 *
 * Revision 1.263  2022-08-22 21:04:56+05:30  Cprogrammer
 * handle errors in gs_callback
 *
 * Revision 1.262  2022-08-22 11:03:21+05:30  Cprogrammer
 * removed auth_cram.h
 * altered few error messages
 *
 * Revision 1.261  2022-08-21 19:31:25+05:30  Cprogrammer
 * fix compilation error when TLS is not defined in conf-tls
 * replace hard coded auth methods with defines in authmethods.h
 * disable AUTH= string in EHLO if OLD_CLIENT isn't set
 *
 * Revision 1.260  2022-08-15 20:55:05+05:30  Cprogrammer
 * fixed channel binding logic
 * added option to disable SCRAM PLUS variants
 *
 * Revision 1.259  2022-08-15 08:36:01+05:30  Cprogrammer
 * fixed minor formatting issue with die_read(), err_scram() functions
 *
 * Revision 1.258  2022-08-14 20:57:12+05:30  Cprogrammer
 * conditionally add log_gsasl_version() using #ifdef HASLIBGSAS
 *
 * Revision 1.257  2022-08-14 19:13:11+05:30  Cprogrammer
 * added auth methods SCRAM-SHA-1-PLUS, SCRAM-SHA-256-PLUS
 *
 * Revision 1.256  2022-08-05 11:43:05+05:30  Cprogrammer
 * added missing flush() statements
 * display error if incorrect scram encryption level is used
 *
 * Revision 1.255  2022-08-04 13:55:53+05:30  Cprogrammer
 * scram authentication with salt and stored/server keys
 *
 * Revision 1.254  2022-07-26 09:33:10+05:30  Cprogrammer
 * added AUTH CRAM-SHA224, CRAM-SHA384 methods
 *
 * Revision 1.253  2022-06-01 13:05:24+05:30  Cprogrammer
 * clear errno when client drops connection
 *
 * Revision 1.252  2022-05-18 17:01:02+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.251  2022-02-26 09:05:54+05:30  Cprogrammer
 * fix ehlo greating for local connections
 *
 * Revision 1.250  2022-01-30 09:42:52+05:30  Cprogrammer
 * allow disabling of databytes check
 * replaced execvp with execv
 *
 * Revision 1.249  2021-10-20 22:56:20+05:30  Cprogrammer
 * allow SMTP code to be configured when setting SHUTDOWN env variable
 *
 * Revision 1.248  2021-09-11 19:02:28+05:30  Cprogrammer
 * pass null remotehost to received when remotehost is unknown
 *
 * Revision 1.247  2021-09-10 15:25:23+05:30  Cprogrammer
 * removed setting of SPFRESULT env variable
 *
 * Revision 1.246  2021-08-19 20:22:49+05:30  Cprogrammer
 * disable VRFY using DISABLE_VRFY env variable
 *
 * Revision 1.245  2021-08-12 22:36:22+05:30  Cprogrammer
 * disable help if DISABLE_HELP is set
 *
 * Revision 1.244  2021-07-03 14:01:42+05:30  Cprogrammer
 * replaced getpwent() with in-build check for user in /etc/passwd
 *
 * Revision 1.243  2021-06-28 16:58:02+05:30  Cprogrammer
 * fix error when libindimail is missing
 *
 * Revision 1.242  2021-06-14 01:09:28+05:30  Cprogrammer
 * added missing error check for stralloc failure
 *
 * Revision 1.241  2021-06-12 19:57:11+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.240  2021-05-26 10:46:59+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.239  2021-05-23 07:11:18+05:30  Cprogrammer
 * include wildmat.h for wildmat_internal
 *
 * Revision 1.238  2021-03-02 10:40:40+05:30  Cprogrammer
 * renamed TLSCIPHERS env variable to TLS_CIPHER_LIST
 *
 * Revision 1.237  2021-02-08 00:05:54+05:30  Cprogrammer
 * fixed checkrecipient for local users
 *
 * Revision 1.236  2021-02-07 23:14:51+05:30  Cprogrammer
 * use functions directly from libindimail
 *
 * Revision 1.235  2021-01-23 15:17:55+05:30  Cprogrammer
 * use remotehost for automatic helo
 *
 * Revision 1.234  2021-01-23 08:14:52+05:30  Cprogrammer
 * renamed env variable UTF8 to SMTPUTF8
 * check FORCE_TLS in smtp_mail()
 *
 * Revision 1.233  2020-12-07 12:06:27+05:30  Cprogrammer
 * refactored mailfrom_parms()
 *
 * Revision 1.232  2020-12-03 17:29:59+05:30  Cprogrammer
 * added EAI - RFC 6530-32 - unicode address support
 *
 * Revision 1.231  2020-11-26 14:01:08+05:30  Cprogrammer
 * refactored batv code
 *
 * Revision 1.230  2020-11-24 13:48:13+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.229  2020-10-26 22:55:33+05:30  Cprogrammer
 * added documentation on return value of authenticate() with account has NO_RELAY
 *
 * Revision 1.228  2020-10-10 21:21:33+05:30  Cprogrammer
 * functions not needed outside made static
 *
 * Revision 1.227  2020-09-15 09:40:00+05:30  Cprogrammer
 * ctl_maxcmdlen moved to libqmail
 *
 * Revision 1.226  2020-07-01 21:35:28+05:30  Cprogrammer
 * added missing error check for "out of mem"
 *
 * Revision 1.225  2020-05-11 11:12:08+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.224  2019-10-27 10:43:04+05:30  Cprogrammer
 * display table/file name in error logs
 *
 * Revision 1.223  2019-07-10 13:39:26+05:30  Cprogrammer
 * fixed ssl getting terminated by output on fd 1 after exec
 *
 * Revision 1.222  2019-05-27 20:30:50+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.221  2019-05-27 12:39:17+05:30  Cprogrammer
 * set libfn with full path of libindimail control file
 *
 * Revision 1.220  2019-05-26 12:32:40+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.219  2019-04-20 19:52:52+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.218  2019-04-16 23:59:48+05:30  Cprogrammer
 * changed parse_email() parameters
 *
 * Revision 1.217  2019-03-07 00:55:21+05:30  Cprogrammer
 * do not treat regcomp error as matches
 *
 * Revision 1.216  2018-11-12 08:28:27+05:30  Cprogrammer
 * removed potential leaks
 *
 * Revision 1.215  2018-08-21 20:37:56+05:30  Cprogrammer
 * fixed comments and indentation
 *
 * Revision 1.214  2018-08-12 00:28:37+05:30  Cprogrammer
 * removed memory leaks due to auto stralloc variables
 *
 * Revision 1.213  2018-07-02 00:34:15+05:30  Cprogrammer
 * checkrecipient support for domains in /etc/indimail/control/locals
 *
 * Revision 1.212  2018-07-01 11:50:03+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.211  2018-06-13 08:55:27+05:30  Cprogrammer
 * refactored smtp_rset()
 *
 * Revision 1.210  2018-06-11 23:29:28+05:30  Cprogrammer
 * replaced ssl_free() with SSL_shutdown(), SSL_free()
 *
 * Revision 1.209  2018-05-25 08:44:38+05:30  Cprogrammer
 * added whitespace for readability
 *
 * Revision 1.208  2018-05-13 15:52:36+05:30  Cprogrammer
 * disable SSLv2, SSLv3 to fix Poodle vulnerability
 *
 * Revision 1.207  2018-01-15 09:22:04+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.206  2018-01-09 12:34:23+05:30  Cprogrammer
 * use loadLibrary() function to load indimail functions
 *
 * Revision 1.205  2017-12-26 21:58:06+05:30  Cprogrammer
 * BUGFIX - Fixed wrong copy of PLUGINDIR
 *
 * Revision 1.204  2017-12-26 15:26:06+05:30  Cprogrammer
 * make PLUGINDIR path absolute
 * use auto_prefix for plugindir path
 * set environment variable AUTHENTICATED
 *
 * Revision 1.203  2017-12-17 19:12:52+05:30  Cprogrammer
 * moved RCS log to bottom and added documentation.
 *
 * Revision 1.202  2017-08-26 11:41:16+05:30  Cprogrammer
 * initialize proto.len in smtp_init()
 *
 * Revision 1.201  2017-08-26 11:05:20+05:30  Cprogrammer
 * fixed reading tlsservermethod control file
 *
 * Revision 1.200  2017-08-25 19:33:25+05:30  Cprogrammer
 * fixed syntax error
 *
 * Revision 1.199  2017-08-24 13:19:33+05:30  Cprogrammer
 * improved logging of TLS method errors
 *
 * Revision 1.198  2017-08-23 13:10:50+05:30  Cprogrammer
 * replaced SSLv23_server_method() with TLS_server_method()
 * fixed ifdefs for openssl 1.0.1 version.
 *
 * Revision 1.197  2017-08-08 23:56:33+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.196  2017-05-15 19:21:50+05:30  Cprogrammer
 * fixed spurious error "no valid cert for gateway"
 *
 * Revision 1.195  2017-05-08 13:52:14+05:30  Cprogrammer
 * check for tlsv1_1_server_method() and tlsv1_2_server_method()
 *
 * Revision 1.194  2017-04-16 13:12:31+05:30  Cprogrammer
 * use different variable for nodnscheck control file
 *
 * Revision 1.193  2017-03-21 15:39:52+05:30  Cprogrammer
 * use CERTDIR to override tls/ssl certificates
 *
 * Revision 1.192  2017-03-10 17:56:07+05:30  Cprogrammer
 * fixed value of protocol line for tls session in Received header
 *
 * Revision 1.191  2017-03-10 11:33:05+05:30  Cprogrammer
 * TLS server method configurable through control file tlsservermethod
 *
 */

char           *
getversion_smtpd_c()
{
	static char    *x = "$Id: smtpd.c,v 1.269 2022-10-09 23:00:33+05:30 Cprogrammer Exp mbhangui $";

	x++;
	return revision + 11;
}
