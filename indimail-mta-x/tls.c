/*
 * $Log: tls.c,v $
 * Revision 1.8  2021-05-26 10:47:57+05:30  Cprogrammer
 * replaced strerror() with error_str()
 *
 * Revision 1.7  2018-06-11 23:28:54+05:30  Cprogrammer
 * removed ssl_free()
 *
 * Revision 1.6  2017-05-02 16:40:18+05:30  Cprogrammer
 * avoid potential SIGSEGV
 *
 * Revision 1.5  2008-06-01 15:33:39+05:30  Cprogrammer
 * new function ssl_error_str() which does not use the strerror() function
 *
 * Revision 1.4  2007-12-20 13:53:23+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.3  2004-10-22 20:31:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-30 18:23:49+05:30  Cprogrammer
 * fixed compilation warning for exit
 *
 * Revision 1.1  2004-07-30 17:35:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef TLS
#include "error.h"
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int             smtps = 0;
SSL            *ssl = NULL;

void
ssl_exit(int status)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0)
			usleep(100);
		SSL_free(ssl);
		ssl = 0;
	}
	_exit(status);
}

const char     *
ssl_error()
{
	int             r = ERR_get_error();
	if (!r)
		return "unknown error";
	SSL_load_error_strings();
	return ERR_error_string(r, NULL);
}

const char     *
ssl_error_str()
{
	const char     *err = ssl_error();

	if (err)
		return err;
	if (!errno)
		return "unknown error";
	return (errno == error_timeout) ? "timed out" : error_str(errno);
}

const char     *
ssl_strerror()
{
	const char     *err = ssl_error();
	if (err)
		return err;
	if (!errno)
		return 0;
	return errno == error_timeout ? "timed out" : error_str(errno);
}
#endif

void
getversion_tls_c()
{
	static char    *x = "$Id: tls.c,v 1.8 2021-05-26 10:47:57+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
