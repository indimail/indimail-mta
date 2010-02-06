/*
 * $Log: qmail-remote.c,v $
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
 * do not compare with maproutes if smtproutes is absent
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
#include "cdb.h"
#include "open.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sig.h"
#include "env.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "scan.h"
#include "case.h"
#include "error.h"
#include "auto_qmail.h"
#include "control.h"
#include "dns.h"
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "quote.h"
#include "ip.h"
#include "ipalloc.h"
#include "ipme.h"
#include "gen_alloc.h"
#include "gen_allocdefs.h"
#include "str.h"
#include "now.h"
#include "exit.h"
#include "constmap.h"
#include "tcpto.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "variables.h"
#include "fmt.h"
#include "socket.h"
#include "base64.h"
#include <unistd.h>
#ifdef TLS
#include <sys/stat.h>
#include "tls.h"
#include "ssl_timeoutio.h"
#include <openssl/x509v3.h>
#define EHLO 1
#endif
#define HUGESMTPTEXT  5000
#define MIN_PENALTY   3600
#define MAX_TOLERANCE 120

#define PORT_SMTP     25 /*- silly rabbit, /etc/services is for users */

#ifdef TLS
int             tls_init();
#endif

unsigned long   port = PORT_SMTP;

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, i, n, x, 10, saa_readyplus)

static stralloc sauninit = { 0 };
stralloc        helohost = { 0 };
stralloc        senderbind = { 0 };
stralloc        localips = { 0 };
stralloc        outgoingip = { 0 };
stralloc        routes = { 0 };
int             cntrl_stat;
struct constmap maproutes;
struct constmap maplocalips;
stralloc        host = { 0 };
stralloc        sender = { 0 };
stralloc        qqeh = { 0 };
stralloc        user = { 0 };
stralloc        pass = { 0 };
saa             reciplist = { 0 };
struct ip_mx    partner;
union v46addr   outip;
static int      inside_greeting = 0;
static char    *msgsize, *use_auth_smtp;
#ifdef TLS
const char     *ssl_err_str = 0;
#endif
int             fdmoreroutes = -1;
int             flagtcpto = 1;
int             min_penalty = MIN_PENALTY;
unsigned long   max_tolerance = MAX_TOLERANCE;

/*- http://mipassoc.org/pipermail/batv-tech/2007q4/000032.html */
#ifdef BATV 
#define BATVLEN 3 /*- number of bytes */
#include "byte.h"
#include <openssl/md5.h>
char            batvok = 0;
stralloc        signkey = {0};
stralloc        nosign = {0};
stralloc        nosigndoms = {0};
struct constmap mapnosign;
struct constmap mapnosigndoms;
#endif

void
out(char *s)
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

void
zerodie()
{
	zero();
	substdio_flush(subfdoutsmall);
	_exit(0);
}

void
outsafe(stralloc *sa)
{
	int             i;
	char            ch;

	for (i = 0; i < sa->len; ++i)
	{
		ch = sa->s[i];
		if (ch < 33)
			ch = '?';
		if (ch > 126)
			ch = '?';
		if (substdio_put(subfdoutsmall, &ch, 1) == -1)
			_exit(0);
	}
}

void
temp_noip()
{
	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	out("Zinvalid ipaddr in ");
	out(controldir);
	out("/outgoingip (#4.3.0)\n");
	zerodie();
}

void
temp_nomem()
{
	out("ZOut of memory. (#4.3.0)\n");
	zerodie();
}

void
temp_oserr()
{
	out("ZSystem resources temporarily unavailable. (#4.3.0)\n");
	zerodie();
}

void
temp_read()
{
	out("ZUnable to read message. (#4.3.0)\n");
	zerodie();
}

void
temp_dnscanon()
{
	out("ZCNAME lookup failed temporarily. (#4.4.3)\n");
	zerodie();
}

void
temp_dns()
{
	out("ZSorry, I couldn't find any host by that name. (#4.1.2)\n");
	zerodie();
}

void
temp_chdir()
{
	out("ZUnable to switch to home directory. (#4.3.0)\n");
	zerodie();
}

void
temp_control()
{
	out("ZUnable to read control files. (#4.3.0)\n");
	zerodie();
}

void
perm_partialline()
{
	out("DSMTP cannot transfer messages with partial final lines. (#5.6.2)\n");
	zerodie();
}

void
perm_usage()
{
	out("DI (qmail-remote) was invoked improperly. (#5.3.5)\n");
	zerodie();
}

void
perm_dns()
{
	out("DSorry, I couldn't find any host named ");
	outsafe(&host);
	out(". (#5.1.2)\n");
	zerodie();
}

void
perm_nomx()
{
	out("DSorry, I couldn't find a mail exchanger or IP address. (#5.4.4)\n");
	zerodie();
}

void
perm_ambigmx()
{
	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	out("DSorry. Although I'm listed as a best-preference MX or A for that host,\nit isn't in my ");
	out(controldir);
	out("/locals file, so I don't treat it as local. (#5.4.6)\n");
	zerodie();
}

void
outhost()
{
	char            x[IPFMT];

#ifdef IPV6
	if (partner.af == AF_INET)
	{
		if (substdio_put(subfdoutsmall, x, ip_fmt(x, &partner.addr.ip)) == -1)
			_exit(0);
	} else
	if (substdio_put(subfdoutsmall, x, ip6_fmt(x, &partner.addr.ip6)) == -1)
		_exit(0);
#else
	if (substdio_put(subfdoutsmall, x, ip_fmt(x, &partner.addr.ip)) == -1)
		_exit(0);
#endif
}

int             flagcritical = 0;

