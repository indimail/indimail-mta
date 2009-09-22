#include "exit.h"
#include "scan.h"
#include "str.h"
#include "subfd.h"
#include "strerr.h"
#include "cdb.h"

#define FATAL "cdbget: fatal: "

void
die_read(void)
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

void
die_usage(void)
{
	strerr_die1x(111, "cdbget: usage: cdbget key [skip]");
}

static struct cdb c;
char            buf[1024];

int
main(int argc, char **argv)
{
	char           *key;
	int             r;
	uint32          pos;
	uint32          len;
	unsigned long   u = 0;

	if (!*argv)
		die_usage();

	if (!*++argv)
		die_usage();
	key = *argv;

	if (*++argv) {
		scan_ulong(*argv, &u);
	}

	_cdb_init(&c, 0);
	cdb_findstart(&c);

	for (;;) {
		r = cdb_findnext(&c, key, str_len(key));
		if (r == -1)
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
