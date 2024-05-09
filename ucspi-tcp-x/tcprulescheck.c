/*
 * $Log: tcprulescheck.c,v $
 * Revision 1.5  2021-08-30 12:47:59+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.4  2020-08-03 17:27:59+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.3  2009-05-29 15:02:57+05:30  Cprogrammer
 * unset env variables
 *
 * Revision 1.2  2003-12-30 00:33:24+05:30  Cprogrammer
 * added standard header files
 *
 * Revision 1.1  2003-10-21 11:18:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <byte.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <env.h>
#include <open.h>
#include <unistd.h>
#include <noreturn.h>
#include "rules.h"

no_return void
found(char *data, unsigned int datalen)
{
	unsigned int    next0;

	substdio_puts(subfdout, "rule ");
	substdio_put(subfdout, rules_name.s, rules_name.len);
	substdio_puts(subfdout, ":\n");
	while ((next0 = byte_chr(data, datalen, 0)) < datalen) {
		switch (data[0])
		{
		case 'D':
			substdio_puts(subfdout, "deny connection\n");
			substdio_flush(subfdout);
			_exit(0);
		case '+':
			substdio_puts(subfdout, "set environment variable ");
			substdio_puts(subfdout, data + 1);
			substdio_puts(subfdout, "\n");
			break;
		case '-':
			substdio_puts(subfdout, "unset environment variable ");
			substdio_puts(subfdout, data + 1);
			substdio_puts(subfdout, "\n");
			break;
		}
		++next0;
		data += next0;
		datalen -= next0;
	}
	substdio_puts(subfdout, "allow connection\n");
	substdio_flush(subfdout);
	_exit(0);
}

int
main(int argc, char **argv)
{
	char           *fnrules;
	int             fd;
	char           *ip;
	const char     *info, *host;

	fnrules = argv[1];
	if (!fnrules)
		strerr_die1x(100, "tcprulescheck: usage: tcprulescheck rules.cdb");

	if (!(ip = env_get("TCPREMOTEIP")))
		ip = (char *) "0.0.0.0";
	info = env_get("TCPREMOTEINFO");
	host = env_get("TCPREMOTEHOST");

	fd = open_read(fnrules);
	if ((fd == -1) || (rules(found, fd, ip, host, info) == -1))
		strerr_die3sys(111, "tcprulescheck: fatal: unable to read ", fnrules, ": ");

	substdio_putsflush(subfdout, "default:\nallow connection\n");
	_exit(0);
}
