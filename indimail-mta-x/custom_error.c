/*
 * $Log: custom_error.c,v $
 * Revision 1.2  2022-03-27 20:06:52+05:30  Cprogrammer
 * include extended error from errno if EXTENDED_ERROR env variable is defined
 *
 * Revision 1.1  2022-03-08 22:56:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <env.h>
#include <scan.h>
#include <noreturn.h>
#include <stralloc.h>
#include <error.h>
#include "custom_error.h"
#include "qmail.h"

static stralloc e = {0};

no_return void
custom_error(char *program, char *type, char *message, char *extra, char *code)
{
	char           *c;
	static char     flag;
	static char     errbuf[256];
	static int      errfd = -1;
	static struct substdio sserr;

	if (errfd == -1) {
		if (!(c = env_get("ERROR_FD")))
			errfd = CUSTOM_ERR_FD;
		else
			scan_int(c, &errfd);
	}
	if (!flag)
		substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	if (substdio_put(&sserr, type, 1) == -1 ||
			substdio_puts(&sserr, program) == -1 ||
			substdio_put(&sserr, ": ", 2) ||
			substdio_puts(&sserr, message) == -1)
		_exit(53);
	if (extra && (substdio_put(&sserr, ": ", 2) == -1 || substdio_puts(&sserr, extra) == -1))
		_exit(53);
	if (code) {
		if (substdio_put(&sserr, " (#", 3) == -1)
			_exit(53);
		c = (*type == 'D') ? "5" : "4";
		if (substdio_put(&sserr, c, 1) == -1 ||
				substdio_put(&sserr, code + 1, 4) == -1 ||
				substdio_put(&sserr, ")", 1) == -1)
			_exit(53);
	}
	if (substdio_flush(&sserr) == -1)
		_exit(53);
	_exit(88);
}

char           *
extended_err(char *str1, char *str2)
{
	if (!stralloc_copys(&e, str1))
		return "Zqq out of memory (#4.3.0)";
	if (str2) {
		if (!stralloc_catb(&e, " (#", 3) ||
				!stralloc_cats(&e, str2) ||
				!stralloc_catb(&e, ")", 2))
			return "Zqq out of memory (#4.3.0)";
	}
	if (!env_get("EXTENDED_ERROR")) {
		if (!stralloc_0(&e))
			return "Zqq out of memory (#4.3.0)";
		return e.s;
	}
	if (!stralloc_catb(&e, ": ", 2) ||
			!stralloc_cats(&e, error_str(errno)) ||
			!stralloc_0(&e))
		return "Zqq out of memory (#4.3.0)";
	return e.s;
}
