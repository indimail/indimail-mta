/*
 * qcat -- concatenate files and print on the standard output.
 * Copyright (C) 1988-2024 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * convert() function code taken and adaptred from coreutils/src/cat.c
 * By tege@sics.se, Torbjörn Granlund, advised by rms, Richard Stallman.
 *
 * $Id: qcat.c,v 1.12 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <substdio.h>
#include <stralloc.h>
#include <str.h>
#include <getln.h>
#include <error.h>
#include <sig.h>
#include <sgetopt.h>
#include <noreturn.h>
#include "buffer_defs.h"

static char     ssinbuf[BUFSIZE_IN];
static char     ssoutbuf[BUFSIZE_OUT];
static char     sserrbuf[BUFSIZE_OUT];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

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
	if (s2)
	{
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

static void
sigterm()
{
	substdio_flush(&ssout);
	substdio_flush(&sserr);
	close(0);
	close(1);
	close(2);
	_exit(0);
}

void
convert(char *inbuf, int len, int show_nonprinting, int show_tabs, int show_ends)
{
	int             i;
	char           ch;

	/* taken from coreutils/srcs/cat.c */
	for (i = 0; i < len; i++) {
		ch = inbuf[i];
		if (show_nonprinting) {
			if (ch >= 32) {
				if (ch < 127)
					substdio_bput(&ssout, inbuf + i, 1);
				else
				if (ch == 127) {
					substdio_bput(&ssout, "^?", 2);
				} else {
					substdio_bput(&ssout, "M-", 2);
					if (ch >= 128 + 32) {
						if (ch < 128 + 127) {
							inbuf[i] -= 128;
							substdio_bput(&ssout, inbuf + i, 1);
						} else
							substdio_bput(&ssout, "^?", 2);
					} else {
						substdio_bput(&ssout, "^", 1);
						inbuf[i] -= 64;
						substdio_bput(&ssout, inbuf + i, 1);
					}
				}
			} else
			if (ch == '\t' && !show_tabs)
				substdio_bput(&ssout, inbuf + i, 1);
			else
			if (ch == '\n') {
				if (show_ends)
					substdio_bput(&ssout, "$\n", 2);
				else
					substdio_bput(&ssout, "\n", 1);
			}
			else {
				substdio_bput(&ssout, "^", 1);
				inbuf[i] += 64;
				substdio_bput(&ssout, inbuf + i, 1);
			}
		} else {
			/*- Not quoting, -v not specified. */
			if (ch == '\t' && show_tabs) {
				substdio_bput(&ssout, "^", 1);
				inbuf[i] += 64;
				substdio_bput(&ssout, inbuf + i, 1);
			} else
			if (ch == '\r') {
				if (i < len && inbuf[i + 1] == '\n')
					substdio_bput(&ssout, "^M", 2);
				else {
					if (show_ends)
						substdio_bput(&ssout, "^M$\n", 4);
					else
						substdio_bput(&ssout, "^M\n", 3);
				}
			} else
			if (ch == '\n') {
				if (show_ends)
					substdio_bput(&ssout, "$\n", 2);
				else
					substdio_bput(&ssout, "\n", 1);
			} else
				substdio_bput(&ssout, inbuf + i, 1);
		}
	} /*- for (i = 0; i < len; i++) */
}

void
output(int show_nonprinting, int show_tabs, int show_ends)
{
	stralloc        line = { 0 };
	int             match;

	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("qcat: read", 0, 2);
		if (!match && line.len == 0)
			break;
		if (show_nonprinting || show_tabs || show_ends)
			convert(line.s, line.len, show_nonprinting, show_tabs, show_ends);
		else
		if (substdio_bput(&ssout, line.s, line.len) == -1)
			my_error("qcat: write", 0, 3);
		if (substdio_flush(&ssout) == -1)
			my_error("qcat: write", 0, 3);
	}
	if (substdio_flush(&ssout) == -1)
		my_error("qcat: write", 0, 3);
}

int
main(int argc, char **argv)
{
	struct stat     st;
	int             i, fd, mode = O_RDONLY, show_nonprinting = 0, show_ends = 0, show_tabs = 0;

	while ((i = getopt(argc, argv, "etv")) != opteof) {
		switch (i)
		{
		case 'e': /*- display $ at the end */
			show_ends = 1;
			break;
		case 't': /*- show tables */
			show_tabs = 1;
			break;
		case 'v': /*- show non-printing */
			show_nonprinting = 1;
			break;
		}
	}
	sig_termcatch(sigterm);
	if (!(argc - optind))
		output(show_nonprinting, show_tabs, show_ends);
	else {
		for (i = optind; i < argc; i++) {
			if (!str_diff(argv[i], "-")) {
				if (dup2(0, 3) == -1)
					my_error("qcat: error duplicating descriptor 0 to 3", 0, 1);
				break;
			}
		}
		for (i = optind; i < argc; i++) {
			if (!str_diff(argv[i], "-")) {
				if (dup2(3, 0) == -1)
					my_error("qcat: error duplicating descriptor 3 to 0", 0, 1);
			} else {
				if (stat(argv[i], &st) == -1)
					my_error("qcat: stat", argv[i], 1);
				mode = ((st.st_mode & S_IFMT) == S_IFIFO) ? O_RDWR : O_RDONLY;
				if ((fd = open(argv[i], mode)) == -1)
					my_error("qcat: open", argv[i], 1);
				if (dup2(fd, 0) == -1)
					my_error("qcat: error duplicating descriptor to 0", 0, 1);
				if (fd && close(fd))
					my_error("qcat: close", 0, 1);
			}
			output(show_nonprinting, show_tabs, show_ends);
		}
	}
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_cat_c()
{
	const char     *x = "$Id: qcat.c,v 1.12 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: qcat.c,v $
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2024-02-22 08:35:30+05:30  Cprogrammer
 * added options -v, -e, -t with code from coreutils/src/cat.c
 *
 * Revision 1.10  2024-01-23 01:24:23+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.9  2021-10-20 23:30:20+05:30  Cprogrammer
 * added SIGTERM handler to flush io
 *
 * Revision 1.8  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.7  2009-10-03 15:02:59+05:30  Cprogrammer
 * process all arguments on command line
 * bug fix in my_error - 2nd arg never printed
 *
 * Revision 1.6  2009-03-31 09:07:55+05:30  Cprogrammer
 * open files other than fifo in O_RDONLY
 *
 * Revision 1.5  2004-10-27 17:52:37+05:30  Cprogrammer
 * stralloc_0() not needed if substdio_bput() is used instead of substdio_bputs()
 *
 * Revision 1.4  2004-10-22 20:28:10+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-15 23:31:56+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2004-01-10 09:45:07+05:30  Cprogrammer
 * close file descriptor
 *
 * Revision 1.1  2004-01-07 21:09:03+05:30  Cprogrammer
 * Initial revision
 *
 */
