/*
 * $Log: cdbtest.c,v $
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:44:19+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2019-07-18 10:46:50+05:30  Cprogrammer
 * use cdb_init() from libqmail
 *
 * Revision 1.1  2008-09-16 08:14:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <uint32.h>
#include <str.h>
#include <fmt.h>
#include <subfd.h>
#include <strerr.h>
#include <seek.h>
#include <cdb.h>
#include <noreturn.h>

#define FATAL "cdbtest: fatal: "

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

void
put(const char *buf, unsigned int len)
{
	if (substdio_put(subfdoutsmall, buf, len) == -1)
		die_write();
}

void
putflush(void)
{
	if (substdio_flush(subfdoutsmall) == -1)
		die_write();
}

uint32          pos = 0;

void
get(char *buf, unsigned int len)
{
	int             r;
	while (len > 0) {
		r = substdio_get(subfdin, buf, len);
		if (r == -1)
			die_read();
		if (r == 0)
			strerr_die2x(111, FATAL, "unable to read input: truncated file");
		pos += r;
		buf += r;
		len -= r;
	}
}

void
getnum(uint32 *num)
{
	char            buf[4];
	get(buf, 4);
	uint32_unpack(buf, num);
}

char            strnum[FMT_ULONG];

void
putnum(const char *label, unsigned long count)
{
	put(label, str_len(label));
	put(strnum, fmt_ulong(strnum, count));
	put("\n", 1);
}

char            key[1024];

unsigned long   numuntested = 0;
unsigned long   numnotfound = 0;
unsigned long   numotherpos = 0;
unsigned long   numbadlen = 0;
unsigned long   numfound = 0;

static struct cdb c;

int
main()
{
	uint32          eod;
	uint32          klen;
	uint32          dlen;
	seek_pos        rest;

	cdb_init(&c, 0);

	getnum(&eod);
	while (pos < 2048)
		getnum(&dlen);

	while (pos < eod) {
		getnum(&klen);
		getnum(&dlen);
		if (klen > sizeof key) {
			++numuntested;
			while (klen) {
				get(key, 1);
				--klen;
			}
		} else {
			get(key, klen);
			rest = seek_cur(0);
			switch (cdb_find(&c, key, klen)) {
			case -1:
				die_read();
			case 0:
				++numnotfound;
				break;
			default:
				if (cdb_datapos(&c) != pos)
					++numotherpos;
				else if (cdb_datalen(&c) != dlen)
					++numbadlen;
				else
					++numfound;
			}
			if (seek_set(0, rest) == -1)
				die_read();
		}
		while (dlen) {
			get(key, 1);
			--dlen;
		}
	}

	putnum("found: ", numfound);
	putnum("different record: ", numotherpos);
	putnum("bad length: ", numbadlen);
	putnum("not found: ", numnotfound);
	putnum("untested: ", numuntested);
	putflush();
	_exit(0);
}

void
getversion_cdbtest_c()
{
	const char     *x = "$Id: cdbtest.c,v 1.5 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
