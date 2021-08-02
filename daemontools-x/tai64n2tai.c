/*-
 * tai64n2tai -- Convert TAI64N timestamps to older TAI format
 * Copyright (C) 2000 Bruce Guenter <bruceg@em.ca>
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
#include "tai2.h"
#include "substdio.h"
#include "getln.h"
#include "stralloc.h"
#include "error.h"
#include <unistd.h>

#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define USAGE_ERR 4

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
my_error(char *s1, char *s2, int exit_val)
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
my_puts(char *s, int len)
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
	stralloc        line = { 0 };
	int             match;
	char           *ptr;
	tai            *t;

	if (!stralloc_ready(&line, 26))
		my_error("tai64n2tai: mem", 0, MEM_ERR);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("tai64n2tai: read", 0, READ_ERR);
		if (!match && line.len == 0)
			break;
		if (!stralloc_0(&line))
			my_error("tai64n2tai: mem", 0, MEM_ERR);
		ptr = 0;
		t = tai64n_decode(&line, &ptr);
		if (ptr) {
		    /*- tai is 20 bytes, tai64n is 25 */
			tai_encode(t, line.s);
			my_puts(line.s, 0);
			my_puts(ptr, 0);
		} else
			my_puts(line.s, line.len);
	}
	if (substdio_flush(&ssout) == -1)
		_exit(1);
	return 0;
}

void
getversion_tai64n2tai_c()
{
	static char    *x = "$Id: tai64n2tai.c,v 1.2 2021-08-03 00:32:51+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
