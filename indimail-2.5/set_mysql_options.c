/*
 * $Log: set_mysql_options.c,v $
 * Revision 2.20  2018-04-01 12:59:29+05:30  Cprogrammer
 * use MARIADB_BASE_VERSION additionally to LIBMARIADB
 *
 * Revision 2.19  2018-03-30 09:32:26+05:30  Cprogrammer
 * mysql_options() in libmariadb is unable to locate cnf files without an absolute path
 *
 * Revision 2.18  2018-03-27 12:14:12+05:30  Cprogrammer
 * call mysql_ssl_set() if flags is set
 *
 * Revision 2.17  2018-03-26 10:41:47+05:30  Cprogrammer
 * renamed LIBMARIADB to HAVE_LIBMARIADBCLIENT
 *
 * Revision 2.16  2018-03-24 22:25:43+05:30  Cprogrammer
 * use mysql_ssl_set() for mariadb ssl connection issue
 *
 * Revision 2.15  2018-03-24 17:09:54+05:30  Cprogrammer
 * shortened env variable MYSQL_OPT_SSL_VERIFY_SERVER_CERT to MYSQL_OPT_SSL_VERIFY_CERT
 *
 * Revision 2.14  2018-03-21 11:11:05+05:30  Cprogrammer
 * added error_mysql_options_str() function to display the exact mysql_option() error
 *
 * Revision 2.13  2018-03-21 08:08:50+05:30  Cprogrammer
 * use conditional defines from configure.ac to compile in mysql options
 *
 * Revision 2.12  2018-03-20 12:16:13+05:30  Cprogrammer
 * added #ifdef statements for conditional compilation of setting SSL/TLS options
 *
 * Revision 2.11  2018-03-19 23:08:51+05:30  Cprogrammer
 * added ability to set SSLTLS options
 *
 * Revision 2.10  2018-02-18 16:08:51+05:30  Cprogrammer
 * use mysql_optionsv() with mariadb
 *
 * Revision 2.9  2014-01-02 23:54:03+05:30  Cprogrammer
 * set delayed_insert variable for MYSQL delayed insert
 *
 * Revision 2.8  2010-04-15 14:14:25+05:30  Cprogrammer
 * set client flags for mysql_real_connect()
 *
 * Revision 2.7  2010-04-15 12:47:36+05:30  Cprogrammer
 * corrected data type of 3rd argument of mysql_options()
 *
 * Revision 2.6  2009-11-09 10:43:01+05:30  Cprogrammer
 * added comment for MYSQL_OPT_PROTOCOL enum values
 *
 * Revision 2.5  2009-11-09 08:34:36+05:30  Cprogrammer
 * added option to set MYSQL_OPT_PROTOCOL
 *
 * Revision 2.4  2009-03-16 10:36:42+05:30  Cprogrammer
 * added MYSQL_SET_CLIENT_IP, MYSQL_OPT_RECONNECT
 *
 * Revision 2.3  2009-03-16 09:38:40+05:30  Cprogrammer
 * added MYSQL_OPT_READ_TIMEOUT, MYSQL_OPT_WRITE_TIMEOUT
 *
 * Revision 2.2  2008-05-28 17:46:24+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2006-03-17 14:43:49+05:30  Cprogrammer
 * Initial Version
 *
 */
#include <stdlib.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: set_mysql_options.c,v 2.20 2018-04-01 12:59:29+05:30 Cprogrammer Exp mbhangui $";
#endif

#define max_mysql_option_err_num 21
static char *mysql_option_err[] = {
	"No Error",
	"MYSQL_INIT_COMMAND",
	"MYSQL_READ_DEFAULT_FILE",
	"MYSQL_READ_DEFAULT_GROUP",
	"MYSQL_OPT_CONNECT_TIMEOUT",
	"MYSQL_OPT_READ_TIMEOUT",
	"MYSQL_OPT_WRITE_TIMEOUT",
	"MYSQL_SET_CLIENT_IP",
	"MYSQL_OPT_RECONNECT",
	"MYSQL_OPT_PROTOCOL",
	"MYSQL_OPT_SSL_CA",
	"MYSQL_OPT_SSL_CAPATH",
	"MYSQL_OPT_SSL_CERT",
	"MYSQL_OPT_SSL_CIPHER",
	"MYSQL_OPT_SSL_CRL",
	"MYSQL_OPT_SSL_CRLPATH",
	"MYSQL_OPT_SSL_ENFORCE",
	"MYSQL_OPT_SSL_VERIFY_SERVER_CERT",
	"MYSQL_OPT_SSL_MODE",
	"MYSQL_OPT_SSL_KEY",
	"MYSQL_OPT_TLS_VERSION",
	0
};