void
dropped()
{
	char            strnum[FMT_ULONG];

	out("ZConnected to ");
	outhost();
	out(" but connection died. ");
	if (flagcritical)
		out("Possible duplicate! ");
	else
	if (inside_greeting) /*- RFC 2821 - client should treat connection failures as temporary error */
	{
		out("Will try alternate MX. ");
		if (flagtcpto)
		{
			out("tcpto interval ");
			strnum[fmt_ulong(strnum, max_tolerance)] = 0;
			out(strnum);
			out(" ");
			tcpto_err(&partner, 1, max_tolerance);
		}
	}
#ifdef TLS
	if (ssl_err_str)
	{
		out((char *) ssl_err_str);
		out(" ");
	}
#endif
	out("(#4.4.2)\n");
	zerodie();
}

void
temp_noconn(stralloc *h, char *ip, int port)
{
	char            strnum[FMT_ULONG];

	out("ZSorry, I wasn't able to establish an SMTP connection for ");
	if (substdio_put(subfdoutsmall, h->s, h->len) == -1)
		_exit(0);
	if (ip)
	{
		out(" to ");
		out(ip);
		out(" port ");
		strnum[fmt_ulong(strnum, port)] = 0;
		out(strnum);
		alloc_free(ip);
	}
	out(". (#4.4.1)\n");
	zerodie();
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
	if (ssl)
	{
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
	if (ssl)
	{
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

char            inbuf[1024];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
char            smtptobuf[1024];
substdio        smtpto = SUBSTDIO_FDBUF(safewrite, -1, smtptobuf, sizeof smtptobuf);
char            smtpfrombuf[128];
substdio        smtpfrom = SUBSTDIO_FDBUF(saferead, -1, smtpfrombuf, sizeof smtpfrombuf);
stralloc        smtptext = { 0 };

void
get(char *ch)
{
	substdio_get(&smtpfrom, ch, 1);
	if (*ch != '\r')
	{
		if (smtptext.len < HUGESMTPTEXT)
		{
			if (!stralloc_append(&smtptext, ch))
				temp_nomem();
		}
	}
}

unsigned long
smtpcode()
{
	unsigned char   ch;
	unsigned long   code;

	if (!stralloc_copys(&smtptext, ""))
		temp_nomem();
	get((char *) &ch);
	code = ch - '0';
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	for (;;)
	{
		get((char *) &ch);
		if (ch != '-')
			break;
		while (ch != '\n')
			get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
	}
	while (ch != '\n')
		get((char *) &ch);
	return code;
}

saa             ehlokw = { 0 };	/*- list of EHLO keywords and parameters */
int             maxehlokwlen = 0;

unsigned long
ehlo()
{
	stralloc       *sa;
	char           *s, *e, *p;
	unsigned long   code;

	if (ehlokw.len > maxehlokwlen)
		maxehlokwlen = ehlokw.len;
	ehlokw.len = 0;

#ifdef MXPS
	if (type == 's')
		return 0;
#endif
	substdio_puts(&smtpto, "EHLO ");
	substdio_put(&smtpto, helohost.s, helohost.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 250)
		return code;
	s = smtptext.s;
	while (*s++ != '\n');		/*- skip the first line: contains the domain */
	e = smtptext.s + smtptext.len - 6;	/*- 250-?\n */
	while (s <= e)
	{
		int             wasspace = 0;

		if (!saa_readyplus(&ehlokw, 1))
			temp_nomem();
		sa = ehlokw.sa + ehlokw.len++;
		if (ehlokw.len > maxehlokwlen)
			*sa = sauninit;
		else
			sa->len = 0;

		/*- smtptext is known to end in a '\n' */
		for (p = (s += 4);; ++p)
		{
			if (*p == '\n' || *p == ' ' || *p == '\t')
			{
				if (!wasspace)
					if (!stralloc_catb(sa, s, p - s) || !stralloc_0(sa))
						temp_nomem();
				if (*p == '\n')
					break;
				wasspace = 1;
			} else
			if (wasspace == 1)
			{
				wasspace = 0;
				s = p;
			}
		}
		s = ++p;

		/*
		 * keyword should consist of alpha-num and '-'
		 * broken AUTH might use '=' instead of space 
		 */
		for (p = sa->s; *p; ++p)
		{
			if (*p == '=')
			{
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

	if (smtptext.s)
	{
		if (smtptext.len)
		{
			out("Remote host said: ");
			for (i = 0; i < smtptext.len; ++i)
			{
				if (!smtptext.s[i])
					smtptext.s[i] = '?';
			}
			if (substdio_put(subfdoutsmall, smtptext.s, smtptext.len) == -1)
				_exit(0);
			smtptext.len = 0;
		}
	}
}

void
quit(char *prepend, char *append, int die)
{
#ifdef TLS
	/*
	 * shouldn't talk to the client unless in an appropriate state 
	 */
	int             state = ssl ? ssl->state : SSL_ST_BEFORE;

	if ((state & SSL_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		substdio_putsflush(&smtpto, "QUIT\r\n");
#else
	substdio_putsflush(&smtpto, "QUIT\r\n");
#endif
	/*- waiting for remote side is just too ridiculous */
	out(prepend);
	outhost();
	out(append);
	out(".\n");
	outsmtptext();
#if defined(TLS) && defined(DEBUG)
	if (ssl)
	{
		X509           *peercert;

		out("STARTTLS proto=");
		out(SSL_get_version(ssl));
		out("; cipher=");
		out(SSL_get_cipher(ssl));

		/*
		 * we want certificate details 
		 */
		if (peercert = SSL_get_peer_certificate(ssl))
		{
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
#endif
	if (die)
		zerodie();
	else
	{
		zero();
		substdio_flush(subfdoutsmall);
	}
}

void
blast()
{
	int             r;
	char            ch;

	for (r = 0; r < qqeh.len; r++)
	{
		if (qqeh.s[r] == '\n')
			substdio_put(&smtpto, "\r", 1);
		substdio_put(&smtpto, qqeh.s + r, 1);
	}
	for (;;)
	{
		if (!(r = substdio_get(&ssin, &ch, 1)))
			break;
		if (r == -1)
			temp_read();
		if (ch == '.')
			substdio_put(&smtpto, ".", 1);
		while (ch != '\n')
		{
			substdio_put(&smtpto, &ch, 1);
			r = substdio_get(&ssin, &ch, 1);
			if (r == 0)
				perm_partialline();
			if (r == -1)
				temp_read();
		}
		substdio_put(&smtpto, "\r\n", 2);
	}
	flagcritical = 1;
	substdio_put(&smtpto, ".\r\n", 3);
	substdio_flush(&smtpto);
}

#ifdef TLS

#define TLS_QUIT quit(ssl ? "; connected to " : "; connecting to ", "", 1)
#define tls_quit_error(s) tls_quit(s, ssl_error())

char           *partner_fqdn = 0;

void
tls_quit(const char *s1, const char *s2)
{
	out((char *) s1);
	if (s2)
	{
		out(": ");
		out((char *) s2);
	}
	TLS_QUIT;
}

int
match_partner(char *s, int len)
{
	if (!case_diffb(partner_fqdn, len, (char *) s) && !partner_fqdn[len])
		return 1;
	/*
	 * we also match if the name is *.domainname 
	 */
	if (*s == '*')
	{
		const char     *domain = partner_fqdn + str_chr(partner_fqdn, '.');
		if (!case_diffb((char *) domain, --len, (char *) ++s) && !domain[len])
			return 1;
	}
	return 0;
}

/*
 * don't want to fail handshake if certificate can't be verified 
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX * ctx)
{
	return 1;
}

int
tls_init()
{
	int             i = 0, needtlsauth = 0;
	const char     *ciphers;
	SSL            *myssl;
	SSL_CTX        *ctx;
	stralloc        saciphers = {0}, servercert = {0}, clientcert = {0};

	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	if (partner_fqdn)
	{
		struct stat     st;
		if (!stralloc_copys(&servercert, controldir))
			temp_nomem();
		if (!stralloc_catb(&servercert, "/tlshosts/", 10))
			temp_nomem();
		if (!stralloc_catb(&servercert, partner_fqdn, str_len(partner_fqdn)))
			temp_nomem();
		if (!stralloc_catb(&servercert, ".pem", 4))
			temp_nomem();
		if (!stralloc_0(&servercert))
			temp_nomem();
		if (stat(servercert.s, &st))
		{
			needtlsauth = 0;
			if (!stralloc_copys(&servercert, controldir))
				temp_nomem();
			if (!stralloc_catb(&servercert, "/notlshosts/", 12))
				temp_nomem();
			if (!stralloc_catb(&servercert, partner_fqdn, str_len(partner_fqdn) + 1))
				temp_nomem();
			if (!stralloc_0(&servercert))
				temp_nomem();
			if (!stat(servercert.s, &st))
			{
				alloc_free(servercert.s);
				return (0);
			}
			if (!stralloc_copys(&servercert, controldir))
				temp_nomem();
			if (!stralloc_catb(&servercert, "/tlshosts/exhaustivelist", 24))
				temp_nomem();
			if (!stralloc_0(&servercert))
				temp_nomem();
			if (!stat(servercert.s, &st))
			{
				alloc_free(servercert.s);
				return 0;
			}
		} else
			needtlsauth = 1;
	}
	if (!smtps)
	{
		stralloc       *sa = ehlokw.sa;
		unsigned int    len = ehlokw.len;
		/*
		 * look for STARTTLS among EHLO keywords 
		 */
		for (; len && case_diffs(sa->s, "STARTTLS"); ++sa, --len);
		if (!len)
		{
			if (!needtlsauth)
			{
				if (servercert.s)
					alloc_free(servercert.s);
				return 0;
			}
			out("ZNo TLS achieved while ");
			out(servercert.s);
			out(" exists");
			smtptext.len = 0;
			TLS_QUIT;
		}
	}
	SSL_library_init();
	if (!(ctx = SSL_CTX_new(SSLv23_client_method())))
	{
		if (!smtps && !needtlsauth)
		{
			if (servercert.s)
				alloc_free(servercert.s);
			return 0;
		}
		smtptext.len = 0;
		tls_quit_error("ZTLS error initializing ctx");
	}

	if (needtlsauth)
	{
		if (!SSL_CTX_load_verify_locations(ctx, servercert.s, NULL))
		{
			SSL_CTX_free(ctx);
			smtptext.len = 0;
			out("ZTLS unable to load ");
			if (servercert.s)
				alloc_free(servercert.s);
			tls_quit_error(servercert.s);
		}
		/*
		 * set the callback here; SSL_set_verify didn't work before 0.9.6c 
		 */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
	}

	/*
	 * let the other side complain if it needs a cert and we don't have one 
	 */
	if (!stralloc_copys(&clientcert, controldir))
		temp_nomem();
	if (!stralloc_catb(&clientcert, "/clientcert.pem", 15))
		temp_nomem();
	if (!stralloc_0(&clientcert))
		temp_nomem();
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert.s))
		SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
	alloc_free(clientcert.s);
	myssl = SSL_new(ctx);
	SSL_CTX_free(ctx);
	if (!myssl)
	{
		if (!smtps && !needtlsauth)
		{
			if (servercert.s)
				alloc_free(servercert.s);
			return 0;
		}
		smtptext.len = 0;
		if (servercert.s)
			alloc_free(servercert.s);
		tls_quit_error("ZTLS error initializing ssl");
	}

	if (!smtps)
		substdio_putsflush(&smtpto, "STARTTLS\r\n");

	/*
	 * while the server is preparing a responce, do something else 
	 */
	if (control_readfile(&saciphers, "tlsclientciphers", 0) == -1)
	{
		SSL_free(myssl);
		temp_control();
	}
	if (saciphers.len)
	{
		for (i = 0; i < saciphers.len - 1; ++i)
			if (!saciphers.s[i])
				saciphers.s[i] = ':';
		ciphers = saciphers.s;
	} else
		ciphers = "DEFAULT";
	SSL_set_cipher_list(myssl, ciphers);
	alloc_free(saciphers.s);

	/*- SSL_set_options(myssl, SSL_OP_NO_TLSv1); */
	SSL_set_fd(myssl, smtpfd);

	/*
	 * read the responce to STARTTLS 
	 */
	if (!smtps)
	{
		if (smtpcode() != 220)
		{
			SSL_free(myssl);
			if (!needtlsauth)
			{
				if (servercert.s)
					alloc_free(servercert.s);
				return 0;
			}
			out("ZSTARTTLS rejected while ");
			out(servercert.s);
			out(" exists");
			TLS_QUIT;
		}
		smtptext.len = 0;
	}
	ssl = myssl;
	if (ssl_timeoutconn(timeout, smtpfd, smtpfd, ssl) <= 0)
	{
		if (servercert.s)
			alloc_free(servercert.s);
		tls_quit("ZTLS connect failed", ssl_error_str());
	}
	if (needtlsauth)
	{
		X509           *peercert;
		STACK_OF(GENERAL_NAME) *gens;

		int             r = SSL_get_verify_result(ssl);
		if (r != X509_V_OK)
		{
			out("ZTLS unable to verify server with ");
			if (servercert.s)
				alloc_free(servercert.s);
			tls_quit(servercert.s, X509_verify_cert_error_string(r));
		}
		alloc_free(servercert.s);
		if (!(peercert = SSL_get_peer_certificate(ssl)))
		{
			out("ZTLS unable to verify server ");
			tls_quit(partner_fqdn, "no certificate provided");
		}

		/*
		 * RFC 2595 section 2.4: find a matching name
		 * * first find a match among alternative names 
		 */
		if ((gens = X509_get_ext_d2i(peercert, NID_subject_alt_name, 0, 0)))
		{
			for (i = 0, r = sk_GENERAL_NAME_num(gens); i < r; ++i)
			{
				const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, i);
				if (gn->type == GEN_DNS)
					if (match_partner((char *) gn->d.ia5->data, gn->d.ia5->length))
						break;
			}
			sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
		}

		/*
		 * no alternative name matched, look up commonName 
		 */
		if (!gens || i >= r)
		{
			stralloc        peer = { 0 };
			X509_NAME      *subj = X509_get_subject_name(peercert);
			i = X509_NAME_get_index_by_NID(subj, NID_commonName, -1);
			if (i >= 0)
			{
				const ASN1_STRING *s = X509_NAME_get_entry(subj, i)->value;
				if (s)
				{
					peer.len = s->length;
					peer.s = (char *) s->data;
				}
			}
			if (peer.len <= 0)
			{
				out("ZTLS unable to verify server ");
				tls_quit(partner_fqdn, "certificate contains no valid commonName");
			}
			if (!match_partner((char *) peer.s, peer.len))
			{
				out("ZTLS unable to verify server ");
				out(partner_fqdn);
				out(": received certificate for ");
				outsafe(&peer);
				TLS_QUIT;
			}
		}

		X509_free(peercert);
	}
	if (smtps && smtpcode() != 220)
		quit("ZTLS Connected to ", " but greeting failed", 1);
	return 1;
}
#endif

stralloc        xuser = { 0 };
stralloc        auth = { 0 };
stralloc        plain = { 0 };

int
xtext(stralloc *ptr, char *s, int len)
{
	int             i;

	if (!stralloc_copys(ptr, ""))
		temp_nomem();
	for (i = 0; i < len; i++)
	{
		if (s[i] == '+')
		{
			if (!stralloc_cats(ptr, "+2B"))
				temp_nomem();
		} else
		if (s[i] == '=')
		{
			if (!stralloc_cats(ptr, "+3D"))
				temp_nomem();
		} else
		if ((int) s[i] < 33 || (int) s[i] > 126)
		{
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
	substdio_puts(&smtpto, "MAIL FROM:<");
	substdio_put(&smtpto, sender.s, sender.len);
	if (use_size)
	{
		substdio_puts(&smtpto, "> SIZE=");
		substdio_puts(&smtpto, msgsize);
		substdio_puts(&smtpto, "\r\n");
	} else
		substdio_puts(&smtpto, ">\r\n");
}

void
auth_plain(int use_size)
{
	substdio_puts(&smtpto, "AUTH PLAIN\r\n");
	substdio_flush(&smtpto);
	if (smtpcode() != 334)
		quit("ZConnected to ", " but authentication was rejected (AUTH PLAIN).", 1);
	if (!stralloc_cat(&plain, &sender))
		temp_nomem();			/*- Mail From: <authorize-id> */
	if (!stralloc_0(&plain))
		temp_nomem();
	if (!stralloc_cat(&plain, &user))
		temp_nomem();			/*- user-id */
	if (!stralloc_0(&plain))
		temp_nomem();
	if (!stralloc_cat(&plain, &pass))
		temp_nomem();			/*- password */
	if (!stralloc_0(&plain))
		temp_nomem();
	if (b64encode(&plain, &auth))
		quit("ZConnected to ", " but unable to base64encode (plain).", 1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if (smtpcode() != 235)
		quit("ZConnected to ", " but authentication was rejected (plain).", 1);
	if (!xtext(&xuser, user.s, user.len))
		temp_nomem();
	substdio_puts(&smtpto, "MAIL FROM:<");
	substdio_put(&smtpto, sender.s, sender.len);
	if (use_size)
	{
		substdio_puts(&smtpto, "> SIZE=");
		substdio_puts(&smtpto, msgsize);
		substdio_puts(&smtpto, " AUTH=<");
	} else
		substdio_puts(&smtpto, "> AUTH=");
	substdio_put(&smtpto, xuser.s, xuser.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
}

void
auth_login(int use_size)
{
	substdio_puts(&smtpto, "AUTH LOGIN\r\n");
	substdio_flush(&smtpto);
	if (smtpcode() != 334)
		quit("ZConnected to ", " but authentication was rejected (AUTH LOGIN).", 1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&user, &auth))
		quit("ZConnected to ", " but unable to base64encode user.", 1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if (smtpcode() != 334)
		quit("ZConnected to ", " but authentication was rejected (username).", 1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&pass, &auth))
		quit("ZConnected to ", " but unable to base64encode pass.", 1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if (smtpcode() != 235)
		quit("ZConnected to ", " but authentication was rejected (password)", 1);

	if (!xtext(&xuser, user.s, user.len))
		temp_nomem();
	substdio_puts(&smtpto, "MAIL FROM:<");
	substdio_put(&smtpto, sender.s, sender.len);
	if (use_size)
	{
		substdio_puts(&smtpto, "> SIZE=");
		substdio_puts(&smtpto, msgsize);
		substdio_puts(&smtpto, " AUTH=<");
	} else
		substdio_puts(&smtpto, "> AUTH=");
	substdio_put(&smtpto, xuser.s, xuser.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
}

void
smtp_auth(char *type, int use_size)
{
	int             i = 0, j, login_supp = 0, plain_supp = 0;

	if (!type)
	{
		mailfrom(use_size);
		return;
	}
	while ((i += str_chr(smtptext.s + i, '\n') + 1) && (i + 8 < smtptext.len)
		&& str_diffn(smtptext.s + i + 4, "AUTH", 4));

	if ((j = str_chr(smtptext.s + i + 8, 'L')) > 0)	/*- AUTH LOGIN */
	{
		if (case_starts(smtptext.s + i + 8 + j, "LOGIN"))
			login_supp = 1;
	}
	if ((j = str_chr(smtptext.s + i + 8, 'P')) > 0)	/*- AUTH PLAIN */
	{
		if (case_starts(smtptext.s + i + 8 + j, "PLAIN"))
			plain_supp = 1;
	}
	if (!case_diffs(type, "LOGIN"))
	{
		if (login_supp)
		{
			auth_login(use_size);
			return;
		} 
	} else
	if (!case_diffs(type, "PLAIN"))
	{
		if (plain_supp)
		{
			auth_plain(use_size);
			return;
		} 
	} else
	if (!*type)
	{
		if (login_supp)
		{
			auth_login(use_size);
			return;
		}
		if (plain_supp)
		{
			auth_plain(use_size);
			return;
		}
	}
	err_authprot();
	mailfrom(use_size);
	return;
}

stralloc         helo_str = { 0 };

void
smtp()
{
	unsigned long   code;
	int             flagbother;
	int             i, use_size = 0, is_esmtp = 1;

	inside_greeting = 1;
#ifdef TLS
#ifdef MXPS
	if (type == 'S')
		smtps = 1;
	else
	if (type != 's' && port == 465)
		smtps = 1;
#else
	if (port == 465)
		smtps = 1;
#endif
	if (!smtps)
		code = smtpcode();
	else
		code = 220;
#else
	code = smtpcode();
#endif
	inside_greeting = 0;
	if (code >= 500 && code < 600)
		quit("DConnected to ", " but greeting failed", 1);
	else
	if (code >= 400 && code < 500)
		return; /* try next MX, see RFC-2821 */
	else
	if (code != 220)
		quit("ZConnected to ", " but greeting failed", 1);
#ifdef TLS
	if (!smtps)
		code = ehlo();
	/*
	 * RFC2487 says we should issue EHLO (even if we might not need
	 * extensions); at the same time, it does not prohibit a server
	 * to reject the EHLO and make us fallback to HELO 
	 */
	if (tls_init())
		code = ehlo();
#else
	code = ehlo();
#endif
	if (!use_auth_smtp)
	{
		if (code >= 500)
		{
			is_esmtp = 0;
			substdio_puts(&smtpto,"HELO ");
			substdio_put(&smtpto,helohost.s,helohost.len);
			substdio_puts(&smtpto,"\r\n");
			substdio_flush(&smtpto);
			code = smtpcode();
		}
	}
	if (code != 250)
	{
		if (!stralloc_copys(&helo_str, " but my name -->"))
			temp_nomem();
		if (!stralloc_cat(&helo_str, &helohost))
			temp_nomem();
		if (!stralloc_cats(&helo_str, "<-- was rejected :("))
			temp_nomem();
		if (!stralloc_0(&helo_str))
			temp_nomem();
		quit(code >= 500 ? "DConnected to " : "ZConnected to " , helo_str.s, code >= 500 ? 1 : 0);
	}
	/*
	 * go through all lines of the multi line answer until one begins
	 * with "XXX[ -]SIZE" or we reach the last line
	 */
	if (is_esmtp)
	{
		i = 0;
		do {
			i += 5 + str_chr(smtptext.s + i, '\n');
			use_size = !case_diffb(smtptext.s + i, 4, "SIZE");
			if (use_size)
				break;
		} while (smtptext.s[i - 1] == '-');
	}
	smtp_auth(use_auth_smtp, use_size);
	substdio_flush(&smtpto);
	code = smtpcode();
	if (code >= 500)
		quit("DConnected to ", " but sender was rejected", 1);
	if (code >= 400)
		quit("ZConnected to ", " but sender was rejected", 0);
	flagbother = 0;
	for (i = 0; i < reciplist.len; ++i)
	{
		substdio_puts(&smtpto, "RCPT TO:<");
		substdio_put(&smtpto, reciplist.sa[i].s, reciplist.sa[i].len);
		substdio_puts(&smtpto, ">\r\n");
		substdio_flush(&smtpto);
		code = smtpcode();
		if (code >= 500)
		{
			out("hFrom: <");
			outsafe(&sender);
			out("> RCPT: <");
			outsafe(&reciplist.sa[i]);
			out("> ");
			outhost();
			out(" does not like recipient.\n");
			outsmtptext();
			zero();
		} else
		if (code >= 400)
		{
			out("sFrom: <");
			outsafe(&sender);
			out("> RCPT: <");
			outsafe(&reciplist.sa[i]);
			out("> ");
			outhost();
			out(" does not like recipient.\n");
			outsmtptext();
			zero();
		} else
		{
			out("rFrom: <");
			outsafe(&sender);
			out("> RCPT: <");
			outsafe(&reciplist.sa[i]);
			out("> ");
			zero();
			flagbother = 1;
		}
	}
	if (!flagbother)
		quit("DGiving up on ", "", 1);
	substdio_putsflush(&smtpto, "DATA\r\n");
	code = smtpcode();
	if (code >= 500)
		quit("D", " failed on DATA command", 1);
	if (code >= 400)
		quit("Z", " failed on DATA command", 0);
	blast();
	code = smtpcode();
	flagcritical = 0;
	if (code >= 500)
		quit("D", " failed after I sent the message", 1);
	if (code >= 400)
		quit("Z", " failed after I sent the message", 0);
	quit("K", " accepted message", 1);
}

stralloc        canonhost = { 0 };
stralloc        canonbox = { 0 };

void /*- host has to be canonical, box has to be quoted */
addrmangle(stralloc *saout, char *s, int *flagalias, int flagcname)
{
	int             j;

	*flagalias = flagcname;
	j = str_rchr(s, '@');
	if (!s[j])
	{
		if (!stralloc_copys(saout, s))
			temp_nomem();
		return;
	}
	if (!stralloc_copys(&canonbox, s))
		temp_nomem();
	canonbox.len = j;
	if (!quote(saout, &canonbox))
		temp_nomem();
	if (!stralloc_cats(saout, "@"))
		temp_nomem();
	if (!stralloc_copys(&canonhost, s + j + 1))
		temp_nomem();
	if (flagcname)
	{
		switch (dns_cname(&canonhost))
		{
		case 0:
			*flagalias = 0;
			break;
		case DNS_MEM:
			temp_nomem();
		case DNS_SOFT:
			temp_dnscanon();
		case DNS_HARD:;		/*- alias loop, not our problem */
		}
	}
	if (!stralloc_cat(saout, &canonhost))
		temp_nomem();
}

void
getcontrols()
{
	int             r;
	char           *ip, *senderdomain, *x;
	static stralloc controlfile, outgoingipfn;

	if (control_init() == -1)
		temp_control();
	if (control_readint(&timeout, "timeoutremote") == -1)
		temp_control();
	if (control_readint(&timeoutconnect, "timeoutconnect") == -1)
		temp_control();
	if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
		temp_control();
	if ((ip = env_get("SMTPROUTE"))) /* mysql */
	{
		if (!stralloc_copyb(&routes, ip, str_len(ip) + 1))
			temp_nomem();
		cntrl_stat = 1;
	} else
	{
		cntrl_stat = control_readfile(&routes, "smtproutes", 0);
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&controlfile, controldir))
			temp_nomem();
		else
		if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
			temp_nomem();
		else
		if (!stralloc_cats(&controlfile, "moresmtproutes.cdb"))
			temp_nomem();
		else
		if (!stralloc_0(&controlfile))
			temp_nomem();
		else
		if ((fdmoreroutes = open_read(controlfile.s)) == -1)
		{
			if (errno != error_noent)
				cntrl_stat = -1;
		}
	}
	switch (cntrl_stat)
	{
	case -1: /*- error reading smtproutes */
		temp_control();
	case 0: /*- smtproutes absent */
		if (!constmap_init(&maproutes, "", 0, 1))
			temp_nomem();
		break;
	case 1: /*- smtproutes present */
		if (!constmap_init(&maproutes, routes.s, routes.len, 1))
			temp_nomem();
		break;
	}
#ifdef BATV
	if ((batvok = control_readline(&signkey, (x = env_get("SIGNKEY")) ? x : "signkey")) == -1)
		temp_control();
	if (batvok)
	{
		switch (control_readfile(&nosign, "nosignhosts",0)) {
		case -1:
			temp_control();
		case 0:
			if (!constmap_init(&mapnosign, "", 0, 1))
				temp_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosign, nosign.s, nosign.len, 0))
				temp_nomem();
			break;
		}
		switch (control_readfile(&nosigndoms, "nosignmydoms", 0)) {
		case -1:
			temp_control();
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
	/*- per recipient domain outgoingip */
	if (!stralloc_copys(&outgoingipfn, "outgoingip."))
		temp_nomem();
	else
	if (!stralloc_cat(&outgoingipfn, &host))
		temp_nomem();
	else
	if (!stralloc_0(&outgoingipfn))
		temp_nomem();
	if (!(r = control_readline(&outgoingip, outgoingipfn.s)))
		r = control_readline(&outgoingip, "outgoingip");
	if (r == -1)
	{
		if (errno == error_nomem)
			temp_nomem();
		temp_control();
	}
#ifdef IPV6
	if (0 == r && !stralloc_copys(&outgoingip, "::"))
		temp_nomem();
	if (0 == str_diffn(outgoingip.s, "::", 2))
	{
		int             i;
		for (i = 0;i < 16;i++)
			outip.ip6.d[i] = 0;
	} else
	if (!ip6_scan(outgoingip.s, &outip.ip6))
		temp_noip();
#else
	if (0 == r && !stralloc_copys(&outgoingip, "0.0.0.0"))
		temp_nomem();
	if (0 == str_diffn(outgoingip.s, "0.0.0.0", 7))
	{
		int             i;
		for (i = 0;i < 4;i++)
			outip.ip.d[i] = 0;
	} else
	if (!ip_scan(outgoingip.s, &outip.ip))
		temp_noip();
#endif
	/*- domainbinding patch */
	switch (control_readfile(&localips, (x = env_get("DOMAINBINDINGS")) ? x : "domainbindings" ,0))
	{
	case -1:
		temp_control();
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
	if (sender.len)
	{
		int             i;
		i = str_rchr(sender.s, '@');
		if (i)
			senderdomain = sender.s + i + 1;
		stralloc_copyb(&senderbind, senderdomain, sender.len - i - 1);
		for (i = 0;i <= senderbind.len;++i)
		{
			if ((i == 0) || (i == senderbind.len) || (senderbind.s[i] == '.'))
			{
				if ((senderdomain = constmap(&maplocalips, senderbind.s + i, senderbind.len - i)))
					break;
			}
		}
		if (senderdomain && !*senderdomain)
			senderdomain = 0;
		if (senderdomain)
		{ 
#ifdef IPV6
			if (!ip6_scan(senderdomain, &outip.ip6))
				temp_noip();
#else
			if (!ip_scan(senderdomain, &outip.ip))
				temp_noip();
#endif
			helohost = senderbind;
		}
	}
	/*- ---------------- END DOMAIN BINDING PATCH */
}

#if BATV
stralloc        newsender = { 0 };

void
sign_batv()
{
	int             daynumber = (now() / 86400) % 1000;
	int             i;
	char            kdate[] = "0000";
	static char     hex[] = "0123456789abcdef";
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];

	if (stralloc_starts(&sender, "prvs="))
		return;	/*- already signed */
	if (stralloc_starts(&sender, "sb*-")) {	/* don't sign this */
		sender.len -= 4;
		byte_copy(sender.s, sender.len, sender.s + 4);
		return;
	}
	if (!stralloc_ready(&newsender, sender.len + (2 * BATVLEN + 10)))
		temp_nomem();
	if (!stralloc_copyb(&newsender, "prvs=", 5))
		temp_nomem();
	/*- only one key so far */
	kdate[1] = '0' + daynumber / 100;
	kdate[2] = '0' + (daynumber / 10) % 10;
	kdate[3] = '0' + daynumber % 10;
	if (!stralloc_catb(&newsender, kdate, 4))
		temp_nomem();
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, sender.s, sender.len);
	MD5_Update(&md5, signkey.s, signkey.len);
	MD5_Final(md5digest, &md5);
	for (i = 0; i < BATVLEN; i++) {
		char            md5hex[2];

		md5hex[0] = hex[md5digest[i] >> 4];
		md5hex[1] = hex[md5digest[i] & 15];
		if (!stralloc_catb(&newsender, md5hex, 2))
			temp_nomem();
	}
	/*-	separator */    
	if (!stralloc_catb(&newsender, "=", 1))
		temp_nomem();    
	if (!stralloc_cat(&newsender, &sender))
		temp_nomem();    
	if (!stralloc_copy(&sender, &newsender))
		temp_nomem();
}
#endif

/* 
 * Original by Richard Lyons
 * Case insensitivity by Ted Fines
 * http://www.apecity.com/qmail/moresmtproutes.txt
 */
char           *
lookup_host(char *host, int len)
{
	static stralloc morerelayhost = { 0 };
	stralloc        h = {0};
	uint32          dlen;

	if (!stralloc_copyb(&h, host, len))
		temp_nomem();
	case_lowerb(h.s, h.len);
	if (fdmoreroutes != -1 && cdb_seek(fdmoreroutes, h.s, h.len, &dlen) == 1)
	{
		if (!stralloc_ready(&morerelayhost, (unsigned int) (dlen + 1)))
			temp_nomem();
		morerelayhost.len = dlen;
		if (cdb_bread(fdmoreroutes, morerelayhost.s, morerelayhost.len) == -1)
			temp_control();
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
timeoutconn46(int fd, struct ip_mx *ix, union v46addr *ip, int port, int timeout)
{
#ifdef IPV6
	if (ix->af == AF_INET)
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
	else
		return timeoutconn6(fd, &ix->addr.ip6, ip, port, timeout);
#else
	return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
#endif
}

/*
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

	sig_pipeignore();
	if (argc < 5)
		perm_usage();
	if (chdir(auto_qmail) == -1)
		temp_chdir();
	msgsize = argv[4];
	recips = argv + 5;
	if (!stralloc_copys(&host, argv[1])) /*- host required by getcontrols below */
		temp_nomem();
	getcontrols();
	use_auth_smtp = env_get("AUTH_SMTP");
	relayhost = 0;
	min_penalty = (x = env_get("MIN_PENALTY")) ? scan_int(x, &min_penalty) : MIN_PENALTY;
	max_tolerance = (x = env_get("MAX_TOLERANCE")) ? scan_ulong(x, &max_tolerance) : MAX_TOLERANCE;
	if (cntrl_stat) /*- set in getcontrols() above */
	{
		/*- Per user SMTPROUTE functionality */
		relayhost = lookup_host(argv[5], str_len(argv[5]));
		/*- Look at smtproutes */
		for (i = 0; !relayhost && i <= host.len; ++i)
		{
			if ((i == 0) || (i == host.len) || (host.s[i] == '.'))
			{
				if ((relayhost = constmap(&maproutes, host.s + i, host.len - i)))
					break;
				if ((relayhost = lookup_host(host.s + i, host.len - i)))
					break;
			}
		}
		if (relayhost && !*relayhost)
			relayhost = 0;
	}
	if (relayhost)
	{
		if (use_auth_smtp)
		{
			/*- test.com:x.x.x.x:x user pass */
			i = str_chr(relayhost, ' ');
			if (relayhost[i])
			{
				relayhost[i] = 0;
				j = str_chr(relayhost + i + 1, ' ');
				if (relayhost[i + j + 1])
				{
					relayhost[i + j + 1] = 0;
					if (relayhost[i + 1] && relayhost[i + j + 2])
					{
						if (!stralloc_copys(&user, relayhost + i + 1))
							temp_nomem();
						if (!stralloc_copys(&pass, relayhost + i + j + 2))
							temp_nomem();
					}
				} else
					use_auth_smtp = 0;
			} else
				use_auth_smtp = 0;
		}
		i = str_chr(relayhost, ':');
		if (relayhost[i])
		{
			scan_ulong(relayhost + i + 1, &port);
			relayhost[i] = 0;
			x = relayhost + i + 1;
			i = str_chr(x, ':');
			if (x[i])
			{
				if (x[i + 1] != ':')
					scan_int(x + i + 1, &min_penalty);
				x = relayhost + i + 1;
				i = str_chr(x, ':');
				if (x[i])
				{
					if (x[i + 1] != ':')
						scan_ulong(x + i + 1, &max_tolerance);
				}
				if (!min_penalty)
					flagtcpto = 0;
			}
		}
		if (!stralloc_copys(&host, relayhost))
			temp_nomem();
	} else
		use_auth_smtp = 0;
	addrmangle(&sender, argv[2], &flagalias, 0);
	if (!stralloc_copys(&qqeh, argv[3]))
		temp_nomem();
#if BATV
	if (batvok && sender.len && signkey.len) {
		int             j;

		if (!stralloc_0(&sender))
			temp_nomem(); /* null terminate */
		sender.len--;
		i = str_rchr(argv[5], '@');	/* should check all recips, not just the first */
		j = str_rchr(sender.s, '@');
		if (!constmap(&mapnosign, argv[5] + i + 1, str_len(argv[5] + i + 1)) 
				&& !constmap(&mapnosigndoms, sender.s + j + 1, sender.len - (j + 1)))
			sign_batv(); /*- modifies sender */
	}
#endif
	if (!saa_readyplus(&reciplist, 0))
		temp_nomem();
	if (ipme_init() != 1)
		temp_oserr();
	flagallaliases = 1;
	while (*recips)
	{
		if (!saa_readyplus(&reciplist, 1))
			temp_nomem();
		reciplist.sa[reciplist.len] = sauninit;
		addrmangle(reciplist.sa + reciplist.len, *recips, &flagalias, !relayhost);
		if (!flagalias)
			flagallaliases = 0;
		++reciplist.len;
		++recips;
	}
	random = now() + (getpid() << 16);
	switch (relayhost ? dns_ip(&ip, &host) : dns_mxip(&ip, &host, random))
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
	for (i = 0; i < ip.len; ++i)
	{
		if (ipme_is46(&ip.ix[i]))
		{
			if (ip.ix[i].pref < prefme)
				prefme = ip.ix[i].pref;
		}
	}
	if (relayhost)
		prefme = 300000;
	if (flagallaliases)
		prefme = 500000;
	for (i = 0; i < ip.len; ++i)
	{
		if (ip.ix[i].pref < prefme)
			break;
	}
	if (i >= ip.len)
		perm_ambigmx();
	x = 0;
	for (i = j = 0; i < ip.len; ++i)
	{
		if (ip.ix[i].pref < prefme)
		{
			if (flagtcpto && tcpto(&ip.ix[i], min_penalty))
				continue;
#ifdef IPV6
			if ((smtpfd = (ip.ix[i].af == AF_INET ? socket_tcp4() : socket_tcp6())) == -1)
#else
    		if ((smtpfd = socket_tcp4()) == -1)
#endif
				temp_oserr();
			if (!x)
			{
				if (!(x = alloc(IPFMT + 1)))
					temp_nomem();
			} else
			if (!alloc_re((char *) &x, j, j + IPFMT + 1))
				temp_nomem();
			j += ip_fmt(x + j, &ip.ix[i].addr.ip);
			x[j++] = ',';
			x[j] = 0;
			if (!timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconnect))
			{
				if (flagtcpto)
					tcpto_err(&ip.ix[i], 0, max_tolerance);
				partner = ip.ix[i];
#ifdef TLS
				partner_fqdn = ip.ix[i].fqdn;
#endif
				smtp(); /*- only returns when the next MX is to be tried */
				smtp_error = 1;
			} else
				smtp_error = 0;
			/*-
			 * Add network errors preventing smtp connections.
			 * Add the IP address to tcp timeout table
			 * and try the next MX
			 */
			if (flagtcpto)
			{
				errors = (errno == error_timeout || errno == error_connrefused || errno == error_hostdown || 
					errno == error_netunreach || errno == error_hostunreach || smtp_error);
				tcpto_err(&ip.ix[i], errors, max_tolerance);
			}
			close(smtpfd);
		}
	}
	if (j)
	{
		x[j - 1] = 0; /*- remove the last ',' */
		temp_noconn(&host, x, port);
	} else
		temp_noconn(&host, 0, 0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_remote_c()
{
	static char    *x = "$Id: qmail-remote.c,v 1.53 2010-02-01 10:11:30+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
