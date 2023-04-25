/*
 * $Log: custom_error.c,v $
 * Revision 1.2  2023-04-25 22:41:34+05:30  Cprogrammer
 * removed use of static variables as function is noreturn
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
#include "custom_error.h"
#include "qmail.h"

no_return void
custom_error(char *program, char *type, char *message, char *extra, char *code)
{
	char           *c;
	char            errbuf[256];
	int             errfd;
	struct substdio sserr;

	if (!(c = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(c, &errfd);
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