int
int_mysql_options(MYSQL *mysql, enum mysql_option option, const void *arg)
{
#ifdef HAVE_MYSQL_OPTIONSV
	return (mysql_options(mysql, option, arg)); /*- at the moment mysql_optionsv() does a core dump */
#else
	return (mysql_options(mysql, option, arg));
#endif
}

char *
error_mysql_options_str(unsigned int errnum)
{
	return ((errnum > max_mysql_option_err_num) ? 0 : mysql_option_err[errnum]);
}

int
set_mysql_options(MYSQL *mysql, char *file, char *group, unsigned int *flags)
{
	char           *default_file, *default_group, *c_timeout, 
				   *r_timeout, *w_timeout, *init_cmd, *ptr,
				   *set_client_ip, *opt_reconnect, *opt_protocol;
	char            temp[4];
	char            o_reconnect, tmpv_c, use_ssl = 0;
	unsigned int    protocol, connect_timeout, read_timeout, write_timeout;
#if defined(LIBMARIADB) || defined(MARIADB_BASE_VERSION)
	char            fpath[MAX_BUFF];
	char           *sysconfdir;
#endif
#ifdef HAVE_MYSQL_OPT_SSL_MODE
	unsigned int    ssl_mode;
#endif
	char           *cipher;

	use_ssl = (*flags == 1 ? 1 : 0);
	*flags = 0;
	if (getenv("CLIENT_COMPRESS"))
		*flags += CLIENT_COMPRESS;
	if (getenv("CLIENT_INTERACTIVE"))
		*flags += CLIENT_INTERACTIVE;
	getEnvConfigStr(&init_cmd, "MYSQL_INIT_COMMAND", 0);
#if defined(LIBMARIADB) || defined(MARIADB_BASE_VERSION)
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	snprintf(fpath, sizeof(fpath) - 1, "%s/%s", sysconfdir, file);
	getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", fpath);
#else
	getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", file);
#endif
	getEnvConfigStr(&default_group, "MYSQL_READ_DEFAULT_GROUP", group);
	getEnvConfigStr(&c_timeout, "MYSQL_OPT_CONNECT_TIMEOUT", "120");
	getEnvConfigStr(&r_timeout, "MYSQL_OPT_READ_TIMEOUT", "20");
	getEnvConfigStr(&w_timeout, "MYSQL_OPT_WRITE_TIMEOUT", "20");
	getEnvConfigStr(&set_client_ip, "MYSQL_SET_CLIENT_IP", 0);
	getEnvConfigStr(&opt_reconnect, "MYSQL_OPT_RECONNECT", "0");
	snprintf(temp, sizeof(temp) - 1, "%d", MYSQL_PROTOCOL_DEFAULT);
	getEnvConfigStr(&opt_protocol, "MYSQL_OPT_PROTOCOL", temp);
	protocol = atoi(opt_protocol);
	o_reconnect = atoi(opt_reconnect);
	connect_timeout = atoi(c_timeout);
	read_timeout = atoi(r_timeout);
	write_timeout = atoi(w_timeout);
	if (init_cmd && int_mysql_options(mysql, MYSQL_INIT_COMMAND, init_cmd))
		return (1);
	if (int_mysql_options(mysql, MYSQL_READ_DEFAULT_FILE, default_file))
		return (2);
	if (int_mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, default_group))
		return (3);
	if (int_mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &connect_timeout))
		return (4);
	if (int_mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, (char *) &read_timeout))
		return (5);
	if (int_mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, (char *) &write_timeout))
		return (6);
	if (getenv("MYSQL_SET_CLIENT_IP") && 
			int_mysql_options(mysql, MYSQL_SET_CLIENT_IP, set_client_ip))
		return (7);
	if (getenv("MYSQL_OPT_RECONNECT") &&
			int_mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *) &o_reconnect))
		return (8);
	/*-
	 * enum mysql_protocol_type 
	 * MYSQL_PROTOCOL_DEFAULT, MYSQL_PROTOCOL_TCP, MYSQL_PROTOCOL_SOCKET,
	 * MYSQL_PROTOCOL_PIPE, MYSQL_PROTOCOL_MEMORY
	 */
	if (int_mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *) &protocol))
		return (9);

	/*- SSL options */
	if (use_ssl) {
		getEnvConfigStr(&cipher, "CIPHER", 0); /*- DHE-RSA-AES256-SHA */
		mysql_ssl_set(mysql, 0, 0, 0, 0, cipher); /*- this always returns 0 */
	}

