/*
 * $Log: tcprulescheck.c,v $
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
#include "byte.h"
#include "buffer.h"
#include "strerr.h"
#include "env.h"
#include "rules.h"
#include "open.h"
#include <unistd.h>

void
found(char *data, unsigned int datalen)
{
	unsigned int    next0;

	buffer_puts(buffer_1, "rule ");
	buffer_put(buffer_1, rules_name.s, rules_name.len);
	buffer_puts(buffer_1, ":\n");
	while ((next0 = byte_chr(data, datalen, 0)) < datalen)
	{
		switch (data[0])
		{
		case 'D':
			buffer_puts(buffer_1, "deny connection\n");
			buffer_flush(buffer_1);
			_exit(0);
		case '+':
			buffer_puts(buffer_1, "set environment variable ");
			buffer_puts(buffer_1, data + 1);
			buffer_puts(buffer_1, "\n");
			break;
		case '-':
			buffer_puts(buffer_1, "unset environment variable ");
			buffer_puts(buffer_1, data + 1);
			buffer_puts(buffer_1, "\n");
			break;
		}
		++next0;
		data += next0;
		datalen -= next0;
	}
	buffer_puts(buffer_1, "allow connection\n");
	buffer_flush(buffer_1);
	_exit(0);
}

int
main(int argc, char **argv)
{
	char           *fnrules;
	int             fd;
	char           *ip;
	char           *info;
	char           *host;

	fnrules = argv[1];
	if (!fnrules)
		strerr_die1x(100, "tcprulescheck: usage: tcprulescheck rules.cdb");

	ip = env_get("TCPREMOTEIP");
	if (!ip)
		ip = "0.0.0.0";
	info = env_get("TCPREMOTEINFO");
	host = env_get("TCPREMOTEHOST");

	fd = open_read(fnrules);
	if ((fd == -1) || (rules(found, fd, ip, host, info) == -1))
		strerr_die3sys(111, "tcprulescheck: fatal: unable to read ", fnrules, ": ");

	buffer_putsflush(buffer_1, "default:\nallow connection\n");
	_exit(0);
}
