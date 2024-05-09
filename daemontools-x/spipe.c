/*
 * $Log: spipe.c,v $
 * Revision 1.6  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.4  2020-09-16 19:07:34+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.3  2020-06-08 22:52:12+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.2  2004-10-22 20:30:42+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 22:56:30+05:30  Cprogrammer
 * Initial revision
 *
 *
 * spipe -- supervise a pipeline of programs
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
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <noreturn.h>
#include "bool.h"
#include "svcfns.h"

static unsigned part_count;
static pid_t   *part_pids;
static int      selfpipe[2];
static const char **part_names;

void
err(const char *msg)
{
	fputs("spipe: Error: ", stderr);
	fputs(msg, stderr);
	fputc('\n', stderr);
}

no_return void
die(const char *msg)
{
	err(msg);
	exit(1);
}

bool
is_pipe(const char *p)
{
	return p[0] == '|' && p[1] == 0;
}

no_return void
usage(const char *msg)
{
	if (msg)
		err(msg);
	fputs("usage: spipe dir1 dir2 ...\n", stderr);
	exit(1);
}

void
parse_args(int argc, char **argv)
{
	--argc, ++argv;
	if (argc <= 0)
		usage("Too few command-line arguments");

	part_count = argc;
	part_names = (const char **) argv;
	part_pids = calloc(part_count, sizeof(pid_t));
	memset(part_pids, 0, sizeof(pid_t) * part_count);

	if (pipe(selfpipe))
		die("Could not create self pipe");
}

void
stop_parts()
{
	unsigned        i;
	for (i = 0; i < part_count; i++)
		if (part_pids[i])
			stop_supervise(part_names[i], part_pids[i]);
}

void
start_parts()
{
	int             fdout = FD_STDOUT;
	int             fdout_next = -1;
	unsigned        i;
	for (i = part_count; i > 0; i--) {
		int             fdin = FD_STDIN;
		if (i > 1) {
			int             p[2];
			if (pipe(p)) {
				die("Could not create pipe");
				stop_parts();
				exit(1);
			}
			fdin = p[0];
			fdout_next = p[1];
		}
		if (!start_supervise(part_names[i - 1], fdin, fdout)) {
			stop_parts();
			exit(1);
		}
		close(fdout);
		close(fdin);
		fdout = fdout_next;
	}
}

void
handle_intr(int sig)
{
	if (write(selfpipe[1], "", 1) == -1)
		;
}

no_return void
mainloop()
{
	char            buf[1];
	signal(SIGINT, handle_intr);
	signal(SIGQUIT, handle_intr);
	signal(SIGTERM, handle_intr);
	if (read(selfpipe[0], buf, 1) == -1)
		;
	stop_parts();
	exit(0);
}

int
main(int argc, char **argv)
{
	parse_args(argc, argv);
	start_parts();
	mainloop();
	return 0;
}

void
getversion_spipe_c()
{
	const char     *x = "$Id: spipe.c,v 1.6 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $";

	x++;
}
