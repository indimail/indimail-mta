/*
 * $Log: cdbget.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-02-12 13:29:59+05:30  Cprogrammer
 * added -n option for searching cdb created by cdb-database
 *
 * Revision 1.5  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.4  2020-11-24 13:44:14+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.3  2019-07-18 10:46:16+05:30  Cprogrammer
 * use cdb_init() from libqmail
 *
 * Revision 1.2  2013-09-04 12:51:01+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2013-09-03 22:54:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sgetopt.h>
#include <scan.h>
#include <str.h>
#include <subfd.h>
#include <strerr.h>
#include <cdb.h>
#include <stralloc.h>
#include <noreturn.h>

#define FATAL "cdbget: fatal: "

no_return void
die_read(void)
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

no_return void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

no_return void
die_usage(void)
{
	strerr_die1x(111, "cdbget: usage: cdbget [-n] key [skip]");
}


int
main(int argc, char **argv)
{
	char           *key;
	int             r, i, put_null = 0;
	uint32          pos, len;
	unsigned long   u = 0;
	char            buf[1024];
	struct        cdb c;
	static stralloc _key = { 0 };

	while ((i = getopt(argc, argv, "n")) != opteof) {
		switch (i)
		{
		case 'n':
			put_null = 1;
			break;
		default:
			die_usage();
		}
	}
	argv += optind;
	if (!*argv)
		die_usage();
	key = *argv;

	if (*++argv)
		scan_ulong(*argv, &u);

	cdb_init(&c, 0);
	cdb_findstart(&c);

	if (!stralloc_copys(&_key, key) || !stralloc_0(&_key))
		_exit(111);
	if (!put_null)
		_key.len--;
	for (;;) {
		if ((r = cdb_findnext(&c, _key.s, _key.len)) == -1)
			die_read();
		if (!r)
			_exit(100);
		if (!u)
			break;
		--u;
	}

	pos = cdb_datapos(&c);
	len = cdb_datalen(&c);

	while (len > 0) {
		r = sizeof buf;
		if (r > len)
			r = len;
		if (cdb_read(&c, buf, r, pos) == -1)
			die_read();
		if (substdio_put(subfdoutsmall, buf, r) == -1)
			die_write();
		pos += r;
		len -= r;
	}
	if (substdio_flush(subfdoutsmall) == -1)
		die_write();
	_exit(0);
}

void
getversion_cdbget_c()
{
	const char     *x = "$Id: cdbget.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
	if (x)
		x++;
}
