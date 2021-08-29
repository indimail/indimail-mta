/*
 * $Log: cdbdump.c,v $
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.2  2020-11-24 13:44:11+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2008-09-16 08:14:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <uint32.h>
#include <fmt.h>
#include <subfd.h>
#include <strerr.h>
#include <noreturn.h>

#define FATAL "cdbdump: fatal: "

no_return void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

void
put(char *buf, unsigned int len)
{
	if (substdio_put(subfdout, buf, len) == -1)
		die_write();
}

void
putflush(void)
{
	if (substdio_flush(subfdout) == -1)
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
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (r == 0)
			strerr_die2x(111, FATAL, "unable to read input: truncated file");
		pos += r;
		buf += r;
		len -= r;
	}
}

char            buf[512];

void
copy(uint32 len)
{
	unsigned int    x;

	while (len) {
		x = sizeof buf;
		if (len < x)
			x = len;
		get(buf, x);
		put(buf, x);
		len -= x;
	}
}

void
getnum(uint32 * num)
{
	get(buf, 4);
	uint32_unpack(buf, num);
}

char            strnum[FMT_ULONG];

int
main()
{
	uint32          eod;
	uint32          klen;
	uint32          dlen;

	getnum(&eod);
	while (pos < 2048)
		getnum(&dlen);

	while (pos < eod) {
		getnum(&klen);
		getnum(&dlen);
		put("+", 1);
		put(strnum, fmt_ulong(strnum, klen));
		put(",", 1);
		put(strnum, fmt_ulong(strnum, dlen));
		put(":", 1);
		copy(klen);
		put("->", 2);
		copy(dlen);
		put("\n", 1);
	}

	put("\n", 1);
	putflush();
	_exit(0);
}

void
getversion_cdbdump_c()
{
	static char    *x = "$Id: cdbdump.c,v 1.3 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
