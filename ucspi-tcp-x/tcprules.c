/*
 * $Log: tcprules.c,v $
 * Revision 1.8  2021-08-30 12:47:59+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.7  2020-09-16 20:50:26+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.6  2020-08-03 17:28:09+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.5  2009-11-12 15:04:58+05:30  Cprogrammer
 * fixed tcprules going into endless loop with lines not having allow and deny
 *
 * Revision 1.4  2009-05-29 15:03:12+05:30  Cprogrammer
 * added code to unset env variables
 *
 * Revision 1.3  2008-07-25 16:50:19+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.2  2005-06-10 09:13:51+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#include <byte.h>
#include <cdb_make.h>
#include <open.h>
#include <scan.h>
#include <unistd.h>
#include <noreturn.h>

#define FATAL "tcprules: fatal: "

static struct cdb_make c;
static char    *fntemp;
static stralloc line = { 0 };
static stralloc address = { 0 };
static stralloc data = { 0 };
static stralloc key = { 0 };
static stralloc sanum = { 0 };

#if defined(linux) || defined(DARWIN) || defined(FREEBSD)
extern int      rename(const char *, const char *);
#endif

no_return void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
usage(void)
{
	strerr_die1x(100, "tcprules: usage: tcprules rules.cdb rules.tmp");
}

no_return void
die_bad(void)
{
	if (!stralloc_0(&line))
		nomem();
	strerr_die3x(100, FATAL, "unable to parse this line: ", line.s);
}

no_return void
die_write(void)
{
	strerr_die4sys(111, FATAL, "unable to write to ", fntemp, ": ");
}

void
getnum(char *buf, int len, unsigned long *u)
{
	if (!stralloc_copyb(&sanum, buf, len) ||
			!stralloc_0(&sanum))
		nomem();
	if (sanum.s[scan_ulong(sanum.s, u)])
		die_bad();
}

void
doaddressdata(void)
{
	int             i;
	int             left;
	int             right;
	unsigned long   bot;
	char            strnum[FMT_ULONG];
	unsigned long   top;

	if (byte_chr(address.s, address.len, '=') == address.len) {
		if (byte_chr(address.s, address.len, '@') == address.len) {
			if ((i = byte_chr(address.s, address.len, '-')) < address.len) {
				left = byte_rchr(address.s, i, '.');
				if (left == i)
					left = 0;
				else
					++left;
				++i;
				right = i + byte_chr(address.s + i, address.len - i, '.');
				getnum(address.s + left, i - 1 - left, &bot);
				getnum(address.s + i, right - i, &top);
				if (top > 255)
					top = 255;
				while (bot <= top) {
					if (!stralloc_copyb(&key, address.s, left) ||
							!stralloc_catb(&key, strnum, fmt_ulong(strnum, bot)) ||
							!stralloc_catb(&key, address.s + right, address.len - right))
						nomem();
					if (cdb_make_add(&c, key.s, key.len, data.s, data.len) == -1)
						die_write();
					++bot;
				}
				return;
			}
		}
	}
	if (cdb_make_add(&c, address.s, address.len, data.s, data.len) == -1)
		die_write();
}

int
main(int argc, char **argv)
{
	char           *x, *fn;
	char            ch;
	int             len, fd, i, e, colon, match = 1;

	if (!(fn = argv[1]))
		usage();
	if (!(fntemp = argv[2]))
		usage();
	if ((fd = open_trunc(fntemp)) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", fntemp, ": ");
	if (cdb_make_start(&c, fd) == -1)
		die_write();
	while (match) {
		if (getln(subfdin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		x = line.s;
		len = line.len;
		if (!len)
			break;
		if (x[0] == '#')
			continue;
		if (x[0] == '\n')
			continue;
		while (len) {
			ch = x[len - 1];
			if (ch != '\n' && ch != ' ' && ch != '\t')
				break;
			--len;
		}
		line.len = len;			/*- for die_bad() */
#ifdef IPV6
		for (colon = 0;;) {
			int             tmp;

			tmp = byte_chr(x + colon, len - colon, ':');
			if (colon == 0 && tmp == len)
				strerr_die2x(111, FATAL, "Unable to find colon on non-empty line.");
			colon += tmp;
			if (colon == len)
				break;
			if (byte_equal(x + colon + 1, 4, "deny") || byte_equal(x + colon + 1, 5, "allow"))
				break;
			++colon;
		}
#else
		colon = byte_chr(x, len, ':');
#endif
		if (colon == len)
			continue;
		if (!stralloc_copyb(&address, x, colon) ||
				!stralloc_copys(&data, ""))
			nomem();
		x += colon + 1;
		len -= colon + 1;
		if ((len >= 4) && byte_equal(x, 4, "deny")) {
			if (!stralloc_catb(&data, "D", 2))
				nomem();
			x += 4;
			len -= 4;
		} else
		if ((len >= 5) && byte_equal(x, 5, "allow")) {
			x += 5;
			len -= 5;
		} else
			die_bad();
		while (len) {
			switch (*x)
			{
			case ',':
				e = byte_chr(x + 1, len - 1, ',');
				i = byte_chr(x, len, '=');
				if (i > e) {
					if (e < 2 || x[1] != '!')
						die_bad();
					if (!stralloc_catb(&data, "-", 1) ||
							!stralloc_catb(&data, x + 2, e - 1) ||
							!stralloc_0(&data))
						nomem();
					x += e + 1;
					len -= e + 1;
					break;
				}
				if (!stralloc_catb(&data, "+", 1) ||
						!stralloc_catb(&data, x + 1, i))
					nomem();
				x += i + 1;
				len -= i + 1;
				if (!len)
					die_bad();
				ch = *x;
				x += 1;
				len -= 1;
				i = byte_chr(x, len, ch);
				if (i == len)
					die_bad();
				if (!stralloc_catb(&data, x, i) ||
						!stralloc_0(&data))
					nomem();
				x += i + 1;
				len -= i + 1;
				break;
			default:
				die_bad();
			}
		}
		doaddressdata();
	}
	if (cdb_make_finish(&c) == -1)
		die_write();
	if (fsync(fd) == -1)
		die_write();
	if (close(fd) == -1)
		die_write();			/*- NFS stupidity */
	if (rename(fntemp, fn))
		strerr_die6sys(111, FATAL, "unable to move ", fntemp, " to ", fn, ": ");
	_exit(0);
}