#ifdef HAVE_MYSQL_OPT_SSL_CA
	/*-
	 * MYSQL_OPT_SSL_CA - The path name of the Certificate Authority (CA) certificate file.
	 * This option, if used, must specify the same certificate used by the server.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CA")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CA, ptr))
		return (10);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CAPATH
	/*-
	 * MYSQL_OPT_SSL_CAPATH
	 * The path name of the directory that contains trusted SSL CA certificate files.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CAPATH")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, ptr))
		return (11);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CERT
	/*-
	 * MYSQL_OPT_SSL_CERT
	 * The path name of the client public key certificate file.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CERT")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CERT, ptr))
		return (12);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CIPHER
	/*-
	 * MYSQL_OPT_SSL_CIPHER
	 * The list of permitted ciphers for SSL encryption.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CIPHER")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, ptr))
		return (13);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CRL
	/*
	 * MYSQL_OPT_SSL_CRL (argument type: char *)
	 * The path name of the file containing certificate revocation lists.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CRL")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CRL, ptr))
		return (14);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CRLPATH
	/*-
	 * MYSQL_OPT_SSL_CRLPATH (argument type: char *)
	 * The path name of the directory that contains files containing certificate revocation lists.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_CRLPATH")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CRLPATH, ptr))
		return (15);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_ENFORCE
	/*-
	 * MYSQL_OPT_SSL_ENFORCE (argument type: my_bool *)
	 *
	 * Whether to require the connection to use SSL. If enabled and an encrypted
	 * connection cannot be established, the connection attempt fails.
	 * This option is deprecated as of MySQL 5.7.11 and is removed in MySQL 8.0.
	 * Instead, use MYSQL_OPT_SSL_MODE with a value of SSL_MODE_REQUIRED.
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_ENFORCE")) && int_mysql_options(mysql, MYSQL_OPT_SSL_ENFORCE, ptr))
		return (16);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT
	/*-
	 * MYSQL_OPT_SSL_VERIFY_SERVER_CERT (argument type: my_bool *)
	 * Enable or disable verification of the server's Common Name identity in its certificate
	 * against the host name used when connecting to the server. The connection is rejected
	 * if there is a mismatch. For encrypted connections, this feature can be used to prevent
	 * man-in-the-middle attacks. Identity verification is disabled by default.
	 * This option does not work with self-signed certificates, which do not contain the
	 * server name as the Common Name value.
	 *
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_VERIFY_CERT"))) {
		tmpv_c = atoi(ptr) ? 1 : 0;
		if (int_mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &tmpv_c))
			return (17);
	}
#endif
#ifdef HAVE_MYSQL_OPT_SSL_MODE
	/*-
	 * MYSQL_OPT_SSL_MODE (argument type: unsigned int *)
	 *
	 * The security state to use for the connection to the server: 
	 * SSL_MODE_DISABLED, SSL_MODE_PREFERRED, SSL_MODE_REQUIRED, SSL_MODE_VERIFY_CA,
	 * SSL_MODE_VERIFY_IDENTITY.
	 * The default is SSL_MODE_PREFERRED. These modes are the permitted values of
	 * the mysql_ssl_mode enumeration defined in mysql.h.
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_MODE"))) {
		ssl_mode = atoi(ptr);
		if (int_mysql_options(mysql, MYSQL_OPT_SSL_MODE, &ssl_mode))
			return (18);
	}
#endif
#ifdef HAVE_MYSQL_OPT_SSL_KEY
	/*-
	 * MYSQL_OPT_SSL_KEY (argument type: char *)
	 * The path name of the client private key file.
	 * community/mariadb
	 */
	if ((ptr = getenv("MYSQL_OPT_SSL_KEY")) && int_mysql_options(mysql, MYSQL_OPT_SSL_KEY, ptr))
		return (19);
#endif
#ifdef HAVE_MYSQL_OPT_TLS_VERSION
	/*-
	 * MYSQL_OPT_TLS_VERSION (argument type: char *)
	 * The protocols permitted by the client for encrypted connections.
	 * The value is a comma-separated list containing one or more protocol names.
	 * The protocols that can be named for this option depend on the SSL library
	 * used to compile MySQL. 
	 *
	 * When compiled using OpenSSL 1.0.1 or higher, MySQL supports the TLSv1, TLSv1.1, and TLSv1.2 protocols.
	 * When compiled using the bundled version of yaSSL, MySQL supports the TLSv1 and TLSv1.1 protocols.
	 */
	if ((ptr = getenv("MYSQL_OPT_TLS_VERSION")) && int_mysql_options(mysql, MYSQL_OPT_TLS_VERSION, ptr))
		return (20);
#endif
	return (0);
}

void
getversion_set_mysql_options_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
