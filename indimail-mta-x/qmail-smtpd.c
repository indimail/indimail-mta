/*
 * $Log: qmail-smtpd.c,v $
 * Revision 1.10  2023-02-08 11:19:49+05:30  Cprogrammer
 * fixed format string for subprintf
 *
 * Revision 1.9  2023-01-18 00:02:13+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.8  2022-11-23 15:08:03+05:30  Cprogrammer
 * rename mysql_lib to libmysql on upgrade
 *
 * Revision 1.7  2022-10-31 12:53:17+05:30  Cprogrammer
 * do chdir /etc/indimail as default cwd
 *
 * Revision 1.6  2022-10-15 12:09:18+05:30  Cprogrammer
 * updated control file list and env variable list
 *
 * Revision 1.5  2022-08-17 13:12:35+05:30  Cprogrammer
 * -v option added to display feature list and exit
 *
 * Revision 1.4  2020-07-05 10:58:04+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2004-10-22 20:29:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:21:51+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <strerr.h>
#include <str.h>
#include <unistd.h>
#include <env.h>
#include <qprintf.h>
#include <subfd.h>
#include <noreturn.h>
#ifdef TLS
#include <openssl/opensslv.h>
#endif
#include "variables.h"
#include "hassmtputf8.h"
#include "hasmysql.h"
#include "haslibgsasl.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"
#ifdef HASLIBGSASL
#include <gsasl.h>
#endif

void            qmail_smtpd(int, char **, char **);
void            out(char *);
void            flush();
char           *getversion_smtpd_c();

no_return void
print_details()
{
	char           *ptr;
	char          **p;
	char            revision[28];
	int             i;
	char           *auth_methods[] = {
		"login", "plain", "cram-md5", "cram-sha1", "cram-sha224", "cram-sha256",
		"cram-sha384", "cram-sha512", "cram-ripemd", "digest-md5", "scram-sha-1",
#ifdef HASLIBGSASL
		"scram-sha-256", "scram-sha-1-plus", "scram-sha-256-plus",
#endif
		0};
	char           *control_files[] = {
		"badhost", "badmailpatterns", "badrcptpatterns", "spamignore", "spamignorepatterns",
		"tlsclients", "tlsservermethod", "domainqueue", "from.envrules", "rcpt.envrules",
		"databytes", "maxhops", "dnsbllist", "envnoathost", "helohost", "greetdelay",
		"localiphost", "me", "morercpthosts", "signatures", "bodycheck", "qregex",
		"morercpthosts.cdb", "nodnscheck", "rcpthosts", "smtpgreeting", "locals",
		"timeoutsmtpd", "timeoutread", "timeoutwrite", "virtualdomains",
		"etrnhosts", "relayclients", "relaydomains", "tarpitcount", "tarpitdelay",
		"maxrecipients", "maxcmdlen", "blackholedpatterns", "blackholedrcptpatterns",
		"goodrcptpatterns", "badextpatterns",
#ifdef USE_SPF
		"spfbehavior", "spfexp", "spfguess", "spfrules",
#endif
#ifdef BATV
		"batvkey", "batvkeystale", "batvnosignlocals", "batvnosignremote",
#endif
#ifdef HAVESRS
		"srs_domain", "srs_secrets", "srs_maxage", "srs_hashlength", "srs_hashmin",
		"srs_alwaysrewrite", "srs_separator",
#endif
		"servercert.pem", "tlsserverciphers", "libmysql", 0};
	char          *cdb_sql_files[] = {
		"authdomains",  "badhelo",  "badext",  "badmailfrom", "badrcptto", "blackholedsender",
		"blackholedrcpt", "chkrcptdomains", "goodrcptto", "relaymailfrom", "spamignore",
		"greylist.white", "tlsa.white", "tlsadomains", "badip", 0};
	char          *read_env_vars[] = {
		"ANTISPOOFING", "AUTH_ALL", "AUTHRULES", "BADHELO", "BADHELOCHECK",
		"BADHOST", "BADHOSTCHECK", "BADIP", "BADIPCHECK",
#ifdef USE_SPF
		"SPFBEHAVIOR", "SPFIPV6",
#endif
#ifdef BATV
		"BATVNOSIGNLOCALS", "BATVNOSIGNREMOTE", "BATVKEY", "BATVKEYSTALE",
#endif
		"BODYCHECK", "BOUNCEMAIL", "CERTDIR", "CHECKRECIPIENT", "CHECKRELAY", "CLIENTCA",
		"CLIENTCRL", "CONTROLDIR", "CUGMAIL", "DATABYTES", "DATABYTES_NOTIFY",
		"DEFAULT_DOMAIN", "DISABLE_AUTH_LOGIN", "DISABLE_AUTH_PLAIN", "DISABLE_CRAM_MD5",
		"DISABLE_CRAM_RIPEMD", "DISABLE_CRAM_SHA1", "DISABLE_CRAM_SHA224",
		"DISABLE_CRAM_SHA256", "DISABLE_CRAM_SHA384", "DISABLE_CRAM_SHA512",
		"DISABLE_DIGEST_MD5", "DISABLE_HELP", "DISABLE_PLUGIN", "DISABLE_SCRAM_SHA1",
		"DISABLE_SCRAM_SHA1_PLUS", "DISABLE_SCRAM_SHA256", "DISABLE_SCRAM_SHA256_PLUS",
		"DISABLE_SCRAM_SHA512", "DISABLE_SCRAM_SHA512_PLUS", "DISABLE_VRFY",
		"DOMAINQUEUE", "ENFORCE_FQDN_HELO", "FORCE_TLS", "FROMRULES", "GREYIP",
		"LOGFD", "LOGFILTER", "MASQUERADE", "MAX_RCPT_ERRCOUNT", "NODNSCHECK",
		"OPENRELAY", "PLUGINDIR", "RELAYCLIENT", "REQPTR", "REQUIREAUTH", "SECURE_AUTH",
		"SERVERCERT", "SHUTDOWN", "SIGNATURES", "SMTP_PLUGIN", "SMTP_PLUGIN_SYMB",
		"SMTPS", "SMTPUTF8", "SPAMFILTER",
#ifdef HAVESRS
		"SRS_DOMAIN", "SRS_SECRETS", "SRS_MAXAGE", "SRS_HASHLENGTH", "SRS_HASHMIN",
		"SRS_ALWAYSREWRITE", "SRS_SEPARATOR",
#endif
		"STARTTLS", "TARPITCOUNT", "TARPITDELAY", "GREETDELAY",
		"TLS_CIPHER_LIST", "TMPDIR", "VIRTUAL_PKG_LIB", "VIRUSCHECK", 0};
	char          *write_env_vars[] = {
		"AUTHENTICATED", "AUTHINFO", "NULLQUEUE",
#ifdef USE_SPF
		"SPFRESULT",
#endif
		"QREGEX", "TCP6LOCALIP", "TCP6REMOTEIP", "TCPLOCALHOST", "TCPLOCALIP",
		"TCPLOCALPORT", "TCPPARANOID", "TCPREMOTEHOST", "TCPREMOTEINFO", "TCPREMOTEIP", 0};


	subprintf(subfdout, "Basic Information\n--------------------------------------------\n");
	subprintf(subfdout, "%-35s: ", "qmail-smtpd version");
	ptr = getversion_smtpd_c();
	str_copyb(revision, ptr, 27);
	i = str_chr(revision, ' ');
	if (revision[i])
		revision[i] = 0;
	subprintf(subfdout, "%s\n", revision);

#ifdef TLS
#if defined(OPENSSL_FULL_VERSION_STR) || defined(OPENSSL_VERSION_STR)
	subprintf(subfdout, "%-35s: ", "OpenSSL");
#if defined(OPENSSL_VERSION_TEXT)
	subprintf(subfdout, "%s", OPENSSL_VERSION_TEXT);
#elif defined(OPENSSL_FULL_VERSION_STR)
	subprintf(subfdout, "%s", OPENSSL_FULL_VERSION_STR);
#elif defined(OPENSSL_VERSION_STR)
	subprintf(subfdout, "%s", OPENSSL_VERSION_STR);
#endif
	subprintf(subfdout, "\n");
#endif
#endif
	subprintf(subfdout, "%-35s: ", "SMTPS");
#ifdef TLS
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "STARTTLS");
#ifdef TLS
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif

	subprintf(subfdout, "%-35s: ", "SASL");
#ifdef HASLIBGSASL
	subprintf(subfdout, "Yes, GSASL %s\n", gsasl_check_version(GSASL_VERSION));
#else
	subprintf(subfdout, "No\n");
#endif

	subprintf(subfdout, "%-35s: ", "SPF");
#ifdef USE_SPF
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: Yes\n", "Control files in CDB");

	subprintf(subfdout, "%-35s: ", "Control files in MySQL");
#ifdef HAS_MYSQL
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "Bounce Address Tag Validation");
#ifdef BATV
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "SMTP Plugins");
#ifdef SMTP_PLUGIN
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "GSASL");
#ifdef HASLIBGSASL
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "IPv6");
#ifdef IPV6
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	subprintf(subfdout, "%-35s: ", "Email Address Internationalization");
#ifdef SMTPUTF8
	subprintf(subfdout, "Yes\n");
#else
	subprintf(subfdout, "No\n");
#endif
	for (p = auth_methods; *p; p++) {
		qprintf(subfdout, "AUTH ", "%s");
		qprintf(subfdout, *p, "%-30s");
		qprintf(subfdout, ": ", "%s");
		subprintf(subfdout, "Yes\n");
	}

	qprintf(subfdout, "\nControl Files\n--------------------------------------------\n", "%s");
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (chdir(controldir) == -1)
		strerr_die3sys(111, "chdir: ", controldir, ": ");
	for (p = control_files; *p; p++) {
		qprintf(subfdout, *p, "%-35s");
		qprintf(subfdout, ": ", "%s");
		qprintf(subfdout, access(*p, F_OK) ? "No\n" : "Yes\n", "%s");
	}
	for (p = cdb_sql_files; *p; p++) {
		qprintf(subfdout, *p, "%-35s");
		qprintf(subfdout, ": ", "%s");
		qprintf(subfdout, access(*p, F_OK) ? "No\n" : "Yes\n", "%s");
	}

	qprintf(subfdout, "\nEnvironment Variables Read\n--------------------------------------------\n", "%s");
	for (p = read_env_vars; *p; p++) {
		qprintf(subfdout, *p, "%-35s");
		qprintf(subfdout, "\n", "%s");
	}
	qprintf(subfdout, "\nEnvironment Variables Set\n--------------------------------------------\n", "%s");
	for (p = write_env_vars; *p; p++) {
		qprintf(subfdout, *p, "%-35s");
		qprintf(subfdout, "\n", "%s");
	}
	substdio_flush(subfdout);
	_exit(0);
}

int
main(int argc, char **argv)
{
	if (argc > 1 && !str_diff(argv[1], "-v"))
		print_details();
	if (chdir(auto_sysconfdir) == -1)
		strerr_die3sys(111, "chdir: ", auto_sysconfdir, ": ");
	qmail_smtpd(argc, argv, 0);
	return(0);
}

void
getversion_qmail_smtpd_c()
{
	static char    *x = "$Id: qmail-smtpd.c,v 1.10 2023-02-08 11:19:49+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
