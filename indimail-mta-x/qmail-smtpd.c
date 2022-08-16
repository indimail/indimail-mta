/*
 * $Log: qmail-smtpd.c,v $
 * Revision 1.5  2022-08-17 02:06:54+05:30  Cprogrammer
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
#include <fmt.h>
#include <unistd.h>
#include <sgetopt.h>
#include <env.h>
#include <substdio.h>
#include <qprintf.h>
#include <subfd.h>
#ifdef TLS
#include <openssl/opensslv.h>
#endif
#include "variables.h"
#include "hassmtputf8.h"
#include "hasmysql.h"
#include "haslibgsasl.h"
#include "auto_control.h"

void            qmail_smtpd(int, char **, char **);
void            out(char *);
void            flush();
char           *getversion_smtpd_c();

void
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
		"signkey", "signkeystale", "nosignhosts", "nosignmydoms",
#endif
		"servercert.pem", "tlsserverciphers", "mysql_lib", 0};
	char          *cdb_sql_files[] = {
		"authdomains",  "badhelo",  "badext",  "badmailfrom", "badrcptto", "blackholedsender",
		"blackholedrcpt", "chkrcptdomains", "goodrcptto", "relaymailfrom", "spamignore",
		"greylist.white", "tlsa.white", "tlsadomains", "badip", 0};
	char          *read_env_vars[] = {
		"ANTISPOOFING", "AUTH_ALL", "AUTHRULES", "BADHELO", "BADHELOCHECK",
		"BADHOST", "BADHOSTCHECK", "BADIP", "BADIPCHECK", "BODYCHECK",
		"BOUNCEMAIL", "CERTDIR", "CHECKRECIPIENT", "CHECKRELAY", "CLIENTCA",
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
		"SERVERCERT", "SHUTDOWN", "SMTP_PLUGIN", "SMTP_PLUGIN_SYMB", "SMTPS", "SMTPUTF8",
		"SPAMFILTER", "STARTTLS", "TLS_CIPHER_LIST", "TMPDIR", "VIRTUAL_PKG_LIB", "VIRUSCHECK", 0};
	char          *write_env_vars[] = {
		"AUTHENTICATED", "AUTHINFO", "NULLQUEUE", "QREGEX", "TCP6LOCALIP", "TCP6REMOTEIP",
		"TCPLOCALHOST", "TCPLOCALIP", "TCPLOCALPORT", "TCPPARANOID", "TCPREMOTEHOST",
		"TCPREMOTEINFO", "TCPREMOTEIP", 0};


	qprintf(subfdout, "Basic Information\n--------------------------------------------\n", "%s");
	qprintf(subfdout, "qmail-smtpd version", "%-35s");
	qprintf(subfdout, ": ", "%s");
	ptr = getversion_smtpd_c();
	str_copyb(revision, ptr, 27);
	i = str_chr(revision, ' ');
	if (revision[i])
		revision[i] = 0;
	qprintf(subfdout, revision, "%s");
	qprintf(subfdout, "\n", "%s");
#ifdef TLS
	qprintf(subfdout, "Openssl version", "%-35s");
	qprintf(subfdout, ": ", "%s");
	qprintf(subfdout, OPENSSL_FULL_VERSION_STR, "%s");
	qprintf(subfdout, "\n", "%s");
#endif
	qprintf(subfdout, "SMTPS", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef TLS
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "STARTTLS", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef TLS
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif

	qprintf(subfdout, "SPF", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef USE_SPF
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "Control files in CDB", "%-35s");
	qprintf(subfdout, ": ", "%s");
	qprintf(subfdout, "Yes\n", "%s");

	qprintf(subfdout, "Control files in MySQL", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef HAS_MYSQL
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "Bounce Address Tag Validation", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef BATV
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "SMTP Plugins", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef SMTP_PLUGIN
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "GSASL", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef HASLIBGSASL
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "IPv6", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef IPV6
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	qprintf(subfdout, "Email Address Internationalization", "%-35s");
	qprintf(subfdout, ": ", "%s");
#ifdef SMTPUTF8
	qprintf(subfdout, "Yes\n", "%s");
#else
	qprintf(subfdout, "No\n", "%s");
#endif
	for (p = auth_methods; *p; p++) {
		qprintf(subfdout, "AUTH ", "%s");
		qprintf(subfdout, *p, "%-30s");
		qprintf(subfdout, ": ", "%s");
		qprintf(subfdout, "Yes\n", "%s");
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
}

int
main(int argc, char **argv)
{
	int             opt;
	char           *usage = "usage: qmail-smgtpd [-v] options\n";

	while ((opt = getopt(argc, argv, "v")) != opteof) {
		switch (opt)
		{
		case 'v':
			print_details();
			_exit(0);
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	qmail_smtpd(argc - optind, argv + optind, 0);
	return(0);
}

void
getversion_qmail_smtpd_c()
{
	static char    *x = "$Id: qmail-smtpd.c,v 1.5 2022-08-17 02:06:54+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
