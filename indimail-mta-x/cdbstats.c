/*
 * $Log: cdbstats.c,v $
 * Revision 1.3  2020-11-24 13:44:16+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2019-07-18 10:46:41+05:30  Cprogrammer
 * use cdb_init() from libqmail
 *
 * Revision 1.1  2008-09-16 08:14:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "uint32.h"
#include "fmt.h"
#include "str.h"
#include "subfd.h"
#include "strerr.h"
#include "seek.h"
#include "cdb.h"

#define FATAL "cdbstats: fatal: "

void
die_read(void)
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

void
die_readformat(void)
{
	strerr_die2x(111, FATAL, "unable to read input: truncated file");
}

void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

void
put(char *buf, unsigned int len)
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
			die_readformat();
		pos += r;
		buf += r;
		len -= r;
	}
}

void
getnum(uint32 * num)
{
	char            buf[4];
	get(buf, 4);
	uint32_unpack(buf, num);
}

char            strnum[FMT_ULONG];

void
putnum(char *label, unsigned long count)
{
	unsigned int    i;
	put(label, str_len(label));
	for (i = fmt_ulong(0, count); i < 10; ++i)
		put(" ", 1);
	put(strnum, fmt_ulong(strnum, count));
	put("\n", 1);
}

char            key[1024];

static struct cdb c;

static unsigned long numrecords;
static unsigned long numd[11];

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
			while (klen) {
				get(key, 1);
				--klen;
			}
		} else {
			get(key, klen);
			rest = seek_cur(0);
			cdb_findstart(&c);
			do {
				switch (cdb_findnext(&c, key, klen)) {
				case -1:
					die_read();
				case 0:
					die_readformat();
				}
			} while (cdb_datapos(&c) != pos);
			if (!c.loop)
				die_readformat();
			++numrecords;
			if (c.loop > 10)
				++numd[10];
			else
				++numd[c.loop - 1];
			if (seek_set(0, rest) == -1)
				die_read();
		}
		while (dlen) {
			get(key, 1);
			--dlen;
		}
	}
	putnum("records ", numrecords);
	putnum("d0      ", numd[0]);
	putnum("d1      ", numd[1]);
	putnum("d2      ", numd[2]);
	putnum("d3      ", numd[3]);
	putnum("d4      ", numd[4]);
	putnum("d5      ", numd[5]);
	putnum("d6      ", numd[6]);
	putnum("d7      ", numd[7]);
	putnum("d8      ", numd[8]);
	putnum("d9      ", numd[9]);
	putnum(">9      ", numd[10]);
	putflush();
	_exit(0);
}

void
getversion_cdbstats_c()
{
	static char    *x = "$Id: cdbstats.c,v 1.3 2020-11-24 13:44:16+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
