/*-
 * tai2tai64n -- Convert older TAI format timestamps to TAI64N
 * Copyright (C) 2000 Bruce Guenter <bruceg@em.ca>
 *
 * $Log: tai2tai64n.c,v $
 * Revision 1.2  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.1  2016-01-02 19:21:03+05:30  Cprogrammer
 * Initial revision
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <unistd.h>
#include <tai2.h>
#include <substdio.h>
#include <getln.h>
#include <stralloc.h>
#include <error.h>
#include <noreturn.h>

#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define USAGE_ERR 4

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof (sserrbuf));

void
logerr(const char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(const char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

no_return void
my_error(const char *s1, const char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

void
my_puts(const char *s, int len)
{
	if (len) {
		if (substdio_put(&ssout, s, len) == -1)
			my_error("write", 0, WRITE_ERR);
	} else {
		if (substdio_puts(&ssout, s) == -1)
			my_error("write", 0, WRITE_ERR);
	}
}

int
main(int argc, char **argv)
{
	stralloc        user = { 0 };
	int             match, ignore_newline = 0;
	char           *ptr;
	char            tmp[32];
	tai            *t;

	for (;;) {
		if (getln(&ssin, &user, &match, '\n') == -1)
			my_error("tai2tai64n: read", 0, READ_ERR);
		if (!match && user.len == 0)
			break;
		if (match && ignore_newline)
			user.len--;	/*- remove new line */
		ptr = 0;
		t = tai_decode(&user, &ptr);
		if (ptr) {
			if (!stralloc_0(&user))
				my_error("tai2tai64n: read", 0, MEM_ERR);
			tai64n_encode(t, tmp);
			tmp[31] = 0;
			my_puts(tmp, 0);
			my_puts(ptr, 0);
		} else {
			my_puts(user.s, user.len);
		}
	}
	if (substdio_flush(&ssout) == -1)
		my_error("tai2tai64n: write", 0, WRITE_ERR);
	return (0);
}

void
getversion_tai2tai64n_c()
{
	const char     *x = "$Id: tai2tai64n.c,v 1.2 2021-08-30 12:04:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
