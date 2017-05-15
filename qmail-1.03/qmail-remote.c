/*
 * $Log: qmail-remote.c,v $
 * Revision 1.106  2017-05-15 19:18:53+05:30  Cprogrammer
 * use environment variable SMTPROUTEFILE, QMTPROUTEFILE, MORESMTPROUTECDB to configure smtproutes, qmtproutes, moresmtproutes.cdb filenames
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
#include "auto_control.h"
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
#include "wait.h"
#include <unistd.h>
#ifdef TLS
#include <sys/stat.h>
#include "tls.h"
#include "ssl_timeoutio.h"
#include <openssl/x509v3.h>
#include "auth_cram.h"
#include "auth_digest_md5.h"
#include "hastlsv1_1_client.h"
#include "hastlsv1_2_client.h"
#endif

#define EHLO 1
#define HUGESMTPTEXT  5000
#define MIN_PENALTY   3600
#define MAX_TOLERANCE 120

#define PORT_SMTP     25 /*- silly rabbit, /etc/services is for users */
#define PORT_QMTP     209 

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
stralloc        helohosts = { 0 };
stralloc        outgoingip = { 0 };
int             cntrl_stat1, cntrl_stat2;
stralloc        smtproutes = { 0 };
stralloc        qmtproutes = { 0 };
struct constmap mapsmtproutes;
struct constmap mapqmtproutes;
stralloc        bounce = {0};
int             protocol_t = 0; /*- defines smtps, smtp, qmtp */
#ifdef MXPS
int             mxps = 0;
#endif
struct constmap maplocalips;
struct constmap maphelohosts;
stralloc        host = { 0 };
stralloc        rhost = { 0 }; /*- host to which qmail-remote ultimately connects */
stralloc        sender = { 0 };
stralloc        qqeh = { 0 };
stralloc        user = { 0 };
stralloc        pass = { 0 };
stralloc        slop  = { 0 };
stralloc        chal  = { 0 };
saa             reciplist = { 0 };
struct ip_mx    partner;
union v46addr   outip;
static int      inside_greeting = 0;
static char    *msgsize, *use_auth_smtp;
char          **my_argv;
int             my_argc;
#ifdef TLS
const char     *ssl_err_str = 0;
#endif
int             fdmoreroutes = -1;
int             flagtcpto = 1;
int             min_penalty = MIN_PENALTY;
unsigned long   max_tolerance = MAX_TOLERANCE;
stralloc        smtptext = { 0 };
stralloc        smtpenv = { 0 };

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
	if (s2)
	{
		if (substdio_puts(subfderr, ": ") == -1)
			_exit(0);
		if (substdio_puts(subfderr, s2) == -1)
			_exit(0);
	}
	if (s3)
	{
		if (substdio_puts(subfderr, ": ") == -1)
			_exit(0);
		if (substdio_puts(subfderr, s3) == -1)
			_exit(0);
	}
	if (substdio_puts(subfderr, ": ") == -1)
		_exit(0);
	if (substdio_puts(subfderr, error_str(errno)) == -1)
		_exit(0);
	if (substdio_puts(subfderr, "\n") == -1)
		_exit(0);
	substdio_flush(subfderr);
}

/*
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
	case 1: /*- success */
		str = "ONSUCCESS_REMOTE";
		break;
	case 0: /*- failure */
		str = "ONFAILURE_REMOTE";
		break;
	case -1: /*- transient error */
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
		_exit (1);
	}
	if (!(args = (char **) alloc(my_argc + 1)))
	{
		my_error("alert: Out of memory", 0, 0);
		_exit (1);
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
		case 1: /*- success */
			if (!env_unset("ONFAILURE_REMOTE")) {
				my_error("alert: out of memory", 0, 0);
				_exit (1);
			}
			if (!env_unset("ONTEMPORARY_REMOTE")) {
				my_error("alert: out of memory", 0, 0);
				_exit (1);
			}
			break;
		case 0: /*- failure */
			if (!env_unset("ONSUCCESS_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit (1);
			}
			if (!env_unset("ONTEMPORARY_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit (1);
			}
			break;
		case -1:
			if (!env_unset("ONSUCCESS_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit (1);
			}
			if (!env_unset("ONFAILURE_REMOTE")) {
				my_error("alert: Out of memory", 0, 0);
				_exit (1);
			}
			break;
		}
		/*- copy all arguments */
		for (i = 0;i < my_argc;i++)
			args[i] = my_argv[i];
		args[i] = 0;
		execv(prog, args);
		my_error("alert: Unable to run", prog, 0);
		_exit (1);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat))
	{
		my_error("alert", prog, "crashed");
		_exit (1);
	}
	return (wait_exitcode(wstat));
}

void
zero()
{
	if (substdio_put(subfdoutsmall, "\0", 1) == -1)
		_exit(0);
}

/*
 * succ  1 - success
 * succ  0 - perm failure
 * succ -1 - temp failure
 */
void
zerodie(char *s1, int succ)
{
	zero();
	substdio_flush(subfdoutsmall);
	_exit(run_script(s1 ? s1[0] : 'Z', succ));
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
			||env_get("ONTEMPORARY_REMOTE"))
	{
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
			if (!env_put2((type == 'q' || mxps) ? "QMTPTEXT" : "SMTPTEXT", smtpenv.s)) {
				return (1);
			}
		}
		if (code >= 0)
		{
			strnum[fmt_ulong(strnum, code)] = 0;
			if (!env_put2("SMTPCODE", strnum))
				return (1);
		} else /*- shouldn't happen - no additional code available from remote host. BUG in code */
		if (!env_put2((type == 'q' || mxps) ? "QMTPCODE" : "SMTPCODE", "-1"))
			temp_nomem();
	}
	return (0);
}

void
temp_nomem()
{
	out("ZOut of memory. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Out of memory. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_noip()
{
	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	out("Zinvalid ipaddr in ");
	out(controldir);
	out("/outgoingip (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "invalid ipaddr in "))
		temp_nomem();
	if (!stralloc_cats(&smtptext, controldir))
		temp_nomem();
	if (!stralloc_cats(&smtptext, "/outgoingip (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_oserr()
{
	out("ZSystem resources temporarily unavailable. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "System resources temporarily unavailable. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_read()
{
	out("ZUnable to read message. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to read message. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_dnscanon()
{
	out("ZCNAME lookup failed temporarily. (#4.4.3)\n");
	if (!stralloc_copys(&smtptext, "CNAME lookup failed temporarily. (#4.4.3)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_dns()
{
	out("ZSorry, I couldn't find any host by that name. (#4.1.2)\n");
	if (!stralloc_copys(&smtptext, "Sorry, I couldn't find any host by that name. (#4.1.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_chdir()
{
	out("ZUnable to switch to home directory. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to switch to home directory. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_control()
{
	out("ZUnable to read control files. (#4.3.0)\n");
	if (!stralloc_copys(&smtptext, "Unable to read control files. (#4.3.0)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
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

void
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

void
perm_dns()
{
	char           *r = "DSorry, I couldn't find any host named ";

	out(r);
	outsafe(&host);
	out(". (#5.1.2)\n");
	if (!stralloc_copys(&smtptext, r + 1))
		temp_nomem();
	if (!stralloc_cat(&smtptext, &host))
		temp_nomem();
	if (!stralloc_cats(&smtptext, ". (#5.1.2)"))
		temp_nomem();
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie(r, 0);
}

void
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

void
perm_ambigmx()
{
	char           *r = "DSorry. Although I'm listed as a best-preference MX or A for that host,\nit isn't in my ";

	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	out(r);
	out(controldir);
	out("/locals file, so I don't treat it as local. (#5.4.6)\n");
	if (!stralloc_copys(&smtptext, r + 1))
		temp_nomem();
	if (!stralloc_cats(&smtptext, controldir))
		temp_nomem();
	if (!stralloc_cats(&smtptext, "/locals file, so I don't treat it as local. (#5.4.6)"))
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
	if (!stralloc_copyb(&rhost, x, len))
		temp_nomem();
	if (substdio_put(subfdoutsmall, x, len) == -1)
		_exit(0);
	if (!stralloc_0(&rhost))
		temp_nomem();
	if (!env_put2("SMTPHOST", rhost.s)) {
		temp_nomem();
	}
}

int             flagcritical = 0;

void
dropped()
{
	char            strnum[FMT_ULONG];

	out("ZConnected to ");
	outhost();
	out(" but connection died. ");
	if (!stralloc_copys(&smtptext, "Connected to "))
		temp_nomem();
	if (!stralloc_copyb(&smtptext, rhost.s, rhost.len - 1))
		temp_nomem();
	if (!stralloc_cats(&smtptext, " but connection died. "))
		temp_nomem();
	if (flagcritical) {
		out("Possible duplicate! ");
		if (!stralloc_cats(&smtptext, "Possible duplicate! "))
		temp_nomem();
	}
	else
	if (inside_greeting) /*- RFC 2821 - client should treat connection failures as temporary error */
	{
		out("Will try alternate MX. ");
		if (!stralloc_cats(&smtptext, "Will try alternate MX. "))
			temp_nomem();
		if (flagtcpto)
		{
			out("tcpto interval ");
			strnum[fmt_ulong(strnum, max_tolerance)] = 0;
			out(strnum);
			out(" ");
			if (!stralloc_cats(&smtptext, "tcpto interval "))
				temp_nomem();
			if (!stralloc_cats(&smtptext, strnum))
				temp_nomem();
			if (!stralloc_catb(&smtptext, " ", 1))
				temp_nomem();
			tcpto_err(&partner, 1, max_tolerance);
		}
	}
#ifdef TLS
	if (ssl_err_str)
	{
		out((char *) ssl_err_str);
		out(" ");
		if (!stralloc_cats(&smtptext, (char *) ssl_err_str))
			temp_nomem();
		if (!stralloc_catb(&smtptext, " ", 1))
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

void
temp_noconn(stralloc *h, char *ip, int port)
{
	char            strnum[FMT_ULONG];

	if (!stralloc_copys(&smtptext, ""))
		temp_nomem();
	temp_noconn_out("ZSorry, I wasn't able to establish an ");
	temp_noconn_out((protocol_t == 'q' || mxps) ? "QMTP connection for " : "SMTP connection for ");
	if (substdio_put(subfdoutsmall, h->s, h->len) == -1)
		_exit(0);
	if (!stralloc_catb(&smtptext, h->s, h->len))
		temp_nomem();
	if (ip)
	{
		temp_noconn_out(" to ");
		temp_noconn_out(ip);
		temp_noconn_out(" port ");
		strnum[fmt_ulong(strnum, port)] = 0;
		temp_noconn_out(strnum);
		temp_noconn_out(" bind IP [");
		substdio_put(subfdoutsmall, outgoingip.s, outgoingip.len);
		temp_noconn_out("]");
		if (!stralloc_catb(&smtptext, outgoingip.s, outgoingip.len))
			temp_nomem();
		if (!stralloc_copys(&rhost, ip))
			temp_nomem();
		if (!stralloc_0(&rhost))
			temp_nomem();
		if (!env_put2("SMTPHOST", rhost.s)) {
			temp_nomem();
		}
		alloc_free(ip);
	}
	temp_noconn_out(". (#4.4.1)\n");
	if (setsmtptext(0, protocol_t))
		smtpenv.len = 0;
	zerodie("Z", -1);
}

void
temp_qmtp_noconn(stralloc *h, char *ip, int port)
{
	char            strnum[FMT_ULONG];

	temp_noconn_out("ZSorry, I wasn't able to establish an QMTP connection for ");
	if (substdio_put(subfdoutsmall, h->s, h->len) == -1)
		_exit(0);
	if (ip)
	{
		temp_noconn_out(" to ");
		temp_noconn_out(ip);
		temp_noconn_out(" port ");
		strnum[fmt_ulong(strnum, port)] = 0;
		temp_noconn_out(strnum);
		if (!stralloc_copys(&rhost, ip))
			temp_nomem();
		if (!stralloc_0(&rhost))
			temp_nomem();
		if (!env_put2("SMTPHOST", rhost.s)) {
			temp_nomem();
		}
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

char            inbuf[1500];
char            smtptobuf[1500];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        smtpto = SUBSTDIO_FDBUF(safewrite, -1, smtptobuf, sizeof smtptobuf);
char            smtpfrombuf[128];
substdio        smtpfrom = SUBSTDIO_FDBUF(saferead, -1, smtpfrombuf, sizeof smtpfrombuf);

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
	if (protocol_t == 'q') /* QMTP */
		return 0;
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

	if (smtptext.s && smtptext.len)
	{
		out("Remote host said: ");
		for (i = 0; i < smtptext.len; ++i)
		{
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
void
quit(char *prepend, char *append, int code, int die)
{
	substdio_putsflush(&smtpto, "QUIT\r\n");
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

char           *partner_fqdn = 0;

void
tls_quit(const char *s1, char *s2, char *s3, char *s4, stralloc *sa)
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
	if (sa && sa->len) {
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

	/*- shouldn't talk to the client unless in an appropriate state */
	state = ssl ? ssl->state : SSL_ST_BEFORE;
	if ((state & SSL_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		substdio_putsflush(&smtpto, "QUIT\r\n");
	out(ssl ? "; connected to " : "; connecting to ");
	outhost();
	out(".\n");
	setsmtptext(-1, 0);
	outsmtptext();
	if (env_get("DEBUG") && ssl)
	{
		X509           *peercert;

		out("STARTTLS proto=");
		out((char *) SSL_get_version(ssl));
		out("; cipher=");
		out((char *) SSL_get_cipher(ssl));

		/*- we want certificate details */
		if ((peercert = SSL_get_peer_certificate(ssl)))
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
	zerodie((char *) s1, -1);
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

/*
 * returns 0 --> fallback to non-tls
 *    if certs do not exist
 *    host is in notlshosts
 *    smtps == 0 and tls session cannot be initated
 * returns 1 if tls session was initated
 * exits on error, smtps == 1 and tls session did not succeed
 */
int
tls_init()
{
	int             code, i = 0, needtlsauth = 0;
	const char     *ciphers;
	char           *t;
	SSL            *myssl;
	SSL_CTX        *ctx;
	stralloc        saciphers = {0}, tlsFilename = {0}, clientcert = {0};
	stralloc        ssl_option = {0};
	int             method = 4; /* (1..2 unused) [1..3] = ssl[1..3], 4 = tls1, 5=tls1.1, 6=tls1.2 */
	int             method_fail = 1;

	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&tlsFilename, controldir))
		temp_nomem();
	if (!stralloc_catb(&tlsFilename, "/tlsclientmethod", 16))
		temp_nomem();
	if (!stralloc_0(&tlsFilename))
		temp_nomem();
	if (control_rldef(&ssl_option, tlsFilename.s, 0, "TLSv1") != 1)
		temp_control();
	if (str_equal( ssl_option.s, "SSLv23"))
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
	if (!certdir && !(certdir = env_get("CERTDIR")))
		certdir = auto_control;
	if (!stralloc_copys(&tlsFilename, certdir))
		temp_nomem();
	if (!stralloc_catb(&tlsFilename, "/clientcert.pem", 15))
		temp_nomem();
	if (!stralloc_0(&tlsFilename))
		temp_nomem();
	if (access(tlsFilename.s, F_OK)) {
		alloc_free(tlsFilename.s);
		return (0);
	}
	if (partner_fqdn)
	{
		struct stat     st;
		if (!stralloc_copys(&tlsFilename, certdir))
			temp_nomem();
		if (!stralloc_catb(&tlsFilename, "/tlshosts/", 10))
			temp_nomem();
		if (!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn)))
			temp_nomem();
		if (!stralloc_catb(&tlsFilename, ".pem", 4))
			temp_nomem();
		if (!stralloc_0(&tlsFilename))
			temp_nomem();
		if (stat(tlsFilename.s, &st))
		{
			needtlsauth = 0;
			if (!stralloc_copys(&tlsFilename, certdir))
				temp_nomem();
			if (!stralloc_catb(&tlsFilename, "/notlshosts/", 12))
				temp_nomem();
			if (!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn) + 1))
				temp_nomem();
			if (!stralloc_0(&tlsFilename))
				temp_nomem();
			if (!stat(tlsFilename.s, &st))
			{
				alloc_free(tlsFilename.s);
				return (0);
			}
			if (!stralloc_copys(&tlsFilename, certdir))
				temp_nomem();
			if (!stralloc_catb(&tlsFilename, "/tlshosts/exhaustivelist", 24))
				temp_nomem();
			if (!stralloc_0(&tlsFilename))
				temp_nomem();
			if (!stat(tlsFilename.s, &st))
			{
				alloc_free(tlsFilename.s);
				return (0);
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
				alloc_free(tlsFilename.s);
				return 0;
			}
			if (!stralloc_copyb(&smtptext, "No TLS achieved while ", 22))
				temp_nomem();
			if (!stralloc_cats(&smtptext, tlsFilename.s))
				temp_nomem();
			if (!stralloc_catb(&smtptext, " exists", 7))
				temp_nomem();
			tls_quit("ZNo TLS achieved while", tlsFilename.s, " exists", 0, 0);
		}
	}
	SSL_library_init();
	if (method == 2 && (ctx = SSL_CTX_new(SSLv23_client_method())))
		method_fail = 0;
	else
	if (method == 3 && (ctx=SSL_CTX_new(SSLv3_client_method())))
		method_fail = 0;
	else
	if (method == 4 && (ctx=SSL_CTX_new(TLSv1_client_method())))
		method_fail = 0;
#ifdef TLSV1_1_CLIENT_METHOD
	else
	if (method == 5 && (ctx=SSL_CTX_new(TLSv1_1_client_method())))
		method_fail = 0;
#endif
#ifdef TLSV1_2_CLIENT_METHOD
	else
	if (method == 6 && (ctx=SSL_CTX_new(TLSv1_2_client_method())))
		method_fail = 0;
#endif
	if (method_fail) 
	{
		if (!smtps && !needtlsauth)
		{
			alloc_free(tlsFilename.s);
			SSL_CTX_free(ctx);
			return 0;
		}
		t = (char *) ssl_error();
		if (!stralloc_copyb(&smtptext, "TLS error initializing ctx: ", 28))
			temp_nomem();
		if (!stralloc_cats(&smtptext, t))
			temp_nomem();
		SSL_CTX_free(ctx);
		tls_quit("ZTLS error initializing ctx: ", t, 0, 0, 0);
	}

	if (needtlsauth)
	{
		if (!SSL_CTX_load_verify_locations(ctx, tlsFilename.s, NULL))
		{
			t = (char *) ssl_error();
			if (!stralloc_copyb(&smtptext, "TLS unable to load ", 19))
				temp_nomem();
			if (!stralloc_cats(&smtptext, tlsFilename.s))
				temp_nomem();
			if (!stralloc_catb(&smtptext, ": ", 2))
				temp_nomem();
			if (!stralloc_cats(&smtptext, t))
				temp_nomem();
			SSL_CTX_free(ctx);
			tls_quit("ZTLS unable to load ", tlsFilename.s, ": ", t, 0);
		}
		/*
		 * set the callback here; SSL_set_verify didn't work before 0.9.6c 
		 */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
	}

	/*
	 * let the other side complain if it needs a cert and we don't have one 
	 */
	if (!stralloc_copys(&clientcert, certdir))
		temp_nomem();
	if (!stralloc_catb(&clientcert, "/clientcert.pem", 15))
		temp_nomem();
	if (!stralloc_0(&clientcert))
		temp_nomem();
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert.s))
		SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);
	alloc_free(clientcert.s);
	myssl = SSL_new(ctx);
	if (!myssl)
	{
		t = (char *) ssl_error();
		if (!smtps && !needtlsauth)
		{
			alloc_free(tlsFilename.s);
			SSL_CTX_free(ctx);
			return 0;
		}
		if (!stralloc_copyb(&smtptext, "TLS error initializing ssl: ", 28))
			temp_nomem();
		if (!stralloc_cats(&smtptext, t))
			temp_nomem();
		SSL_CTX_free(ctx);
		tls_quit("ZTLS error initializing ssl: ", t, 0, 0, 0);
	} else
		SSL_CTX_free(ctx);

	if (!smtps)
		substdio_putsflush(&smtpto, "STARTTLS\r\n");

	/*
	 * while the server is preparing a response, do something else 
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
	 * read the response to STARTTLS 
	 */
	if (!smtps)
	{
		if (smtpcode() != 220)
		{
			SSL_free(myssl);
			if (!needtlsauth)
			{
				alloc_free(tlsFilename.s);
				return 0;
			}
			if (!stralloc_copyb(&smtptext, "STARTTLS rejected while ", 24))
				temp_nomem();
			if (!stralloc_cats(&smtptext, tlsFilename.s))
				temp_nomem();
			if (!stralloc_catb(&smtptext, " exists", 7))
				temp_nomem();
			tls_quit("ZSTARTTLS rejected while ", tlsFilename.s, " exists", 0, 0);
		}
	}
	ssl = myssl;
	if (ssl_timeoutconn(timeout, smtpfd, smtpfd, ssl) <= 0)
	{
		t = (char *) ssl_error_str();
		if (!stralloc_copyb(&smtptext, "TLS connect failed: ", 20))
			temp_nomem();
		if (!stralloc_cats(&smtptext, t))
			temp_nomem();
		tls_quit("ZTLS connect failed: ", t, 0, 0, 0);
	}
	if (needtlsauth)
	{
		X509           *peercert;
		STACK_OF(GENERAL_NAME) *gens;

		int             r = SSL_get_verify_result(ssl);
		if (r != X509_V_OK)
		{
			t = (char *) X509_verify_cert_error_string(r);
			if (!stralloc_copyb(&smtptext, "TLS unable to verify server with ", 33))
				temp_nomem();
			if (!stralloc_cats(&smtptext, tlsFilename.s))
				temp_nomem();
			if (!stralloc_catb(&smtptext, ": ", 2))
				temp_nomem();
			if (!stralloc_cats(&smtptext, t))
				temp_nomem();
			tls_quit("ZTLS unable to verify server with ", tlsFilename.s, ": ", t, 0);
		}
		if (!(peercert = SSL_get_peer_certificate(ssl)))
		{
			if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28))
				temp_nomem();
			if (!stralloc_cats(&smtptext, partner_fqdn))
				temp_nomem();
			if (!stralloc_catb(&smtptext, ": no certificate provided", 25))
				temp_nomem();
			tls_quit("ZTLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);
		}

		/*
		 * RFC 2595 section 2.4: find a matching name
		 * first find a match among alternative names 
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
				if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28))
					temp_nomem();
				if (!stralloc_cats(&smtptext, partner_fqdn))
					temp_nomem();
				if (!stralloc_catb(&smtptext, ": certificate contains no valid commonName", 42))
					temp_nomem();
				tls_quit("ZTLS unable to verify server ", partner_fqdn, ": certificate contains no valid commonName", 0, 0);
			}
			if (!match_partner((char *) peer.s, peer.len))
			{
				if (!stralloc_copyb(&smtptext, "TLS unable to verify server ", 28))
					temp_nomem();
				if (!stralloc_cats(&smtptext, partner_fqdn))
					temp_nomem();
				if (!stralloc_catb(&smtptext, ": received certificate for ", 27))
					temp_nomem();
				if (!stralloc_cat(&smtptext, &peer))
					temp_nomem();
				if (!stralloc_0(&smtptext))
					temp_nomem();
				tls_quit("ZTLS unable to verify server ", partner_fqdn, ": received certificate for ", 0, &peer);
			}
		}
		X509_free(peercert);
	}
	alloc_free(tlsFilename.s);
	if (smtps && (code = smtpcode()) != 220)
		quit("ZTLS Connected to ", " but greeting failed", code, -1);
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
mailfrom_xtext(int use_size)
{
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

static char     hextab[] = "0123456789abcdef";
stralloc        realm = {0}, nonce = {0}, digesturi = {0};

/* parse digest response */
unsigned int
scan_response(stralloc *dst, stralloc *src, const char *search)
{
	char           *x = src->s;
	int             i, len;
	unsigned int    slen;

	slen = str_len((char *) search);
	if (!stralloc_copys(dst,""))
		temp_nomem();
	for (i=0; src->len>i+slen; i+=str_chr(x+i, ',')+1) {
		char *s=x+i;
		if (case_diffb(s, slen, (char *) search) == 0) {
			s += slen; /* skip name */
			if (*s++ != '=')
				return 0; /* has to be here! */
			if (*s == '"') { /* var="value" */
				s++;
				len = str_chr(s, '"');
				if (!len)
					return 0;
				if (!stralloc_catb(dst, s, len))
					temp_nomem();
			} else { /* var=value */
				len = str_chr(s, ',');
				if (!len)
					str_len(s); /* should be the end */
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
	unsigned char  *s, *x=cnonce;
	unsigned long   i, len = 0;
	int             code;
	char            z[IPFMT];

	substdio_puts(&smtpto, "AUTH DIGEST-MD5\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (AUTH DIGEST-MD5).", code, -1);
	if ((i = str_chr(smtptext.s + 5, '\n')) > 0) /* Challenge */
	{
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
		*x = hextab[digest[i]/16]; ++x;
		*x = hextab[digest[i]%16]; ++x;
	}
	*x=0;
	if (!stralloc_copyb(&slop, "cnonce=\"", 8))
		temp_nomem();
	if (!stralloc_catb(&slop, (char *) cnonce, 32))
		temp_nomem();
	if (!stralloc_catb(&slop, "\",digest-uri=\"", 14))
		temp_nomem();
	if (!stralloc_copyb(&digesturi, "smtp/", 5))
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
	if (!stralloc_catb(&digesturi, z, len))
		temp_nomem();
	if (!stralloc_cat(&slop, &digesturi))
		temp_nomem();
	if (!stralloc_catb(&slop, "\",nc=00000001,nonce=\"", 21))
		temp_nomem();
	if (!stralloc_cat(&slop, &nonce))
		temp_nomem();
	if (!stralloc_catb(&slop, "\",qop=\"auth\",realm=\"", 20))
		temp_nomem();
	if (control_readfile(&realm, "realm", 1) == -1)
		temp_control();
	realm.len--;
	if (!stralloc_cat(&slop, &realm))
		temp_nomem();
	if (!stralloc_catb(&slop, "\",response=", 11))
		temp_nomem();
	if (!(s = (unsigned char *) digest_md5(user.s, user.len, realm.s, realm.len,
		pass.s, pass.len, 0, nonce.s, nonce.len, digesturi.s, digesturi.len,
		(char *) cnonce, "00000001", "auth")))
		quit("ZConnected to ", " but unable to generate response.", -1, -1);
	if (!stralloc_cats(&slop, (char *) s))
		temp_nomem();
	if (!stralloc_catb(&slop, ",username=\"", 11))
		temp_nomem();
	if (!stralloc_cat(&slop, &user))
		temp_nomem();
	if (!stralloc_catb(&slop, "\"", 1))
		temp_nomem();
	if (b64encode(&slop, &auth))
		quit("ZConnected to ", " but unable to base64encode username+digest.", -1, -1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
	mailfrom_xtext(use_size);
}

void 
auth_cram(int type, int use_size)
{
	int             j, code, iter = 16;
	unsigned char   digest[21], encrypted[41];
	unsigned char  *e;

	switch (type)
	{
	case 3:
		iter = 16;
		substdio_puts(&smtpto, "AUTH CRAM-MD5\r\n");
		substdio_flush(&smtpto);
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-MD5).", code, -1);
		break;
	case 4:
		iter = 20;
		substdio_puts(&smtpto, "AUTH CRAM-SHA1\r\n");
		substdio_flush(&smtpto);
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA1).", code, -1);
		break;
	case 5:
		iter = 20;
		substdio_puts(&smtpto, "AUTH CRAM-SHA256\r\n");
		substdio_flush(&smtpto);
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-SHA256).", code, -1);
		break;
	case 6:
		iter = 20;
		substdio_puts(&smtpto, "AUTH CRAM-RIPEMD\r\n");
		substdio_flush(&smtpto);
		if ((code = smtpcode()) != 334)
			quit("ZConnected to ", " but authentication was rejected (AUTH CRAM-RIPEMD).", code, -1);
		break;
	}
	if ((j = str_chr(smtptext.s + 5, '\n')) > 0) /* Challenge */
	{
		if (!stralloc_copys(&slop, ""))
			temp_nomem();
		if (!stralloc_copyb(&slop, smtptext.s + 4, smtptext.len - 5))
			temp_nomem();
		if (b64decode((unsigned char *) slop.s, slop.len, &chal))
			quit("ZConnected to ", " but unable to base64decode challenge.", -1, -1);
	} else
		quit("ZConnected to ", " but got no challenge.", -1, -1);
	switch (type)
	{
	case 3:
		hmac_md5((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case 4:
		hmac_sha1((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case 5:
		hmac_sha256((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case 6:
		hmac_ripemd((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	}

	for (j = 0, e = encrypted; j < iter; j++) {	/* HEX => ASCII */
		*e = hextab[digest[j]/16]; ++e;
		*e = hextab[digest[j]%16]; ++e;
	} *e = 0;

	if (!stralloc_copy(&slop, &user))
		temp_nomem();	/* user-id */
	if (!stralloc_catb(&slop, " ", 1))
		temp_nomem();
	if (!stralloc_cats(&slop, (char *) encrypted))
		temp_nomem();	/* digest */

	if (b64encode(&slop, &auth))
		quit("ZConnected to ", " but unable to base64encode username+digest.", -1, -1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (username+digest)", code, -1);
	mailfrom_xtext(use_size);
}

void
auth_plain(int use_size)
{
	int             code;

	substdio_puts(&smtpto, "AUTH PLAIN\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (AUTH PLAIN).", code, -1);
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
		quit("ZConnected to ", " but unable to base64encode (plain).", -1, -1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (plain).", code, -1);
	mailfrom_xtext(use_size);
}

void
auth_login(int use_size)
{
	int             code;

	substdio_puts(&smtpto, "AUTH LOGIN\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (AUTH LOGIN).", code, -1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&user, &auth))
		quit("ZConnected to ", " but unable to base64encode user.", -1, -1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 334)
		quit("ZConnected to ", " but authentication was rejected (username).", code, -1);

	if (!stralloc_copys(&auth, ""))
		temp_nomem();
	if (b64encode(&pass, &auth))
		quit("ZConnected to ", " but unable to base64encode pass.", -1, -1);
	substdio_put(&smtpto, auth.s, auth.len);
	substdio_puts(&smtpto, "\r\n");
	substdio_flush(&smtpto);
	if ((code = smtpcode()) != 235)
		quit("ZConnected to ", " but authentication was rejected (password)", code, -1);
	mailfrom_xtext(use_size);
}

void
smtp_auth(char *type, int use_size)
{
	int             i = 0, j, login_supp = 0, plain_supp = 0, cram_md5_supp = 0,
					cram_sha1_supp = 0, cram_sha256_supp = 0, cram_rmd_supp = 0,
					digest_md5_supp = 0, secure_auth;
	char           *ptr, *no_auth_login, *no_auth_plain, *no_cram_md5,
		           *no_cram_sha1, *no_cram_sha256, *no_cram_ripemd,
		           *no_digest_md5;

	if (!type)
	{
		mailfrom(use_size);
		return;
	}
	while ((i += str_chr(smtptext.s + i, '\n') + 1) && (i + 8 < smtptext.len)
		&& str_diffn(smtptext.s + i + 4, "AUTH ", 5));

	for (ptr = smtptext.s + i + 4 + 5;*ptr != '\n' && ptr < smtptext.s + smtptext.len; ptr++)
	{
		if (*ptr == '-')
		{
			if (case_starts(ptr - 6, "DIGEST-MD5"))
				digest_md5_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-RIPEMD"))
				cram_rmd_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA256"))
				cram_sha256_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-SHA1"))
				cram_sha1_supp = 1;
			else
			if (case_starts(ptr - 4, "CRAM-MD5"))
				cram_md5_supp = 1;
		}
	}
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
#ifdef TLS
	secure_auth = env_get("SECURE_AUTH") ? 1 : 0;
	no_auth_login = secure_auth && !ssl ? "" : env_get("DISABLE_AUTH_LOGIN");
	no_auth_plain = secure_auth && !ssl ? "" : env_get("DISABLE_AUTH_PLAIN");
#else
	no_auth_login = env_get("DISABLE_AUTH_LOGIN");
	no_auth_plain = env_get("DISABLE_AUTH_PLAIN");
#endif
	no_cram_md5 = env_get("DISABLE_CRAM_MD5");
	no_cram_sha1= env_get("DISABLE_CRAM_SHA1");
	no_cram_sha256= env_get("DISABLE_CRAM_SHA256");
	no_cram_ripemd= env_get("DISABLE_CRAM_RIPEMD");
	no_digest_md5= env_get("DISABLE_DIGEST_MD5");
	if (!case_diffs(type, "DIGEST-MD5"))
	{
		if (digest_md5_supp)
		{
			auth_digest_md5(use_size);
			return;
		}
	} else
	if (!case_diffs(type, "CRAM-RIPEMD"))
	{
		if (cram_rmd_supp)
		{
			auth_cram(6, use_size);
			return;
		}
	} else
	if (!case_diffs(type, "CRAM-SHA256"))
	{
		if (cram_sha256_supp)
		{
			auth_cram(5, use_size);
			return;
		}
	} else
	if (!case_diffs(type, "CRAM-SHA1"))
	{
		if (cram_sha1_supp)
		{
			auth_cram(4, use_size);
			return;
		}
	} else
	if (!case_diffs(type, "CRAM-MD5"))
	{
		if (cram_md5_supp)
		{
			auth_cram(3, use_size);
			return;
		}
	} else
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
		if (!no_digest_md5 && digest_md5_supp)
		{
			auth_digest_md5(use_size);
			return;
		}
		if (!no_cram_ripemd && cram_rmd_supp)
		{
			auth_cram(6, use_size);
			return;
		}
		if (!no_cram_sha256 && cram_sha256_supp)
		{
			auth_cram(5, use_size);
			return;
		}
		if (!no_cram_sha1 && cram_sha1_supp)
		{
			auth_cram(4, use_size);
			return;
		}
		if (!no_cram_md5 && cram_md5_supp)
		{
			auth_cram(3, use_size);
			return;
		}
		if (!no_auth_login && login_supp)
		{
			auth_login(use_size);
			return;
		}
		if (!no_auth_plain && plain_supp)
		{
			auth_plain(use_size);
			return;
		}
	}
	err_authprot();
	mailfrom(use_size);
	return;
}

void temp_proto()
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
 *  or if the QMTP connection attempt fails, the client tries an SMTP connection to port 25 as
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

void
qmtp(stralloc *h, char *ip, int port)
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
	/*
	 * the following code was substantially taken from serialmail'ss serialqmtp.c 
	 */
	substdio_put(&smtpto, num, fmt_ulong(num, len + 1));
	substdio_put(&smtpto, ":\n", 2);
	while (len > 0) {
		if ((n = substdio_feed(&ssin)) <= 0)
			_exit(32);			/* wise guy again */
		x = substdio_PEEK(&ssin);
		substdio_put(&smtpto, x, n);
		substdio_SEEK(&ssin, n);
		len -= n;
	}
	substdio_put(&smtpto, ",", 1);
	len = sender.len;
	substdio_put(&smtpto, num, fmt_ulong(num, len));
	substdio_put(&smtpto, ":", 1);
	substdio_put(&smtpto, sender.s, sender.len);
	substdio_put(&smtpto, ",", 1);
	len = 0;
	for (i = 0; i < reciplist.len; ++i)
		len += fmt_ulong(num, reciplist.sa[i].len) + 1 + reciplist.sa[i].len + 1;
	substdio_put(&smtpto, num, fmt_ulong(num, len));
	substdio_put(&smtpto, ":", 1);
	for (i = 0; i < reciplist.len; ++i) {
		substdio_put(&smtpto, num, fmt_ulong(num, reciplist.sa[i].len));
		substdio_put(&smtpto, ":", 1);
		substdio_put(&smtpto, reciplist.sa[i].s, reciplist.sa[i].len);
		substdio_put(&smtpto, ",", 1);
	}
	substdio_put(&smtpto, ",", 1);
	substdio_flush(&smtpto);
	flagallok = 1;
	for (i = 0; i < reciplist.len; ++i) {
		len = 0;
		for (;;) {
			get((char *) &ch);
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
		get((char *) &ch);
		--len;
		if ((ch != 'Z') && (ch != 'D') && (ch != 'K'))
			temp_proto();
		if (!stralloc_copyb(&smtptext, (char *) &ch, 1))
			temp_proto();
		if (!stralloc_cats(&smtptext, "Remote host said: "))
			temp_nomem();
		while (len > 0) {
			get((char *) &ch);
			--len;
		}
		for (len = 0; len < smtptext.len; ++len) {
			ch = smtptext.s[len];
			if ((ch < 32) || (ch > 126))
				smtptext.s[len] = '?';
		}
		get((char *) &ch);
		if (ch != ',')
			temp_proto();
		smtptext.s[smtptext.len - 1] = '\n';
		if (smtptext.s[0] == 'K')
			out("r");
		else if (smtptext.s[0] == 'D') {
			out("h");
			flagallok = 0;
		} else {				/* if (smtptext.s[0] == 'Z') */
			out("s");
			flagallok = -1;
		}
		if (substdio_put(subfdoutsmall, smtptext.s + 1, smtptext.len - 1) == -1)
			temp_qmtp_noconn(h, ip, port);
		zero();
	} /*- for (i = 0; i < reciplist.len; ++i) */
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

stralloc         helo_str = { 0 };

void
smtp()
{
	unsigned long   code;
	int             flagbother;
	int             i, use_size = 0, is_esmtp = 1;

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
		return; /* try next MX, see RFC-2821 */
	else
	if (code != 220)
		quit("ZConnected to ", " but greeting failed", code, -1);
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
	/* for broken servers like yahoo */
	if (env_get("TRY_NEXT_MX_HELO_FAIL") && code >= 400 && code < 500)
		return;
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
		quit(code >= 500 ? "DConnected to " : "ZConnected to " , helo_str.s, code, code >= 500 ? 1 : -1);
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
		quit("DConnected to ", " but sender was rejected", code, 1);
	if (code >= 400)
		quit("ZConnected to ", " but sender was rejected", code, -1);
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
		quit("DGiving up on ", "", code, 1);
	substdio_putsflush(&smtpto, "DATA\r\n");
	code = smtpcode();
	if (code >= 500)
		quit("D", " failed on DATA command", code, 1);
	if (code >= 400)
		quit("Z", " failed on DATA command", code, -1);
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
	char           *routes, *senderdomain, *ip, *x;
	char           *smtproutefile, *moresmtproutefile, *qmtproutefile;
	static stralloc controlfile, outgoingipfn;

	if (control_init() == -1)
		temp_control();
	if (control_readint(&timeout, "timeoutremote") == -1)
		temp_control();
	if (control_readint(&timeoutconnect, "timeoutconnect") == -1)
		temp_control();
	if (control_rldef(&helohost, "helohost", 1, (char *) 0) != 1)
		temp_control();
	if ((routes = env_get("QMTPROUTE"))) /* mysql */
	{
		if (!stralloc_copyb(&qmtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat2 = 2;
	} else {
		qmtproutefile = (qmtproutefile = env_get("QMTPROUTEFILE")) ? qmtproutefile : "qmtproutes";
		cntrl_stat2 = control_readfile(&qmtproutes, qmtproutefile, 0);
	}
	switch (cntrl_stat2)
	{
	case -1: /*- error reading qmtproutes */
		temp_control();
	case 0: /*- qmtproutes absent */
		if (!constmap_init(&mapqmtproutes, "", 0, 1))
			temp_nomem();
		break;
	case 1: /*- qmtproutes present */
	case 2:
		if (!constmap_init(&mapqmtproutes, qmtproutes.s, qmtproutes.len, 1))
			temp_nomem();
		break;
	}
	routes = (routes = env_get("SMTPROUTE")) ? routes : env_get("X-SMTPROUTES");
	if (routes) /* mysql or X-SMTPROUTES from header */
	{
		if (!stralloc_copyb(&smtproutes, routes, str_len(routes) + 1))
			temp_nomem();
		cntrl_stat1 = 2;
	} else
	{
		smtproutefile = (smtproutefile = env_get("SMTPROUTEFILE")) ? smtproutefile : "smtproutes";
		moresmtproutefile = (moresmtproutefile = env_get("MORESMTPROUTECDB")) ? moresmtproutefile : "moresmtproutes.cdb";
		cntrl_stat1 = control_readfile(&smtproutes, smtproutefile, 0);
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir))
			temp_nomem();
		else
		if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
			temp_nomem();
		else
		if (!stralloc_cats(&controlfile, moresmtproutefile))
			temp_nomem();
		else
		if (!stralloc_0(&controlfile))
			temp_nomem();
		else
		if ((fdmoreroutes = open_read(controlfile.s)) == -1)
		{
			if (errno != error_noent)
				cntrl_stat1 = -1;
		}
	}
	switch (cntrl_stat1)
	{
	case -1: /*- error reading smtproutes */
		temp_control();
	case 0: /*- smtproutes absent */
		if (!constmap_init(&mapsmtproutes, "", 0, 1))
			temp_nomem();
		break;
	case 1: /*- smtproutes present */
	case 2:
		if (!constmap_init(&mapsmtproutes, smtproutes.s, smtproutes.len, 1))
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
	if ((x = env_get("OUTGOINGIP")) && *x)
	{
		if (!stralloc_copys(&outgoingip, x))
			temp_nomem();
		r = 1;
	} else /*- per recipient domain outgoingip */
	{
		if (!stralloc_copys(&outgoingipfn, "outgoingip."))
			temp_nomem();
		else
		if (!stralloc_cat(&outgoingipfn, &host))
			temp_nomem();
		else
		if (!stralloc_0(&outgoingipfn))
			temp_nomem();
		if (!(r = control_readrandom(&outgoingip, outgoingipfn.s)))
			r = control_readrandom(&outgoingip, "outgoingip");
		if (r == -1)
		{
			if (errno == error_nomem)
				temp_nomem();
			temp_control();
		} else 
		if (r && !env_put2("OUTGOINGIP", outgoingip.s)) {
			my_error("alert: Out of memory", 0, 0);
			_exit (1);
		}
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
	{
		if (!ip6_scan(outgoingip.s, &outip.ip6))
			temp_noip();
		if (ip6_isv4mapped(&outip.ip6.d) && !ip4_scan(outgoingip.s, &outip.ip))
			temp_noip();
	}
#else
	if (0 == r && !stralloc_copys(&outgoingip, "0.0.0.0"))
		temp_nomem();
	if (0 == str_diffn(outgoingip.s, "0.0.0.0", 7))
	{
		int             i;
		for (i = 0;i < 4;i++)
			outip.ip.d[i] = 0;
	} else
	if (!ip4_scan(outgoingip.s, &outip.ip))
		temp_noip();
#endif
	/*- domainbinding patch */
	switch (control_readfile(&localips, (x = env_get("DOMAINBINDINGS")) ? x : "domainbindings", 0))
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

		if ((i = str_rchr(sender.s, '@')))
			senderdomain = sender.s + i + 1;
		if (!stralloc_copyb(&senderbind, senderdomain, sender.len - i - 1))
			temp_nomem();
		ip = constmap(&maplocalips, sender.s, sender.len);
		if (ip && !*ip)
			ip = 0;
		for (i = 0;!ip && i <= senderbind.len;++i)
		{
			if (!i || i == senderbind.len || senderbind.s[i] == '.')
			{
				if ((ip = constmap(&maplocalips, senderbind.s + i, senderbind.len - i)))
					break;
			}
		}
		if (ip && !*ip)
			ip = 0;
		if (ip)
		{ 
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
		temp_control();
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
	switch (ix->af)
	{
#ifdef IPV6
	case AF_INET6:
		return timeoutconn6(fd, &ix->addr.ip6, ip, port, timeout);
		break;
#endif
	case AF_INET:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
		break;
	default:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
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

	sig_pipeignore();
	if (argc < 6)
		perm_usage();
	my_argc = argc;
	my_argv = argv;
	if (chdir(auto_qmail) == -1)
		temp_chdir();
	if (!stralloc_copys(&host, argv[1])) /*- host required by getcontrols below */
		temp_nomem();
	addrmangle(&sender, argv[2], &flagalias, 0);
	if (!stralloc_copys(&qqeh, argv[3]))
		temp_nomem();
	msgsize = argv[4];
	recips = argv + 5;
	getcontrols();
	protocol_t = env_get("SMTPS") ? 'S' : 's';
	use_auth_smtp = env_get("AUTH_SMTP");
	/*- Per user SMTPROUTE functionality using moresmtproutes.cdb */
	relayhost = lookup_host(*recips, str_len(*recips));
	min_penalty = (x = env_get("MIN_PENALTY")) ? scan_int(x, &min_penalty) : MIN_PENALTY;
	max_tolerance = (x = env_get("MAX_TOLERANCE")) ? scan_ulong(x, &max_tolerance) : MAX_TOLERANCE;
	if (sender.len == 0) {     /* bounce routes */
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
	if (cntrl_stat1 ||  cntrl_stat2) /*- set in getcontrols() above */
	{
		/*- Look at qmtproutes/smtproutes */
		for (i = 0; !relayhost && i <= host.len; ++i)
		{
			if ((i == 0) || (i == host.len) || (host.s[i] == '.'))
			{
				/* default qmtproutes */
				if (cntrl_stat2 == 2 && (relayhost = constmap(&mapqmtproutes, host.s + i, host.len - i))) {
					protocol_t = 'q';
					port = PORT_QMTP;
					break;
				} else
				if (cntrl_stat1 == 2 && (relayhost = constmap(&mapsmtproutes, host.s + i, host.len - i)))
				{
					port = PORT_SMTP;
					break;
				} else
				if (cntrl_stat2 && (relayhost = constmap(&mapqmtproutes, host.s + i, host.len - i))) {
					protocol_t = 'q';
					port = PORT_QMTP;
					break;
				} else
				if (cntrl_stat1 && (relayhost = constmap(&mapsmtproutes, host.s + i, host.len - i)))
				{
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
	if (relayhost)
	{
		if (use_auth_smtp)
		{
			/*- 
			 * test.com:x.x.x.x:x user pass 
			 *         or
			 * domain:relay:port:penalty:max_tolerance username password
			 */
			i = str_chr(relayhost, ' ');
			if (relayhost[i])
			{
				relayhost[i] = 0;
				j = str_chr(relayhost + i + 1, ' ');
				if (relayhost[i + j + 1]) /*- if password is present */
				{
					relayhost[i + j + 1] = 0;
					if (relayhost[i + 1] && relayhost[i + j + 2]) /* both user and password are present */
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
		/*- 
		 * test.com:x.x.x.x:x user pass 
		 *         or
		 * domain:relay:port:penalty:max_tolerance
		 */
		i = str_chr(relayhost, ':');
		if (relayhost[i]) {
			if (relayhost[i + 1]) { /* port is present */
				scan_ulong(relayhost + i + 1, &port);
				relayhost[i] = 0;
				x = relayhost + i + 1; /* port */
				i = str_chr(x, ':'); /*- : before min_penalty */
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
				relayhost[i] = 0;
		}
		if (!stralloc_copys(&host, relayhost))
			temp_nomem();
	} else
		use_auth_smtp = 0;
#if BATV
	if (batvok && sender.len && signkey.len) {
		int             j;

		if (!stralloc_0(&sender))
			temp_nomem(); /* null terminate */
		sender.len--;
		i = str_rchr(*recips, '@');	/* should check all recips, not just the first */
		j = str_rchr(sender.s, '@');
		if (!constmap(&mapnosign, *recips + i + 1, str_len(*recips + i + 1)) 
				&& !constmap(&mapnosigndoms, sender.s + j + 1, sender.len - (j + 1)))
			sign_batv(); /*- modifies sender */
	}
#endif
	if (!saa_readyplus(&reciplist, 0))
		temp_nomem();
	if (ipme_init() != 1)
		temp_oserr();
	flagallaliases = 1;
	i = (env_get("DISABLE_CNAME_LOOKUP") ? 0 : !relayhost);
	while (*recips)
	{
		if (!saa_readyplus(&reciplist, 1))
			temp_nomem();
		reciplist.sa[reciplist.len] = sauninit;
		addrmangle(reciplist.sa + reciplist.len, *recips, &flagalias, i);
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
#ifdef MXPS
		mxps = 0;
		if (qmtp_priority(ip.ix[i].pref))
			mxps = 1;
		if (protocol_t == 'q' || mxps || ip.ix[i].pref < prefme)
#else
		if (protocol_t == 'q' || ip.ix[i].pref < prefme)
#endif
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
			j += ip4_fmt(x + j, &ip.ix[i].addr.ip);
			x[j++] = ',';
			x[j] = 0;
			for (;;)
			{
				if (!timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconnect))
				{
					if (flagtcpto) /*- clear the error */
						tcpto_err(&ip.ix[i], 0, max_tolerance);
					partner = ip.ix[i];
#ifdef TLS
					partner_fqdn = ip.ix[i].fqdn;
#endif
#ifdef MXPS
					if (mxps != -1 && (protocol_t == 'q' || mxps))
#else
					if (protocol_t == 'q')
#endif
					{
						if (j)
							x[j - 1] = 0;
						qmtp(&host, x, port); /*- does not return */
					} else
						smtp(); /*- only returns when the next MX is to be tried */
					smtp_error = 1;
					break;
				} else /*- connect failed */
				{
#ifdef MXPS
					if (mxps) /*- QMTP failed; try SMTP */
					{
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
			if (flagtcpto)
			{
				errors = (errno == error_timeout || errno == error_connrefused || errno == error_hostdown || 
					errno == error_netunreach || errno == error_hostunreach || smtp_error);
				tcpto_err(&ip.ix[i], errors, max_tolerance);
			}
			close(smtpfd);
		}
	} /*- for (i = j = 0; i < ip.len; ++i) */
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
	static char    *x = "$Id: qmail-remote.c,v 1.106 2017-05-15 19:18:53+05:30 Cprogrammer Exp mbhangui $";
	x=sccsidauthcramh;
	x=sccsidauthdigestmd5h;
	x++;
}
